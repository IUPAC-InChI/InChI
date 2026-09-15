"""Analysis tooling for the PubChem MolecularInorganics regression campaign.

Three runs over the same PubChem SDFs:

  A (reference) v1.07.5, no options
  B             dev, -MolecularInorganics
  C             dev, no options

Results are stored raw, prefixes included. Comparison ignores the
version-and-kind prefix (`comparators.PrefixInsensitiveComparator`), and every
A-vs-B mismatch is re-checked against v1.07.5 `-RecMet` by comparing that run's
reconnected (`/r`) layer with the `-MolecularInorganics` InChI minus its prefix.
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
from inchi_tests.consumers import (
    campaign_regression_consumer,
    regression_consumer,
    inchi_body,
    is_failed,
)

_FAILURE_PATTERN = re.compile(
    r"^INFO:sdf_pipeline:regression test failed( expectedly)?:(?P<entry>\{.*\})$"
)
_SUMMARY_PATTERN = re.compile(r"Comparison summary: (?P<counts>\{.*\})$")


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
    consumer: Callable = campaign_regression_consumer,
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
    if recmet_result is None:
        return "recmet_missing"
    if is_failed(recmet_result):
        return "recmet_failed"
    if reconnected_layer(recmet_result["inchi"]) == inchi_body(
        mismatch.current["inchi"]
    ):
        return "recmet_equivalent"

    return "novel"


def classify_mismatches(
    mismatches: list[Mismatch], recmet_results: dict[str, dict]
) -> list[Classification]:
    classifications: list[Classification] = []

    for mismatch in mismatches:
        recmet_result = recmet_results.get(mismatch.molfile_id)
        classifications.append(
            Classification(
                molfile_id=mismatch.molfile_id,
                sdf=mismatch.sdf,
                category=_categorize(mismatch, recmet_result),
                reference_inchi=mismatch.reference["inchi"],
                dev_mi_inchi=mismatch.current["inchi"],
                recmet_inchi=recmet_result["inchi"] if recmet_result else "",
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
                "total": len(classifications),
                "comparison": comparison_summary or {},
            },
            json_file,
            indent=2,
        )


def explain_failures(
    classifications: list[Classification],
    sdf_paths: list[Path],
    mi_lib_path: str,
    get_molfile_id: Callable,
) -> dict[str, str]:
    """Recover the InChI `message` for the `error_under_mi` subset.

    The campaign consumer omits `message`, so a failure under MI has no stated
    reason. That subset is small enough to re-run with the full consumer."""
    failures = [c for c in classifications if c.category == "error_under_mi"]
    if not failures:
        return {}

    ids_by_sdf: dict[str, set[str]] = defaultdict(set)
    for classification in failures:
        ids_by_sdf[classification.sdf].add(classification.molfile_id)

    results = recompute_subset(
        sdf_paths=sdf_paths,
        ids_by_sdf=dict(ids_by_sdf),
        inchi_lib_path=mi_lib_path,
        inchi_api_parameters="-MolecularInorganics",
        get_molfile_id=get_molfile_id,
        consumer=regression_consumer,
    )

    return {molfile_id: result["message"] for molfile_id, result in results.items()}


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
    parser.add_argument("--mi-lib-path", required=True, type=str, help="dev libinchi.so")
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

    messages = explain_failures(
        classifications,
        data_config.sdf_paths,
        args.mi_lib_path,
        data_config.molfile_id_getter,
    )
    if messages:
        with open(
            output_dir.joinpath("error_under_mi_messages.json"), "w", encoding="utf-8"
        ) as json_file:
            json.dump(messages, json_file, indent=2)

    print(json.dumps(classification_counts(classifications), indent=2))


if __name__ == "__main__":
    main()
