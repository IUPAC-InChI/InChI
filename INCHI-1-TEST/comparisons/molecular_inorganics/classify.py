"""Classify the mismatches of both comparisons of the MolecularInorganics comparison.

Three passes over the same PubChem SDFs, the columns of issue #259:

  A (reference) v1.07.5, no options
  B             working tree, no options (Molecular Inorganics on)
  C             working tree, -LegacyMetalHandling (Molecular Inorganics off)

Results are stored raw, prefixes included. `run.py` has already compared each
pass with A (`comparators.py`); this re-checks the mismatches it logged.

A vs B: every mismatch is re-computed with the baseline and `-RecMet`, whose
reconnected (`/r`) layer is compared with Run B's InChI without its prefix.
Every mismatch gets one category, named for what happened to the structure:

  changed_without_metal          no metal, yet the InChI changed
  reconnected_as_recmet          MI reproduces what 1.07.5 -RecMet reconnected
  reconnected_differently        both reconnect the metal, differently
  salt_kept_bonded               1.07.5 disconnected a salt, which -RecMet cannot
                                 undo; MI keeps the bond
  changed_without_disconnection  1.07.5 broke no bond to the metal, yet the
                                 InChI changed

plus `error_in_reference`, `error_under_mi`, `recmet_missing` and
`recmet_failed` where a side produced nothing to compare. The disconnection
route is the one Run A's own warning states; the `/r` layer is checked against
it per structure (`route_check`).

A vs C: every mismatch is re-computed with the baseline and `-NPZz`, which
column 3 implies and column 1 does not (`explained_by_npzz` / `unexplained`),
and is checked for also occurring in A vs B, as #259 expects.
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
from molecular_inorganics.comparators import LegacyComparator
from molecular_inorganics.identifiers import (  # noqa: F401  (re-exported)
    METALS,
    formula_elements,
    has_metal,
    inchi_body,
    inchi_prefix,
    is_failed,
)
from molecular_inorganics.consumers import raw_regression_consumer

_FAILURE_PATTERN = re.compile(
    r"^INFO:sdf_pipeline:regression test failed( expectedly)?:(?P<entry>\{.*\})$"
)
_SUMMARY_PATTERN = re.compile(r"Comparison summary: (?P<counts>\{.*\})$")


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

    One process per SDF: the cost per SDF is mostly reading and decompressing the
    whole gzipped file to find the requested records, not the InChI calls.
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


def has_reconnected_layer(inchi: str) -> bool:
    """Whether `-RecMet` emitted a reconnected (`/r`) layer for this structure.

    Its absence means the old code used salt disconnection, which `-RecMet` has
    no mechanism to undo."""
    return "/r" in inchi


def reconnected_layer(inchi: str) -> str:
    """The reconnected-metal (`/r`) layer of an InChI, without any prefix.

    `-RecMet` appends the reconnected structure as a `/r` layer, which for a
    structure that Molecular Inorganics keeps bonded is its whole InChI body.
    Measured on Pt(en)Cl2: RecMet `.../p-2/rC2H6Cl2N2Pt/c3-7(4)5-1-2-6-7/h5-6H,1-2H2`
    and Molecular Inorganics `InChI=1BB/C2H6Cl2N2Pt/c3-7(4)5-1-2-6-7/h5-6H,1-2H2`.
    An InChI without a `/r` layer gives its body without prefix, which can only
    match a Molecular Inorganics body if the metal was not disconnected."""
    body = inchi_body(inchi)
    _, separator, reconnected = body.partition("/r")

    return reconnected if separator else body


def disconnection_route(message: str) -> str:
    """The route the baseline says it took: `metal`, `salt`, `both` or `none`."""
    metal = METAL_DISCONNECTED in message
    salt = SALT_DISCONNECTED in message

    return {(True, False): "metal", (False, True): "salt", (True, True): "both"}.get(
        (metal, salt), "none"
    )


# The category of a mismatch that -RecMet does not reproduce, by the route Run A
# states. `both` goes with metal: the metal disconnection is the one -RecMet
# reverses, so MI and the reconnected layer still disagree on a bond to a metal.
_BY_ROUTE = {
    "metal": "reconnected_differently",
    "both": "reconnected_differently",
    "salt": "salt_kept_bonded",
    "none": "changed_without_disconnection",
}


def _categorize(mismatch: Mismatch, recmet_result: dict | None) -> str:
    if is_failed(mismatch.reference):
        return "error_in_reference"
    if is_failed(mismatch.current):
        return "error_under_mi"
    if not has_metal(mismatch.reference["inchi"]) and not has_metal(
        mismatch.current["inchi"]
    ):
        return "changed_without_metal"
    if recmet_result is None:
        return "recmet_missing"
    if is_failed(recmet_result):
        return "recmet_failed"
    if reconnected_layer(recmet_result["inchi"]) == inchi_body(
        mismatch.current["inchi"]
    ):
        return "reconnected_as_recmet"

    return _BY_ROUTE[disconnection_route(mismatch.reference.get("message", ""))]


# Outcomes of checking the `/r` proxy against the baseline's own message.
ROUTE_CHECKS = ("agrees", "disagrees", "no_disconnection", "no_metal", "not_checked")


def route_check(recmet_result: dict | None) -> str:
    """Whether the `/r` layer names the route the baseline says it took.

    The categories follow the route Run A states; this checks the `/r` layer,
    the structural evidence for that route, against it. An `/r` layer means metal
    disconnection, which `-RecMet` reverses, and its absence salt disconnection,
    which it cannot. `no_disconnection` is a structure with a metal for which the
    baseline broke no bond to it at all."""
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
                reference_inchi=mismatch.reference.get("inchi", ""),
                dev_mi_inchi=mismatch.current.get("inchi", ""),
                recmet_inchi=recmet_result["inchi"] if recmet_result else "",
                reference_message=mismatch.reference.get("message", ""),
                dev_mi_message=mismatch.current.get("message", ""),
                recmet_message=recmet_result.get("message", "")
                if recmet_result
                else "",
                # No metal, no route: kept apart from `no_disconnection`, which
                # on a structure with a metal is a finding of its own.
                route_check="no_metal"
                if category == "changed_without_metal"
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


def prefix_breakdown(classifications: list[Classification]) -> dict[str, dict[str, int]]:
    """Mismatches per category, split by whether the prefix changed.

    Only mismatches where both sides produced an InChI, since a prefix needs one.
    The matched side of the same question is pass B's `unaffected` tally."""
    breakdown = {
        side: {category: 0 for category in CATEGORIES}
        for side in ("prefix_changed", "prefix_unchanged")
    }
    for classification in classifications:
        reference = inchi_prefix(classification.reference_inchi)
        current = inchi_prefix(classification.dev_mi_inchi)
        if not reference or not current:
            continue
        side = "prefix_changed" if reference != current else "prefix_unchanged"
        breakdown[side][classification.category] += 1

    return breakdown


