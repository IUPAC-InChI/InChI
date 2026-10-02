"""Classify the mismatches of the MolecularInorganics comparison.

Three runs over the same PubChem SDFs:

  A (reference) v1.07.5, no options
  B             dev, -MolecularInorganics
  C             dev, no options

Results are stored raw, prefixes included. Comparison ignores the
version-and-kind prefix (`comparators.PrefixInsensitiveComparator`), and every
A-vs-B mismatch is re-checked against v1.07.5 `-RecMet` by comparing that run's
reconnected (`/r`) layer with the `-MolecularInorganics` InChI minus its prefix.

A mismatch on a structure without any metal is its own category, `metal_free`:
MolecularInorganics only changes how bonds to metals are treated, so it has no
mechanism to change such a structure. The `-RecMet` re-run also keeps the
baseline's warning message, which states which disconnection route it took, so
the `/r` layer's use as a proxy for the route is checked per structure.
"""

import argparse
import csv
import importlib
import json
import multiprocessing
import os
import re
import sys
from collections import Counter, defaultdict
from functools import partial
from pathlib import Path
from typing import Callable
from pydantic import BaseModel
from sdf_pipeline.utils import select_records_from_gzipped_sdf
from inchi_tests.consumers import inchi_body, is_failed
from molecular_inorganics.consumers import raw_regression_consumer

_FAILURE_PATTERN = re.compile(
    r"^INFO:sdf_pipeline:regression test failed( expectedly)?:(?P<entry>\{.*\})$"
)
_SUMMARY_PATTERN = re.compile(r"Comparison summary: (?P<counts>\{.*\})$")


# Elements InChI treats as metals: the rows of `INCHI_BASE/src/eldata.c` whose
# type is METAL or METAL2, i.e. what `is_el_a_metal` returns true for. Not the
# textbook list: Ge, As and Te are not metals here, Sb, Po, Ts and Og are.
METALS = frozenset(
    "Li Be Na Mg Al K Ca Sc Ti V Cr Mn Fe Co Ni Cu Zn Ga Rb Sr Y Zr Nb Mo Tc Ru "
    "Rh Pd Ag Cd In Sn Sb Cs Ba La Ce Pr Nd Pm Sm Eu Gd Tb Dy Ho Er Tm Yb Lu Hf "
    "Ta W Re Os Ir Pt Au Hg Tl Pb Bi Po Fr Ra Ac Th Pa U Np Pu Am Cm Bk Cf Es Fm "
    "Md No Lr Rf Db Sg Bh Hs Mt Ds Rg Cn Nh Fl Mc Lv Ts Og".split()
)
_ELEMENT = re.compile(r"[A-Z][a-z]?")

# Warnings the baseline emits when it breaks a bond to a metal
# (`runichi3.c:563` and `:683`). Both can appear for one structure.
METAL_DISCONNECTED = "Metal was disconnected"
SALT_DISCONNECTED = "Salt was disconnected"


class Mismatch(BaseModel):
    molfile_id: str
    sdf: str
    current: dict
    reference: dict
    expected: bool


def parse_regression_log(log_path: Path) -> list[Mismatch]:
    mismatches: list[Mismatch] = []

    with open(log_path, "r", encoding="utf-8") as log_file:
        for line in log_file:
            match = _FAILURE_PATTERN.match(line.rstrip("\n"))
            if not match:
                continue
            entry = json.loads(match.group("entry"))
            mismatches.append(
                Mismatch(
                    molfile_id=entry["molfile_id"],
                    sdf=entry["sdf"],
                    current=json.loads(entry["diff"]["current"]),
                    reference=json.loads(entry["diff"]["reference"]),
                    expected=bool(match.group(1)),
                )
            )

    return mismatches


def mismatch_ids_by_sdf(mismatches: list[Mismatch]) -> dict[str, set[str]]:
    grouped: dict[str, set[str]] = defaultdict(set)

    for mismatch in mismatches:
        grouped[mismatch.sdf].add(mismatch.molfile_id)

    return dict(grouped)


def parse_comparison_summary(log_path: Path) -> dict[str, int]:
    """The tally of differences the comparison deliberately ignored."""
    counts: dict[str, int] = {}

    with open(log_path, "r", encoding="utf-8") as log_file:
        for line in log_file:
            match = _SUMMARY_PATTERN.search(line.rstrip("\n"))
            if match:
                counts = json.loads(match.group("counts"))

    return counts


