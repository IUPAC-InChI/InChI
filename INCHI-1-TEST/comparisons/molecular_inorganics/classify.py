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

`reconnected_differently` is split further by how MI's bonds to metals differ
from the reconnected layer's (`difference`, one ID file each under
`ids/reconnected_differently/`).

plus `error_in_reference`, `error_under_mi`, `recmet_missing` and
`recmet_failed` where a side produced nothing to compare. The disconnection
route is the one Run A's own warning states; the `/r` layer is checked against
it per structure (`route_check`).

A vs C: every mismatch is re-computed with the baseline and `-NPZz`, which
column 3 implies and column 1 does not, and is checked for also occurring in
A vs B, as #259 expects. A mismatch `-NPZz` does not explain is matched
against the changes since 1.07.5 that the CHANGELOG lists:

  explained_by_npzz           the baseline with -NPZz gives Run C's result
  mn7_warning_dropped         only the Mn(7) unusual-valence warning is gone
                              (valence 7 was added to neutral Mn)
  accepted_above_old_maxval   only Run C gives an InChI, with an atom of more
                              than 20 bonds (GHI #77 raised MAXVAL to 50)
  message_text_changed        only the wording of a V3000 message changed
  no_hydrogen_on_metal        the baseline's InChI has bare H components Run
                              C's lacks: 1.07.5 gave a metal without bonds or
                              charge implicit H, then disconnected it
  metal_not_disconnected      the InChI changed and Run C did not disconnect
                              the metal the baseline disconnected
  changed_otherwise           the InChI changed for another reason
  accepted_otherwise          only Run C gives an InChI, without an atom above
                              the old MAXVAL
  unexplained                 anything else

The last four have no known cause and fail the control's gate.
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
    metal_bonds,
    most_bonds,
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
    # For `reconnected_differently`: how MI's bonds to metals differ from the
    # reconnected layer's (`difference_kind`); empty for every other category.
    difference: str = ""
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


# How a `reconnected_differently` structure differs, read from the bonds to
# metals in MI's `/c` layer and the reconnected layer's. Both always have the
# same heavy atoms; what differs is which bonds are kept and where H sits.
DIFFERENCES = (
    "metal_hydrogen_bonded",
    "partly_disconnected",
    "group_1_2_disconnected",
    "more_bonds_kept",
    "mixed",
    "same_metal_bonds",
    "not_parsed",
)
GROUP_1_2 = frozenset("Li Na K Rb Cs Fr Be Mg Ca Sr Ba Ra".split())


def difference_kind(dev_mi_inchi: str, recmet_inchi: str) -> str:
    """How MI's bonds to metals differ from the `-RecMet` reconnected layer's.

    `metal_hydrogen_bonded`: MI only has extra metal-H bonds; it writes a
    hydrogen on a metal as a bonded atom in `/c`, where 1.07.5 puts it in `/h`.
    `partly_disconnected`: MI only lacks bonds, some of them to a metal outside
    groups 1 and 2. `group_1_2_disconnected`: MI only lacks bonds to group 1/2
    metals. `more_bonds_kept`: MI only has extra bonds, not all of them to H,
    e.g. ones 1.07.5 broke as a salt. `mixed`: both extra and missing bonds.
    `same_metal_bonds`: the same bonds to metals; H, charge or the rest of the
    structure differ. `not_parsed`: a `/c` layer that could not be read."""
    current = metal_bonds(inchi_body(dev_mi_inchi))
    reconnected = metal_bonds(reconnected_layer(recmet_inchi))
    if current is None or reconnected is None:
        return "not_parsed"
    difference = current[1].copy()
    difference.subtract(reconnected[1])
    extra = {pair for pair, n in difference.items() if n > 0}
    missing = {pair for pair, n in difference.items() if n < 0}

    if extra and missing:
        return "mixed"
    if extra:
        return (
            "metal_hydrogen_bonded"
            if all(partner == "H" for _, partner in extra)
            else "more_bonds_kept"
        )
    if missing:
        return (
            "group_1_2_disconnected"
            if all(metal in GROUP_1_2 for metal, _ in missing)
            else "partly_disconnected"
        )

    return "same_metal_bonds"


# Outcomes of checking the `/r` proxy against the baseline's own message.
ROUTE_CHECKS = (
    "agrees", "disagrees", "no_disconnection", "message_truncated", "no_metal", "not_checked"
)


def route_check(recmet_result: dict | None) -> str:
    """Whether the `/r` layer names the route the baseline says it took.

    The categories follow the route Run A states; this checks the `/r` layer,
    the structural evidence for that route, against it. An `/r` layer means metal
    disconnection, which `-RecMet` reverses, and its absence salt disconnection,
    which it cannot. `no_disconnection` is a structure with a metal for which the
    baseline broke no bond to it at all, as far as its message tells:
    `message_truncated` is one whose message was cut off at the library's
    256-character limit (ending in "...") before naming any route."""
    if recmet_result is None or is_failed(recmet_result):
        return "not_checked"
    message = recmet_result.get("message", "")
    metal = METAL_DISCONNECTED in message
    salt = SALT_DISCONNECTED in message
    if not metal and not salt:
        return "message_truncated" if "..." in message else "no_disconnection"
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
                difference=difference_kind(
                    mismatch.current["inchi"], recmet_result["inchi"]
                )
                if category == "reconnected_differently"
                else "",
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
# baseline also produces with -NPZz is explained by that; the next four by a
# change the CHANGELOG lists. The rest have no known cause.
LEGACY_EXPLAINED = (
    "explained_by_npzz",
    "mn7_warning_dropped",
    "accepted_above_old_maxval",
    "message_text_changed",
    "no_hydrogen_on_metal",
)
LEGACY_UNEXPLAINED = (
    "metal_not_disconnected",
    "changed_otherwise",
    "accepted_otherwise",
    "unexplained",
)
LEGACY_CATEGORIES = LEGACY_EXPLAINED + LEGACY_UNEXPLAINED

# The atom valences of an `Accepted unusual valence(s): Mn(7); N(4)` warning.
# The list shares the message's `; ` separator, so its entries are the items
# that follow the warning and look like `El(n)`, charge included (`Mn+1(6)`).
# The library cuts a long message off with `...` after the last entry.
_VALENCE_WARNING = "Accepted unusual valence(s): "
_VALENCE_ITEM = re.compile(r"^[A-Z][a-z]?(?:[+-]\d*)?\((?P<valence>\d+)\)(?:\.\.\.)?$")
# MAXVAL up to 1.07.5; GHI #77 raised it to 50.
OLD_MAXVAL = 20


def split_message(message: str) -> tuple[list[str], list[str]]:
    """The atom valences of the unusual-valence warning and the other items."""
    valences: list[str] = []
    others: list[str] = []
    in_valences = False
    for item in filter(None, (item.strip() for item in message.split(";"))):
        if item.startswith(_VALENCE_WARNING):
            valences.append(item[len(_VALENCE_WARNING):])
            in_valences = True
        elif in_valences and _VALENCE_ITEM.match(item):
            valences.append(item)
        else:
            others.append(item)
            in_valences = False

    return valences, others


_BARE_HYDROGEN = re.compile(r"\d*H")


def only_hydrogen_on_metal_differs(legacy: dict, npzz: dict) -> bool:
    """Whether the baseline's InChI is Run C's plus bare hydrogen components.

    1.07.5 gives a metal without bonds or charge implicit hydrogens and then
    disconnects them, leaving `.nH` components; Run C adds none to a metal
    (CHANGELOG). Every layer must match once the components' trailing
    separators are dropped, and the messages may differ only in "Metal was
    disconnected", which is missing where the hydrogen was the metal's only
    bond."""
    legacy_layers = inchi_body(legacy["inchi"]).split("/")
    npzz_layers = inchi_body(npzz["inchi"]).split("/")
    npzz_components = npzz_layers[0].split(".")
    bare = [c for c in npzz_components if _BARE_HYDROGEN.fullmatch(c)]
    if not bare or npzz_components[-len(bare):] != bare:
        return False
    if ".".join(npzz_components[: -len(bare)]) != legacy_layers[0]:
        return False
    if len(legacy_layers) != len(npzz_layers) or any(
        a.rstrip(";") != b.rstrip(";")
        for a, b in zip(legacy_layers[1:], npzz_layers[1:])
    ):
        return False

    def items(message: str) -> list[str]:
        return [i.strip() for i in message.split(";") if i.strip() != METAL_DISCONNECTED]

    return items(legacy.get("message", "")) == items(npzz.get("message", ""))


def _legacy_cause(legacy: dict, npzz: dict | None) -> str:
    """The category of an A vs C mismatch that `-NPZz` does not explain."""
    if npzz is None:
        return "unexplained"
    legacy_valences, legacy_others = split_message(legacy.get("message", ""))
    npzz_valences, npzz_others = split_message(npzz.get("message", ""))

    if is_failed(npzz) and not is_failed(legacy):
        # The unusual-valence warning does not cover every atom (none for
        # I- with 38 bonds), so the /c layer is counted as well.
        valences = [int(m["valence"]) for m in map(_VALENCE_ITEM.match, legacy_valences) if m]
        valences.append(most_bonds(inchi_body(legacy["inchi"])))
        if max(valences) > OLD_MAXVAL:
            return "accepted_above_old_maxval"
        return "accepted_otherwise"

    if is_failed(npzz) or is_failed(legacy):
        return "unexplained"

    if inchi_body(legacy["inchi"]) != inchi_body(npzz["inchi"]):
        if legacy["exit"] == npzz["exit"] and only_hydrogen_on_metal_differs(
            legacy, npzz
        ):
            return "no_hydrogen_on_metal"
        if METAL_DISCONNECTED in npzz_others and METAL_DISCONNECTED not in legacy_others:
            return "metal_not_disconnected"
        return "changed_otherwise"

    # Same InChI body; the key hash and exit code must match too, so only the
    # message differs.
    if not LegacyComparator()({**legacy, "message": ""}, {**npzz, "message": ""}):
        return "unexplained"
    if "Mn(7)" in npzz_valences:
        remaining = list(npzz_valences)
        remaining.remove("Mn(7)")
        if remaining == legacy_valences and legacy_others == npzz_others:
            return "mn7_warning_dropped"
    if legacy_valences == npzz_valences and all(
        item.startswith("V3000 ")
        for item in set(legacy_others).symmetric_difference(npzz_others)
    ):
        return "message_text_changed"

    return "unexplained"


def classify_legacy_mismatches(
    mismatches: list[Mismatch],
    npzz_results: dict[str, dict],
    a_vs_b_ids: set[str],
) -> list[LegacyClassification]:
    """Classify the A vs C mismatches against the baseline's `-NPZz` results.

    `explained_by_npzz`: the baseline with `-NPZz` gives Run C's result, compared
    with Run C's rule (every field except the prefix and the key flags).
    Anything else gets the category of its cause (`_legacy_cause`), and
    `unexplained` when it has none, including no `-NPZz` result."""
    same_except_prefix = LegacyComparator()
    classifications: list[LegacyClassification] = []

    for mismatch in mismatches:
        npzz = npzz_results.get(mismatch.molfile_id)
        explained = npzz is not None and same_except_prefix(mismatch.current, npzz)
        classifications.append(
            LegacyClassification(
                molfile_id=mismatch.molfile_id,
                sdf=mismatch.sdf,
                category="explained_by_npzz"
                if explained
                else _legacy_cause(mismatch.current, npzz),
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
    "difference",
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
        "differences": dict(Counter(c.difference for c in classifications if c.difference)),
        "prefix": prefix_breakdown(classifications),
        "total": len(classifications),
        "comparison": comparison or {},
    }


def _seen_by_option_comparator(row: LegacyClassification) -> bool:
    """Whether Run B's comparator would count this A vs C difference."""
    def result(inchi: str, exit: int | None) -> dict:
        return {"inchi": inchi} if exit is None else {"inchi": inchi, "exit": exit}

    reference = result(row.reference_inchi, row.reference_exit)
    legacy = result(row.legacy_inchi, row.legacy_exit)

    return inchi_body(row.reference_inchi) != inchi_body(row.legacy_inchi) or is_failed(
        reference
    ) != is_failed(legacy)


def a_vs_c_summary(
    classifications: list[LegacyClassification],
    comparison: dict[str, int] | None = None,
) -> dict:
    in_a_vs_b = sum(c.in_a_vs_b for c in classifications)
    # Run B compares only the InChI body and the failure state, so a difference
    # in the warning alone cannot show up there.
    warning_only = sum(
        not c.in_a_vs_b and not _seen_by_option_comparator(c) for c in classifications
    )

    return {
        "counts": dict(Counter(c.category for c in classifications)),
        "cross_check": {
            "also_in_a_vs_b": in_a_vs_b,
            "warning_only": warning_only,
            "only_in_a_vs_c": len(classifications) - in_a_vs_b - warning_only,
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
    field: str = "category",
    subdir: str = "ids",
) -> dict[str, int]:
    """Write the molfile IDs of every mismatch to one file per category.

    Returns the count per category. Files land in `<output_dir>/<subdir>/<category>.txt`,
    one ID per line, sorted. `field` names the attribute to group by; rows
    where it is empty are left out."""
    grouped: dict[str, list[str]] = {category: [] for category in categories}

    for classification in classifications:
        value = getattr(classification, field)
        if value:
            grouped.setdefault(value, []).append(classification.molfile_id)

    ids_dir = output_dir.joinpath(subdir)
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


def _write_ids(
    classifications, output_dir: Path, categories, field="category", subdir="ids"
) -> None:
    counts = write_id_lists(classifications, output_dir, categories, field, subdir)
    print(f"Wrote ID lists to {output_dir / subdir}:")
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
    write_classifications(b_classifications, b_dir / "a_vs_b_classifications.csv")
    _write_ids(b_classifications, b_dir, CATEGORIES)
    _write_ids(
        b_classifications, b_dir, DIFFERENCES, "difference", "ids/reconnected_differently"
    )
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
    write_classifications(c_classifications, c_dir / "a_vs_c_classifications.csv")
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
