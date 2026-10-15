import os
import logging
import multiprocessing
import sys
import importlib
from functools import partial
from datetime import datetime
from pathlib import Path
from sdf_pipeline import drivers
from inchi_tests.utils import get_current_time, get_progress, get_config_args
from inchi_tests.consumers import regression_consumer, invariance_consumer


def main(test, inchi_lib_path, data_config, timeout_seconds_per_molfile=60) -> None:
    dataset = data_config.name

    data_path = data_config.path
    sdf_paths = data_config.sdf_paths
    get_molfile_id = data_config.molfile_id_getter
    expected_failures = data_config.expected_failures.get(test, set())
    n_sdf = len(sdf_paths)
    log_path = data_path.joinpath(
        f"{datetime.now().strftime('%Y%m%dT%H%M%S')}_{test}_{dataset}.log"
    )
    n_processes = os.cpu_count() or 8

    logging.basicConfig(filename=log_path, encoding="utf-8", level=logging.INFO)
    logging.info(f"{get_current_time()}: Using '{inchi_lib_path}'.")
    logging.info(
        f"{get_current_time()}: Timeout per molfile: {timeout_seconds_per_molfile}s."
    )
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
                                f"{sdf_path.stem}.regression_reference.sqlite"
                            ),
                            consumer_function=partial(
                                regression_consumer,
                                inchi_lib_path=inchi_lib_path,
                                inchi_api_parameters="",
                            ),
                            get_molfile_id=get_molfile_id,
                            number_of_consumer_processes=n_processes,
                            expected_failures=expected_failures,
                            timeout_seconds_per_molfile=timeout_seconds_per_molfile,
                        ),
                    )

                case "regression-reference":
                    reference_path = data_path.joinpath(
                        f"{sdf_path.stem}.regression_reference.sqlite"
                    )
                    if reference_path.exists():
                        logging.info(f"Not re-computing reference for {sdf_path.name}.")

                        continue

                    # Written under a temporary name and renamed once complete, so
                    # an aborted run leaves no reference that the existence check
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
                            consumer_function=partial(
                                regression_consumer,
                                inchi_lib_path=inchi_lib_path,
                                inchi_api_parameters="",
                            ),
                            get_molfile_id=get_molfile_id,
                            number_of_consumer_processes=n_processes,
                            timeout_seconds_per_molfile=timeout_seconds_per_molfile,
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
                                inchi_api_parameters="",
                                n_invariance_runs=10,
                            ),
                            get_molfile_id=get_molfile_id,
                            number_of_consumer_processes=n_processes,
                            expected_failures=expected_failures,
                            timeout_seconds_per_molfile=timeout_seconds_per_molfile,
                        ),
                    )

            logging.info(
                f"{get_progress(i + 1, n_sdf)}; Ran {test} on {sdf_path.name}."
            )

        except Exception as exception:
            logging.error(
                f"{get_progress(i + 1, n_sdf)}; Aborted {test} on {sdf_path.name} due to {type(exception).__name__}; {exception}."
            )

    raise SystemExit(exit_code)


if __name__ == "__main__":
    # Note that this file must be run as a script, i.e., `python run_tests.py`.

    # Initialize processes consistently across platforms.
    # Otherwise, the default is "fork" on non-macOS-Unix and "spawn" on macOS and Windows.
    # See https://docs.python.org/3/library/multiprocessing.html#contexts-and-start-methods.
    multiprocessing.set_start_method("spawn")

    test, inchi_lib_path, dataset_config_path, timeout_seconds_per_molfile = (
        get_config_args()
    )
    # https://docs.python.org/3/library/importlib.html#importing-a-source-file-directly
    sys.path.append(str(Path(dataset_config_path).parent))
    data_config = importlib.import_module(str(Path(dataset_config_path).stem))

    main(test, inchi_lib_path, data_config.config, timeout_seconds_per_molfile)
