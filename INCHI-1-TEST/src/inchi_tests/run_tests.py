import subprocess
import logging
from functools import partial
from datetime import datetime
from pathlib import Path
from sdf_pipeline import drivers
from typing import cast
from inchi_tests.utils import get_current_time, get_progress, get_config_args
from inchi_tests.config_models import load_config, TestConfig, DataConfig
from inchi_tests.consumers import regression_consumer, invariance_consumer


def main() -> None:

    test_config_path, dataset_config_path = get_config_args()

    test_config = load_config("test_config", Path(test_config_path))
    data_config = load_config("data_config", Path(dataset_config_path))

    if not isinstance(test_config, TestConfig):
        raise TypeError(f"Expected TestConfig, got {type(test_config)}.")

    if not isinstance(data_config, DataConfig):
        raise TypeError(f"Expected DataConfig, got {type(data_config)}.")

    test_config = cast(TestConfig, test_config)
    data_config = cast(DataConfig, data_config)

    test = test_config.name
    dataset = data_config.name

    data_path = data_config.path
    sdf_paths = data_config.sdf_paths
    get_molfile_id = data_config.molfile_id_getter
    expected_failures = data_config.expected_failures.get(test, set())
    n_sdf = len(sdf_paths)
    log_path = data_path.joinpath(
        f"{datetime.now().strftime('%Y%m%dT%H%M%S')}_{test}_{dataset}.log"
    )
    n_processes = test_config.n_processes
    inchi_version = test_config.inchi_version
    inchi_api_parameters = test_config.inchi_api_parameters
    inchi_lib_dir = Path("INCHI-1-TEST/libs")
    inchi_lib_path = inchi_lib_dir.joinpath(f"libinchi.so.{inchi_version}")
    if not inchi_lib_path.exists():
        logging.error(f"libinchi.so.{inchi_version} not found in {inchi_lib_dir}.")
        raise SystemExit(1)

    gcc_version = subprocess.run(
        "gcc -dumpversion",
        shell=True,
        check=True,
        capture_output=True,
        text=True,
    ).stdout.strip()

    logging.basicConfig(filename=log_path, encoding="utf-8", level=logging.INFO)
    logging.info(
        f"{get_current_time()}: InChI '{inchi_version}' compiled with GCC {gcc_version}."
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
                                inchi_api_parameters=inchi_api_parameters,
                            ),
                            get_molfile_id=get_molfile_id,
                            number_of_consumer_processes=n_processes,
                            expected_failures=expected_failures,
                        ),
                    )

                case "regression-reference":
                    reference_path = data_path.joinpath(
                        f"{sdf_path.stem}.regression_reference.sqlite"
                    )
                    if reference_path.exists():
                        logging.info(f"Not re-computing reference for {sdf_path.name}.")

                        continue

                    exit_code = max(
                        exit_code,
                        drivers.regression_reference(
                            sdf_path=sdf_path,
                            reference_path=reference_path,
                            consumer_function=partial(
                                regression_consumer,
                                inchi_lib_path=inchi_lib_path,
                                inchi_api_parameters=inchi_api_parameters,
                            ),
                            get_molfile_id=get_molfile_id,
                            number_of_consumer_processes=n_processes,
                        ),
                    )

                case "invariance":
                    exit_code = max(
                        exit_code,
                        drivers.invariance(
                            sdf_path=sdf_path,
                            consumer_function=partial(
                                invariance_consumer,
                                inchi_lib_path=inchi_lib_path,
                                inchi_api_parameters=inchi_api_parameters,
                                n_invariance_runs=test_config.n_invariance_runs,
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

    raise SystemExit(exit_code)


if __name__ == "__main__":
    main()
