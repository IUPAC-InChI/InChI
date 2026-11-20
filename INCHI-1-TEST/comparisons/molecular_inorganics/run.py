"""One pass of the comparison: compute or compare raw results over a dataset.

The shared `inchi_tests/run_tests.py` compares byte-for-byte against the committed
references. A comparison pass differs in four ways, which is why it has its own
runner rather than options on the shared one:

- results are stored raw, prefix included (`consumers.raw_regression_consumer`),
  and a reference storing other fields is refused (`check_reference_format`);
- references are namespaced by `--run-tag`, so a pass never reads or overwrites
  the committed `<sdf>.regression_reference.sqlite`;
- logs are namespaced by `--log-tag`, so the three passes can be told apart;
- a regression pass compares with a rule from `comparators.py`, chosen by
  `--compare`, and logs the rule's counts as a `Comparison summary` line, which
  `classify.py` and `report.py` read.

Run as a module from `INCHI-1-TEST/comparisons`, e.g.
`python -m molecular_inorganics.run --test=regression ...`."""

import argparse
import importlib
import json
import logging
import multiprocessing
import os
import re
import sqlite3
import sys
from datetime import datetime
from functools import partial
from pathlib import Path
from typing import Callable
from sdf_pipeline import core, drivers, logger
from inchi_tests.utils import PathValidator, get_current_time, get_progress
from molecular_inorganics.comparators import LegacyComparator, OptionComparator
from molecular_inorganics.consumers import RESULT_FIELDS, raw_regression_consumer


TAG_PATTERN = re.compile(r"^[A-Za-z0-9_]+$")


class TagValidator(argparse.Action):
    def __call__(self, parser, namespace, values, option_string=None):
        if not TAG_PATTERN.match(values):
            parser.error(
                f"{option_string}: '{values}' must be non-empty and contain only letters, digits, and underscores."
            )
        setattr(namespace, self.dest, values)


def reference_filename(sdf_stem: str, run_tag: str) -> str:
    return f"{sdf_stem}.{run_tag}.regression_reference.sqlite"


def log_filename(timestamp: str, test: str, dataset: str, log_tag: str) -> str:
    return f"{timestamp}_{test}_{dataset}.{log_tag}.log"


def check_reference_format(reference_path: Path) -> None:
    """Refuse a reference whose rows do not store `RESULT_FIELDS`.

    A reference written before the stored fields last changed would otherwise be
    used as is: the reference pass skips any SDF whose reference file exists, and
    the comparisons would then mismatch on every structure or compare without the
    fields they report on. Rows for molfiles that timed out store
    `timeout_seconds` instead and are skipped."""
    with sqlite3.connect(reference_path) as reference_db:
        for (result,) in reference_db.execute("SELECT result FROM results"):
            fields = set(json.loads(result))
            if fields == {"timeout_seconds"}:
                continue
            if fields != RESULT_FIELDS:
                raise ValueError(
                    f"{reference_path.name} stores {sorted(fields)}, not "
                    f"{sorted(RESULT_FIELDS)}. It was written by an older version of "
                    "the comparison; delete it and re-run the reference pass."
                )
            return


COMPARATORS = {
    "prefix-insensitive": OptionComparator,
    "except-prefix": LegacyComparator,
}


def select_comparator(compare: str) -> OptionComparator | LegacyComparator:
    """The comparison rule for a regression pass, named by `--compare`."""
    return COMPARATORS[compare]()


def compare_with_reference(
    sdf_path: Path,
    reference_path: Path,
    consumer_function: Callable,
    get_molfile_id: Callable,
    number_of_consumer_processes: int,
    expected_failures: set[str],
    comparator: Callable[[dict, dict], bool],
) -> int:
    """Compare every molfile's result with the reference; return the exit code.

    The same as `sdf_pipeline.drivers.regression`, with the comparison done by
    `comparator` instead of byte-for-byte, and the same log lines, which
    `classify.py` parses."""
    exit_code = 0
    processed_molfile_ids = set()

    with sqlite3.connect(reference_path) as reference_db:
        for consumer_result in core.run(
            sdf_path=sdf_path,
            consumer_function=partial(consumer_function, get_molfile_id=get_molfile_id),
            number_of_consumer_processes=number_of_consumer_processes,
            get_molfile_id=get_molfile_id,
        ):
            molfile_id = consumer_result.molfile_id
            expected_failure = molfile_id in expected_failures
            processed_molfile_ids.add(molfile_id)

            if isinstance(consumer_result, core.TimedOut):
                logger.info(
                    f"timed out{' expectedly' if expected_failure else ''}:"
                    + json.dumps(
                        {
                            "molfile_id": molfile_id,
                            "sdf": sdf_path.name,
                            "seconds": round(consumer_result.seconds, 1),
                        }
                    )
                )
                if not expected_failure:
                    exit_code = 1
                continue

            reference_query = reference_db.execute(
                "SELECT result FROM results WHERE molfile_id = ?", (molfile_id,)
            ).fetchone()
            assert reference_query, f"Couldn't find molfile ID {molfile_id} in reference."
            reference_result = reference_query[0]

            if comparator(consumer_result.result, json.loads(reference_result)):
                continue

            if not expected_failure:
                exit_code = 1
            logger.info(
                f"regression test failed{' expectedly' if expected_failure else ''}:"
                + json.dumps(
                    {
                        "time": consumer_result.time,
                        "molfile_id": molfile_id,
                        "sdf": sdf_path.name,
                        "info": dict(consumer_result.info),
                        "diff": {
                            "current": json.dumps(consumer_result.result),
                            "reference": reference_result,
                        },
                    }
                )
            )

        reference_ids = {
            row[0] for row in reference_db.execute("SELECT molfile_id FROM results")
        }

    unprocessed = reference_ids - processed_molfile_ids
    assert (
        not unprocessed
    ), f"Reference contains molfile IDs that haven't been processed: {unprocessed}."

    return exit_code