class LegacyClassification(BaseModel):
    """One A vs C mismatch and the baseline's `-NPZz` result for it."""

    molfile_id: str
    sdf: str
    category: str
    # Whether the molfile also mismatches in A vs B, as issue #259 expects.
    in_a_vs_b: bool
    reference_inchi: str
    legacy_inchi: str
    npzz_inchi: str
    reference_key: str = ""
    legacy_key: str = ""
    npzz_key: str = ""
    reference_exit: int | None = None
    legacy_exit: int | None = None
    npzz_exit: int | None = None
    reference_message: str = ""
    legacy_message: str = ""
    npzz_message: str = ""


# Column 3 of issue #259 implies NPZz; column 1 does not. A difference that the
# baseline also produces with -NPZz is explained by that.
LEGACY_CATEGORIES = ("explained_by_npzz", "unexplained")


def classify_legacy_mismatches(
    mismatches: list[Mismatch],
    npzz_results: dict[str, dict],
    a_vs_b_ids: set[str],
) -> list[LegacyClassification]:
    """Classify the A vs C mismatches against the baseline's `-NPZz` results.

    `explained_by_npzz`: the baseline with `-NPZz` gives Run C's result, compared
    with Run C's rule (every field except the prefix and the key flags).
    `unexplained`: anything else, including no `-NPZz` result."""
    same_except_prefix = LegacyComparator()
    classifications: list[LegacyClassification] = []

    for mismatch in mismatches:
        npzz = npzz_results.get(mismatch.molfile_id)
        explained = npzz is not None and same_except_prefix(mismatch.current, npzz)
        classifications.append(
            LegacyClassification(
                molfile_id=mismatch.molfile_id,
                sdf=mismatch.sdf,
                category="explained_by_npzz" if explained else "unexplained",
                in_a_vs_b=mismatch.molfile_id in a_vs_b_ids,
                reference_inchi=mismatch.reference.get("inchi", ""),
                legacy_inchi=mismatch.current.get("inchi", ""),
                npzz_inchi=npzz.get("inchi", "") if npzz else "",
                reference_key=mismatch.reference.get("key", ""),
                legacy_key=mismatch.current.get("key", ""),
                npzz_key=npzz.get("key", "") if npzz else "",
                reference_exit=mismatch.reference.get("exit"),
                legacy_exit=mismatch.current.get("exit"),
                npzz_exit=npzz.get("exit") if npzz else None,
                reference_message=mismatch.reference.get("message", ""),
                legacy_message=mismatch.current.get("message", ""),
                npzz_message=npzz.get("message", "") if npzz else "",
            )
        )

    return classifications


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


