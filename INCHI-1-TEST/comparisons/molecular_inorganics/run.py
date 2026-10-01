"""One pass of the comparison: compute or compare raw results over a dataset.

The shared `inchi_tests/run_tests.py` compares byte-for-byte against the committed
references. A comparison pass differs in four ways, which is why it has its own
runner rather than flags on the shared one:

- results are stored raw, prefix included (`consumers.raw_regression_consumer`);
- references are namespaced by `--run-tag`, so a pass never reads or overwrites
  the committed `<sdf>.regression_reference.sqlite`;
- logs are namespaced by `--log-tag`, so the three passes can be told apart;
- `--compare=prefix-insensitive` swaps in `PrefixInsensitiveComparator` and logs
  its tallies as a `Comparison summary` line, which `classify.py` and `report.py`
  read back.

Run as a module from `INCHI-1-TEST/comparisons`, e.g.
`python -m molecular_inorganics.run --test=regression ...`."""

import argparse
import importlib
import json
import logging
import multiprocessing
import os
import re
import sys
from datetime import datetime
from functools import partial
from pathlib import Path
from sdf_pipeline import drivers
from inchi_tests.comparators import PrefixInsensitiveComparator
from inchi_tests.utils import PathValidator, get_current_time, get_progress
from molecular_inorganics.consumers import raw_regression_consumer


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


def select_comparator(test: str, compare: str) -> PrefixInsensitiveComparator | None:
    """The comparison rule for a pass, decided by `--compare` alone.

    Leniency is opted into explicitly. The control pass -- same build, no options,
    expected to reproduce its reference -- reads the same tagged reference as the
    option pass, but it must stay byte-for-byte: a prefix-insensitive comparison
    cannot see a changed prefix, InChIKey or warning level, which is exactly the
    drift it exists to catch. `None` leaves the driver on its byte-for-byte default.

    Only `regression` compares anything; a reference pass would otherwise write an
    all-zero summary that `classify.parse_comparison_summary` would happily read."""
    if compare == "prefix-insensitive" and test == "regression":
        return PrefixInsensitiveComparator()

    return None


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
        help="Namespace for the log file, e.g. 'run_b_dev_mi'. Distinct per pass.",
    )
    parser.add_argument(
        "--inchi-api-parameters",
        type=str,
        default="",
        help=(
            "Space-separated InChI options passed to the library. Must be given in "
            "'--inchi-api-parameters=-MolecularInorganics' form: a space-separated "
            "value beginning with '-' is parsed as an unknown option."
        ),
    )
    parser.add_argument(
        "--compare",
        type=str,
        default="exact",
        choices=["exact", "prefix-insensitive"],
        help=(
            "How a result is compared against its reference. 'exact' is "
            "byte-for-byte and is what the control pass needs. 'prefix-insensitive' "
            "compares the InChI body only, for passes whose options change the "
            "prefix by design."
        ),
    )
    parser.add_argument(
        "--timeout-seconds-per-molfile",
        type=int,
        default=60,
        help="How long one molfile may occupy a consumer before it is killed.",
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
    comparator = select_comparator(test, args.compare)
    consumer = partial(
        raw_regression_consumer,
        inchi_lib_path=args.lib_path,
        inchi_api_parameters=args.inchi_api_parameters,
    )

    logging.basicConfig(filename=log_path, encoding="utf-8", level=logging.INFO)
    logging.info(f"{get_current_time()}: Using '{args.lib_path}'.")
    logging.info(f"{get_current_time()}: InChI options: '{args.inchi_api_parameters}'.")
    logging.info(f"{get_current_time()}: Reference tag: '{args.run_tag}'.")
    logging.info(f"{get_current_time()}: Comparison: '{args.compare}'.")
    logging.info(
        f"{get_current_time()}: Timeout per molfile: {args.timeout_seconds_per_molfile}s."
    )
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
                    exit_code = max(
                        exit_code,
                        drivers.regression(
                            sdf_path=sdf_path,
                            reference_path=reference_path,
                            consumer_function=consumer,
                            get_molfile_id=get_molfile_id,
                            number_of_consumer_processes=n_processes,
                            expected_failures=expected_failures,
                            timeout_seconds_per_molfile=args.timeout_seconds_per_molfile,
                            compare=comparator,
                        ),
                    )

                case "regression-reference":
                    if reference_path.exists():
                        logging.info(f"Not re-computing reference for {sdf_path.name}.")

                        continue

                    # Written under a temporary name and renamed once complete, so
                    # an aborted pass leaves no reference that the existence check
                    # above would mistake for a finished one.
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
                            timeout_seconds_per_molfile=args.timeout_seconds_per_molfile,
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