def get_args(argv: list[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Run one pass of the MolecularInorganics comparison.",
    )
    parser.add_argument(
        "--test",
        type=str,
        required=True,
        choices=["regression", "regression-reference"],
    )
    parser.add_argument(
        "--lib-path",
        type=str,
        required=True,
        action=PathValidator,
        help="Path to the InChI library this pass uses.",
    )
    parser.add_argument(
        "--data-config",
        type=str,
        required=True,
        action=PathValidator,
        help="Path to a dataset configuration file.",
    )
    parser.add_argument(
        "--run-tag",
        type=str,
        required=True,
        action=TagValidator,
        help=(
            "Namespace for the reference files, e.g. 'ref_v1_07_5'. Passes compared "
            "against the same reference share this tag."
        ),
    )
    parser.add_argument(
        "--log-tag",
        type=str,
        required=True,
        action=TagValidator,
        help="Namespace for the log file, e.g. 'run_b_mi'. Distinct per pass.",
    )
    parser.add_argument(
        "--inchi-api-parameters",
        type=str,
        default="",
        help=(
            "Space-separated InChI options passed to the library. Must be given in "
            "'--inchi-api-parameters=-LegacyMetalHandling' form: a space-separated "
            "value beginning with '-' is parsed as an unknown option."
        ),
    )
    parser.add_argument(
        "--compare",
        type=str,
        default="prefix-insensitive",
        choices=sorted(COMPARATORS),
        help=(
            "How a regression pass compares a result with the reference. "
            "'prefix-insensitive' compares the InChI body and the failure state "
            "(Run B). 'except-prefix' compares every field except the prefix and "
            "the InChIKey flags (Run C)."
        ),
    )

    return parser.parse_args(argv)


def main(args: argparse.Namespace, data_config) -> None:
    test = args.test
    dataset = data_config.name
    data_path = data_config.path
    sdf_paths = data_config.sdf_paths
    get_molfile_id = data_config.molfile_id_getter
    expected_failures = data_config.expected_failures.get(test, set())
    n_sdf = len(sdf_paths)
    log_path = data_path.joinpath(
        log_filename(
            datetime.now().strftime("%Y%m%dT%H%M%S"), test, dataset, args.log_tag
        )
    )
    n_processes = os.cpu_count() or 8
    comparator = select_comparator(args.compare) if test == "regression" else None
    consumer = partial(
        raw_regression_consumer,
        inchi_lib_path=args.lib_path,
        inchi_api_parameters=args.inchi_api_parameters,
    )

    logging.basicConfig(filename=log_path, encoding="utf-8", level=logging.INFO)
    logging.info(f"{get_current_time()}: Using '{args.lib_path}'.")
    logging.info(f"{get_current_time()}: InChI options: '{args.inchi_api_parameters}'.")
    logging.info(f"{get_current_time()}: Reference tag: '{args.run_tag}'.")
    if comparator is not None:
        logging.info(f"{get_current_time()}: Comparison: '{args.compare}'.")
    logging.info(
        f"{get_current_time()}: Starting to process {n_sdf} SDFs on {n_processes} cores."
    )

    exit_code = 0

    for i, sdf_path in enumerate(sdf_paths):
        reference_path = data_path.joinpath(
            reference_filename(sdf_path.stem, args.run_tag)
        )
        try:
            match test:
                case "regression":
                    check_reference_format(reference_path)
                    exit_code = max(
                        exit_code,
                        compare_with_reference(
                            sdf_path=sdf_path,
                            reference_path=reference_path,
                            consumer_function=consumer,
                            get_molfile_id=get_molfile_id,
                            number_of_consumer_processes=n_processes,
                            expected_failures=expected_failures,
                            comparator=comparator,
                        ),
                    )

                case "regression-reference":
                    if reference_path.exists():
                        check_reference_format(reference_path)
                        logging.info(f"Not re-computing reference for {sdf_path.name}.")

                        continue

                    # Written under a temporary name and renamed when complete, so
                    # a pass that stopped while writing leaves no file that the
                    # check above would take for a complete reference.
                    partial_path = reference_path.with_name(
                        reference_path.name + ".partial"
                    )
                    partial_path.unlink(missing_ok=True)
                    exit_code = max(
                        exit_code,
                        drivers.regression_reference(
                            sdf_path=sdf_path,
                            reference_path=partial_path,
                            consumer_function=consumer,
                            get_molfile_id=get_molfile_id,
                            number_of_consumer_processes=n_processes,
                        ),
                    )
                    partial_path.rename(reference_path)

            logging.info(
                f"{get_progress(i + 1, n_sdf)}; Ran {test} on {sdf_path.name}."
            )

        except Exception as exception:
            logging.error(
                f"{get_progress(i + 1, n_sdf)}; Aborted {test} on {sdf_path.name} due to {type(exception).__name__}; {exception}."
            )

    if comparator is not None:
        logging.info(
            f"{get_current_time()}: Comparison summary: "
            f"{json.dumps(comparator.summary())}"
        )

    raise SystemExit(exit_code)


if __name__ == "__main__":
    # Initialize processes consistently across platforms.
    # See https://docs.python.org/3/library/multiprocessing.html#contexts-and-start-methods.
    multiprocessing.set_start_method("spawn")

    args = get_args()
    # https://docs.python.org/3/library/importlib.html#importing-a-source-file-directly
    sys.path.append(str(Path(args.data_config).parent))
    data_config = importlib.import_module(str(Path(args.data_config).stem))

    main(args, data_config.config)