def write_classifications(classifications: list[BaseModel], path: Path) -> None:
    """Write one row per mismatch, with the model's fields as columns."""
    path.parent.mkdir(parents=True, exist_ok=True)
    fields = list(type(classifications[0]).model_fields) if classifications else []

    with open(path, "w", newline="", encoding="utf-8") as csv_file:
        writer = csv.DictWriter(csv_file, fieldnames=fields or CSV_FIELDS)
        writer.writeheader()
        for classification in classifications:
            writer.writerow(classification.model_dump())


def a_vs_b_summary(
    classifications: list[Classification], comparison: dict[str, int] | None = None
) -> dict:
    return {
        "counts": classification_counts(classifications),
        "route_check": dict(Counter(c.route_check for c in classifications)),
        "prefix": prefix_breakdown(classifications),
        "total": len(classifications),
        "comparison": comparison or {},
    }


def a_vs_c_summary(
    classifications: list[LegacyClassification],
    comparison: dict[str, int] | None = None,
) -> dict:
    in_a_vs_b = sum(c.in_a_vs_b for c in classifications)

    return {
        "counts": dict(Counter(c.category for c in classifications)),
        "cross_check": {
            "also_in_a_vs_b": in_a_vs_b,
            "only_in_a_vs_c": len(classifications) - in_a_vs_b,
        },
        "total": len(classifications),
        "comparison": comparison or {},
    }


# Every category, in report order. One ID file each, always written, so a
# consumer can rely on the filenames rather than probing for them.
CATEGORIES = (
    "changed_without_metal",
    "reconnected_as_recmet",
    "reconnected_differently",
    "salt_kept_bonded",
    "changed_without_disconnection",
    "error_under_mi",
    "error_in_reference",
    "recmet_failed",
    "recmet_missing",
)


def _id_sort_key(molfile_id: str):
    # PubChem IDs are integers and must not sort as strings (42 before 300);
    # other datasets use names, which fall back to lexicographic order.
    return (0, int(molfile_id), "") if molfile_id.isdigit() else (1, 0, molfile_id)