def _recompute_one_sdf(
    sdf_path: Path,
    ids_by_sdf: dict[str, set[str]],
    inchi_lib_path: str,
    inchi_api_parameters: str,
    get_molfile_id: Callable,
    consumer: Callable,
) -> dict[str, dict]:
    molfile_ids = ids_by_sdf.get(sdf_path.name, set())
    if not molfile_ids:
        return {}

    results: dict[str, dict] = {}
    for _, molfile in select_records_from_gzipped_sdf(
        sdf_path, molfile_ids, get_molfile_id
    ):
        consumer_result = consumer(
            molfile,
            get_molfile_id=get_molfile_id,
            inchi_lib_path=inchi_lib_path,
            inchi_api_parameters=inchi_api_parameters,
        )
        results[consumer_result.molfile_id] = consumer_result.result

    return results


def recompute_subset(
    sdf_paths: list[Path],
    ids_by_sdf: dict[str, set[str]],
    inchi_lib_path: str,
    inchi_api_parameters: str,
    get_molfile_id: Callable,
    consumer: Callable = raw_regression_consumer,
    number_of_processes: int | None = None,
) -> dict[str, dict]:
    """Re-compute InChI for selected molfile IDs only, with arbitrary options.

    One process per SDF: the per-shard cost is dominated by streaming and decoding
    the whole gzipped file to find the requested records, not by the InChI calls.
    `get_molfile_id` and `consumer` are pickled, so they must be importable
    module-level callables unless `number_of_processes=1`."""
    todo = [path for path in sdf_paths if ids_by_sdf.get(path.name)]
    if not todo:
        return {}

    worker = partial(
        _recompute_one_sdf,
        ids_by_sdf=ids_by_sdf,
        inchi_lib_path=inchi_lib_path,
        inchi_api_parameters=inchi_api_parameters,
        get_molfile_id=get_molfile_id,
        consumer=consumer,
    )

    if number_of_processes == 1:
        per_sdf = [worker(path) for path in todo]
    else:
        n_processes = min(number_of_processes or os.cpu_count() or 8, len(todo))
        with multiprocessing.get_context("spawn").Pool(n_processes) as pool:
            per_sdf = pool.map(worker, todo)

    results: dict[str, dict] = {}
    for chunk in per_sdf:
        results.update(chunk)

    return results


class Classification(BaseModel):
    molfile_id: str
    sdf: str
    category: str
    reference_inchi: str
    dev_mi_inchi: str
    recmet_inchi: str
    # Warning text of all three sides; the -RecMet one names the disconnection
    # route, and `route_check` records whether the `/r` layer agrees with it.
    reference_message: str = ""
    dev_mi_message: str = ""
    recmet_message: str = ""
    route_check: str = ""
    # Carried through from the raw results so the CSV answers key and exit-code
    # questions without anyone having to parse the logs.
    reference_key: str = ""
    dev_mi_key: str = ""
    recmet_key: str = ""
    reference_exit: int | None = None
    dev_mi_exit: int | None = None
    recmet_exit: int | None = None


def formula_elements(inchi: str) -> set[str]:
    """The element symbols in the formula layer of an InChI."""
    body = inchi_body(inchi)
    if not body:
        return set()

    return set(_ELEMENT.findall(body.split("/", 1)[0]))


def has_metal(inchi: str) -> bool:
    return not formula_elements(inchi).isdisjoint(METALS)


def has_reconnected_layer(inchi: str) -> bool:
    """Whether `-RecMet` emitted a reconnected (`/r`) layer for this structure.

    Its absence means the old code used salt disconnection, which `-RecMet` has
    no mechanism to undo."""
    return "/r" in inchi


