import os
import json
import logging
import multiprocessing
import sys
import importlib
from functools import partial
from datetime import datetime
from pathlib import Path
from sdf_pipeline import drivers
from inchi_tests.utils import (
    get_current_time,
    get_progress,
    get_config_args,
    reference_filename,
    log_filename,
)
from inchi_tests.consumers import (
    regression_consumer,
    invariance_consumer,
    campaign_regression_consumer,
)
from inchi_tests.comparators import PrefixInsensitiveComparator


def select_comparator(test: str, compare: str) -> PrefixInsensitiveComparator | None:
    """The comparison rule for a run, decided by `--compare` alone.

    Leniency is opted into explicitly and is deliberately not implied by
    `--run-tag`. A control run -- same build, no options, expected to reproduce
    its reference -- is tagged too, because it reads the tagged reference, but it
    must stay byte-for-byte: a prefix-insensitive comparison cannot see a changed
    prefix, InChIKey or warning level, which is exactly the drift it exists to
    catch. `None` leaves the driver on its byte-for-byte default.

    Only the `regression` test compares anything; a reference run would otherwise
    write an all-zero summary that `parse_comparison_summary` would happily read."""
    if compare == "prefix-insensitive" and test == "regression":
        return PrefixInsensitiveComparator()

    return None


def main(
    test,
    inchi_lib_path,
    data_config,
    inchi_api_parameters="",
    run_tag="",
    log_tag="",
    compare="exact",
) -> None:
    dataset = data_config.name

    data_path = data_config.path
    sdf_paths = data_config.sdf_paths
    get_molfile_id = data_config.molfile_id_getter
    expected_failures = data_config.expected_failures.get(test, set())
    n_sdf = len(sdf_paths)
    log_path = data_path.joinpath(
        log_filename(datetime.now().strftime("%Y%m%dT%H%M%S"), test, dataset, log_tag)
    )
    n_processes = os.cpu_count() or 8
    # A tagged run is a campaign run: store raw results, prefix included, so that
    # what is compared is a choice made at comparison time rather than baked into
    # the reference.
    consumer = campaign_regression_consumer if run_tag else regression_consumer
    comparator = select_comparator(test, compare)

    logging.basicConfig(filename=log_path, encoding="utf-8", level=logging.INFO)
    logging.info(f"{get_current_time()}: Using '{inchi_lib_path}'.")
    logging.info(f"{get_current_time()}: InChI options: '{inchi_api_parameters}'.")
    logging.info(f"{get_current_time()}: Reference tag: '{run_tag}'.")
    logging.info(f"{get_current_time()}: Comparison: '{compare}'.")
    logging.info(
        f"{get_current_time()}: Starting to process {n_sdf} SDFs on {n_processes} cores."
    )

    exit_code = 0

    for i, sdf_path in enumerate(sdf_paths):
        try:
            match test:
                case "regression":
                    exit_code = max(
                        exit_code,
                        drivers.regression(
                            sdf_path=sdf_path,
                            reference_path=data_path.joinpath(
                                reference_filename(sdf_path.stem, run_tag)
                            ),
                            consumer_function=partial(
                                consumer,
                                inchi_lib_path=inchi_lib_path,
                                inchi_api_parameters=inchi_api_parameters,
                            ),
                            get_molfile_id=get_molfile_id,
                            number_of_consumer_processes=n_processes,
                            expected_failures=expected_failures,
                            compare=comparator,
                        ),
                    )

                case "regression-reference":
                    reference_path = data_path.joinpath(
                        reference_filename(sdf_path.stem, run_tag)
                    )
                    if reference_path.exists():
                        logging.info(f"Not re-computing reference for {sdf_path.name}.")

                        continue

                    partial_path = reference_path.with_name(
                        reference_path.name + ".partial"
                    )
                    partial_path.unlink(missing_ok=True)
                    exit_code = max(
                        exit_code,
                        drivers.regression_reference(
                            sdf_path=sdf_path,
                            reference_path=partial_path,
                            consumer_function=partial(
                                consumer,
                                inchi_lib_path=inchi_lib_path,
                                inchi_api_parameters=inchi_api_parameters,
                            ),
                            get_molfile_id=get_molfile_id,
                            number_of_consumer_processes=n_processes,
                        ),
                    )
                    partial_path.rename(reference_path)

                case "invariance":
                    exit_code = max(
                        exit_code,
                        drivers.invariance(
                            sdf_path=sdf_path,
                            consumer_function=partial(
                                invariance_consumer,
                                inchi_lib_path=inchi_lib_path,
                                inchi_api_parameters=inchi_api_parameters,
                                n_invariance_runs=10,
                            ),
                            get_molfile_id=get_molfile_id,
                            number_of_consumer_processes=n_processes,
                            expected_failures=expected_failures,
                        ),
                    )

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
    # Note that this file must be run as a script, i.e., `python run_tests.py`.

    # Initialize processes consistently across platforms.
    # Otherwise, the default is "fork" on non-macOS-Unix and "spawn" on macOS and Windows.
    # See https://docs.python.org/3/library/multiprocessing.html#contexts-and-start-methods.
    multiprocessing.set_start_method("spawn")

    (
        test,
        inchi_lib_path,
        dataset_config_path,
        inchi_api_parameters,
        run_tag,
        log_tag,
        compare,
    ) = get_config_args()
    sys.path.append(str(Path(dataset_config_path).parent))
    data_config = importlib.import_module(str(Path(dataset_config_path).stem))

    main(
        test,
        inchi_lib_path,
        data_config.config,
        inchi_api_parameters,
        run_tag,
        log_tag,
        compare,
    )