def write_id_lists(
    classifications: list[BaseModel],
    output_dir: Path,
    categories: tuple[str, ...] = CATEGORIES,
) -> dict[str, int]:
    """Write the molfile IDs of every mismatch to one file per category.

    Returns the count per category. Files land in `<output_dir>/ids/<category>.txt`,
    one ID per line, sorted."""
    grouped: dict[str, list[str]] = {category: [] for category in categories}

    for classification in classifications:
        grouped.setdefault(classification.category, []).append(
            classification.molfile_id
        )

    ids_dir = output_dir.joinpath("ids")
    ids_dir.mkdir(parents=True, exist_ok=True)

    counts: dict[str, int] = {}
    for category, molfile_ids in grouped.items():
        molfile_ids.sort(key=_id_sort_key)
        ids_dir.joinpath(f"{category}.txt").write_text(
            "".join(f"{molfile_id}\n" for molfile_id in molfile_ids), encoding="utf-8"
        )
        counts[category] = len(molfile_ids)

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


def _write_ids(classifications, output_dir: Path, categories) -> None:
    counts = write_id_lists(classifications, output_dir, categories)
    print(f"Wrote ID lists to {output_dir / 'ids'}:")
    for category, count in counts.items():
        print(f"  {count:>8,}  {category}.txt")


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Classify the mismatches of both comparisons, A vs B and A vs C."
    )
    parser.add_argument("--run-b-log", required=True, type=Path)
    parser.add_argument("--run-c-log", required=True, type=Path)
    parser.add_argument(
        "--baseline-lib-path",
        required=True,
        type=str,
        help="The baseline's libinchi.so, for the -RecMet and -NPZz re-runs.",
    )
    parser.add_argument("--data-config", required=True, type=str)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()

    data_config = _load_data_config(args.data_config)

    def recompute(mismatches: list[Mismatch], options: str) -> dict[str, dict]:
        results = recompute_subset(
            sdf_paths=data_config.sdf_paths,
            ids_by_sdf=mismatch_ids_by_sdf(mismatches),
            inchi_lib_path=args.baseline_lib_path,
            inchi_api_parameters=options,
            get_molfile_id=data_config.molfile_id_getter,
        )
        print(f"Re-computed {len(results)} structures with {options}.")

        return results

    # A vs B
    b_mismatches = parse_regression_log(args.run_b_log)
    b_comparison = parse_comparison_summary(args.run_b_log)
    print(f"A vs B: {len(b_mismatches)} mismatches in {args.run_b_log}.")
    b_classifications = classify_mismatches(
        b_mismatches, recompute(b_mismatches, "-RecMet")
    )
    b_dir = args.output / "a_vs_b"
    write_classifications(b_classifications, b_dir / "classifications.csv")
    _write_ids(b_classifications, b_dir, CATEGORIES)
    messages = explain_failures(b_classifications)
    if messages:
        (b_dir / "error_under_mi_messages.json").write_text(
            json.dumps(messages, indent=2), encoding="utf-8"
        )

    # A vs C
    c_mismatches = parse_regression_log(args.run_c_log)
    c_comparison = parse_comparison_summary(args.run_c_log)
    print(f"A vs C: {len(c_mismatches)} mismatches in {args.run_c_log}.")
    c_classifications = classify_legacy_mismatches(
        c_mismatches,
        recompute(c_mismatches, "-NPZz"),
        {mismatch.molfile_id for mismatch in b_mismatches},
    )
    c_dir = args.output / "a_vs_c"
    write_classifications(c_classifications, c_dir / "classifications.csv")
    _write_ids(c_classifications, c_dir, LEGACY_CATEGORIES)

    summary = {
        "a_vs_b": a_vs_b_summary(b_classifications, b_comparison),
        "a_vs_c": a_vs_c_summary(c_classifications, c_comparison),
    }
    (args.output / "summary.json").write_text(
        json.dumps(summary, indent=2), encoding="utf-8"
    )
    print(json.dumps({side: part["counts"] for side, part in summary.items()}, indent=2))


if __name__ == "__main__":
    main()