def reconnected_layer(inchi: str) -> str:
    """The reconnected-metal (`/r`) layer of an InChI, without any prefix.

    `-RecMet` appends the reconnected structure as a `/r` layer, and that layer is
    exactly what `-MolecularInorganics` emits as its whole InChI body. Measured on
    Pt(en)Cl2: RecMet `.../p-2/rC2H6Cl2N2Pt/c3-7(4)5-1-2-6-7/h5-6H,1-2H2` vs MI
    `InChI=1B/C2H6Cl2N2Pt/c3-7(4)5-1-2-6-7/h5-6H,1-2H2`. An InChI with no `/r`
    layer yields its prefix-stripped self, which then cannot match an MI body
    unless the metal was never disconnected in the first place."""
    body = inchi_body(inchi)
    _, separator, reconnected = body.partition("/r")

    return reconnected if separator else body


def _categorize(mismatch: Mismatch, recmet_result: dict | None) -> str:
    if is_failed(mismatch.reference):
        return "error_in_reference"
    if is_failed(mismatch.current):
        return "error_under_mi"
    if not has_metal(mismatch.reference["inchi"]) and not has_metal(
        mismatch.current["inchi"]
    ):
        return "metal_free"
    if recmet_result is None:
        return "recmet_missing"
    if is_failed(recmet_result):
        return "recmet_failed"
    if reconnected_layer(recmet_result["inchi"]) == inchi_body(
        mismatch.current["inchi"]
    ):
        return "recmet_equivalent"

    return "novel"


# Outcomes of checking the `/r` proxy against the baseline's own message.
ROUTE_CHECKS = ("agrees", "disagrees", "no_disconnection", "no_metal", "not_checked")


def route_check(recmet_result: dict | None) -> str:
    """Whether the `/r` layer names the route the baseline says it took.

    `novel` is split by the stated route (`novel_pathway`); this keeps the `/r`
    layer honest as the structural evidence for it. The proxy reads an `/r` layer as metal disconnection, which `-RecMet`
    reverses, and its absence as salt disconnection, which it cannot. The
    baseline states the route in its warnings, so the proxy is checked rather
    than assumed. `no_disconnection` is a structure for which the baseline broke
    no bond to a metal at all."""
    if recmet_result is None or is_failed(recmet_result):
        return "not_checked"
    message = recmet_result.get("message", "")
    metal = METAL_DISCONNECTED in message
    salt = SALT_DISCONNECTED in message
    if not metal and not salt:
        return "no_disconnection"
    if has_reconnected_layer(recmet_result["inchi"]):
        return "agrees" if metal else "disagrees"

    return "agrees" if salt and not metal else "disagrees"


def classify_mismatches(
    mismatches: list[Mismatch], recmet_results: dict[str, dict]
) -> list[Classification]:
    classifications: list[Classification] = []

    for mismatch in mismatches:
        recmet_result = recmet_results.get(mismatch.molfile_id)
        category = _categorize(mismatch, recmet_result)
        classifications.append(
            Classification(
                molfile_id=mismatch.molfile_id,
                sdf=mismatch.sdf,
                category=category,
                reference_inchi=mismatch.reference["inchi"],
                dev_mi_inchi=mismatch.current["inchi"],
                recmet_inchi=recmet_result["inchi"] if recmet_result else "",
                reference_message=mismatch.reference.get("message", ""),
                dev_mi_message=mismatch.current.get("message", ""),
                recmet_message=recmet_result.get("message", "")
                if recmet_result
                else "",
                # No metal, no route: kept apart from `no_disconnection`, which
                # on a structure with a metal is a finding of its own.
                route_check="no_metal"
                if category == "metal_free"
                else route_check(recmet_result),
                reference_key=mismatch.reference.get("key", ""),
                dev_mi_key=mismatch.current.get("key", ""),
                recmet_key=recmet_result.get("key", "") if recmet_result else "",
                reference_exit=mismatch.reference.get("exit"),
                dev_mi_exit=mismatch.current.get("exit"),
                recmet_exit=recmet_result.get("exit") if recmet_result else None,
            )
        )

    return classifications


def inchi_prefix(inchi: str) -> str:
    """The version-and-kind prefix of an InChI, e.g. `InChI=1S`; empty if none."""
    return inchi.split("/", 1)[0] if "/" in inchi else ""


# What the baseline's -RecMet re-run says about a mismatch: its `/r` layer equals
# the option's InChI body, exists but differs, or does not exist -- including
# when -RecMet produced nothing at all.
RECMET_LAYERS = ("equals_new_inchi", "differs", "no_layer")


def recmet_layer_status(classification: Classification) -> str:
    recmet = classification.recmet_inchi
    if not has_reconnected_layer(recmet):
        return "no_layer"
    if reconnected_layer(recmet) == inchi_body(classification.dev_mi_inchi):
        return "equals_new_inchi"

    return "differs"


def prefix_breakdown(classifications: list[Classification]) -> dict[str, dict[str, int]]:
    """Mismatches split by whether the prefix changed and by the -RecMet layer.

    Only mismatches where both sides produced an InChI, since a prefix needs one.
    The matched side of the same question is pass B's `only_prefix_changed`
    tally."""
    breakdown = {
        side: {status: 0 for status in RECMET_LAYERS}
        for side in ("prefix_changed", "prefix_unchanged")
    }
    for classification in classifications:
        reference = inchi_prefix(classification.reference_inchi)
        current = inchi_prefix(classification.dev_mi_inchi)
        if not reference or not current:
            continue
        side = "prefix_changed" if reference != current else "prefix_unchanged"
        breakdown[side][recmet_layer_status(classification)] += 1

    return breakdown


def classification_counts(classifications: list[Classification]) -> dict[str, int]:
    return dict(Counter(c.category for c in classifications))


CSV_FIELDS = [
    "molfile_id",
    "sdf",
    "category",
    "reference_inchi",
    "dev_mi_inchi",
    "recmet_inchi",
    "reference_message",
    "dev_mi_message",
    "recmet_message",
    "route_check",
    "reference_key",
    "dev_mi_key",
    "recmet_key",
    "reference_exit",
    "dev_mi_exit",
    "recmet_exit",
]


def write_report(
    classifications: list[Classification],
    output_dir: Path,
    comparison_summary: dict[str, int] | None = None,
) -> None:
    output_dir.mkdir(parents=True, exist_ok=True)

    with open(
        output_dir.joinpath("classifications.csv"), "w", newline="", encoding="utf-8"
    ) as csv_file:
        writer = csv.DictWriter(csv_file, fieldnames=CSV_FIELDS)
        writer.writeheader()
        for classification in classifications:
            writer.writerow(classification.model_dump())

    with open(output_dir.joinpath("summary.json"), "w", encoding="utf-8") as json_file:
        json.dump(
            {
                "counts": classification_counts(classifications),
                "route_check": dict(Counter(c.route_check for c in classifications)),
                "prefix": prefix_breakdown(classifications),
                "total": len(classifications),
                "comparison": comparison_summary or {},
            },
            json_file,
            indent=2,
        )


# One file per cause, always written, so a consumer can rely on the filenames
# rather than probing for them. `novel` is split by disconnection pathway because
# that is the actual cause: the old code breaks bonds to metals by two routes and
# `-RecMet` only reverses one of them. A structure the baseline did not
# disconnect at all has neither route and is kept apart.
ID_LIST_CAUSES = (
    "metal_free",
    "recmet_equivalent",
    "novel_metal_pathway",
    "novel_salt_pathway",
    "novel_no_disconnection",
    "error_under_mi",
    "error_in_reference",
    "recmet_failed",
    "recmet_missing",
)


def disconnection_route(message: str) -> str:
    """The route the baseline says it took: `metal`, `salt`, `both` or `none`."""
    metal = METAL_DISCONNECTED in message
    salt = SALT_DISCONNECTED in message

    return {(True, False): "metal", (False, True): "salt", (True, True): "both"}.get(
        (metal, salt), "none"
    )


# How a `novel` structure's route maps to its cause bucket. `both` goes with
# metal: the metal disconnection is the one `-RecMet` reverses, so MI and the
# reconnected layer still disagree on a bond to a metal.
_NOVEL_PATHWAY = {
    "metal": "novel_metal_pathway",
    "both": "novel_metal_pathway",
    "salt": "novel_salt_pathway",
    "none": "novel_no_disconnection",
}


def novel_pathway(reference_message: str | None, recmet_inchi: str) -> str:
    """The cause bucket of a `novel` structure, from the route the baseline states.

    The baseline's own message in Run A names the route. Classifications written
    before messages were stored have none (`None`); for those the `/r` layer is
    the proxy, which cannot tell a salt disconnection from no disconnection."""
    if reference_message is None:
        return (
            "novel_metal_pathway"
            if has_reconnected_layer(recmet_inchi)
            else "novel_salt_pathway"
        )

    return _NOVEL_PATHWAY[disconnection_route(reference_message)]


def cause_of(classification: Classification) -> str:
    """The cause bucket a classification belongs in, splitting `novel` by pathway."""
    if classification.category != "novel":
        return classification.category

    return novel_pathway(classification.reference_message, classification.recmet_inchi)


def _id_sort_key(molfile_id: str):
    # PubChem IDs are integers and must not sort as strings (42 before 300);
    # other datasets use names, which fall back to lexicographic order.
    return (0, int(molfile_id), "") if molfile_id.isdigit() else (1, 0, molfile_id)


def write_id_lists(
    classifications: list[Classification], output_dir: Path
) -> dict[str, int]:
    """Write the molfile IDs of every mismatch to one file per cause.

    Returns the count per cause. Files land in `<output_dir>/ids/<cause>.txt`,
    one ID per line, sorted."""
    grouped: dict[str, list[str]] = {cause: [] for cause in ID_LIST_CAUSES}

    for classification in classifications:
        grouped.setdefault(cause_of(classification), []).append(
            classification.molfile_id
        )

    ids_dir = output_dir.joinpath("ids")
    ids_dir.mkdir(parents=True, exist_ok=True)

    counts: dict[str, int] = {}
    for cause, molfile_ids in grouped.items():
        molfile_ids.sort(key=_id_sort_key)
        ids_dir.joinpath(f"{cause}.txt").write_text(
            "".join(f"{molfile_id}\n" for molfile_id in molfile_ids), encoding="utf-8"
        )
        counts[cause] = len(molfile_ids)

    return counts


def explain_failures(classifications: list[Classification]) -> dict[str, str]:
    """The InChI `message` of every structure that failed only under MI."""
    return {
        c.molfile_id: c.dev_mi_message
        for c in classifications
        if c.category == "error_under_mi"
    }


def _load_data_config(data_config_path: str):
    sys.path.append(str(Path(data_config_path).parent))

    return importlib.import_module(str(Path(data_config_path).stem)).config


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Classify MolecularInorganics regression mismatches."
    )
    parser.add_argument("--regression-log", required=True, type=str)
    parser.add_argument(
        "--recmet-lib-path", required=True, type=str, help="v1.07.5 libinchi.so"
    )
    parser.add_argument("--data-config", required=True, type=str)
    parser.add_argument("--output", required=True, type=str)
    args = parser.parse_args()

    data_config = _load_data_config(args.data_config)
    log_path = Path(args.regression_log)
    mismatches = parse_regression_log(log_path)
    comparison_summary = parse_comparison_summary(log_path)
    print(f"Parsed {len(mismatches)} mismatches from {args.regression_log}.")
    print(f"Ignored-difference tallies: {json.dumps(comparison_summary)}")

    recmet_results = recompute_subset(
        sdf_paths=data_config.sdf_paths,
        ids_by_sdf=mismatch_ids_by_sdf(mismatches),
        inchi_lib_path=args.recmet_lib_path,
        inchi_api_parameters="-RecMet",
        get_molfile_id=data_config.molfile_id_getter,
    )
    print(
        f"Re-computed {len(recmet_results)} structures with -RecMet using "
        f"{args.recmet_lib_path}."
    )

    classifications = classify_mismatches(mismatches, recmet_results)
    output_dir = Path(args.output)
    write_report(classifications, output_dir, comparison_summary)

    id_counts = write_id_lists(classifications, output_dir)
    print(f"Wrote ID lists to {output_dir / 'ids'}:")
    for cause, count in id_counts.items():
        print(f"  {count:>8,}  {cause}.txt")

    messages = explain_failures(classifications)
    if messages:
        with open(
            output_dir.joinpath("error_under_mi_messages.json"), "w", encoding="utf-8"
        ) as json_file:
            json.dump(messages, json_file, indent=2)

    print(json.dumps(classification_counts(classifications), indent=2))
    print(
        "Route proxy check: "
        f"{json.dumps(dict(Counter(c.route_check for c in classifications)))}"
    )


if __name__ == "__main__":
    main()
