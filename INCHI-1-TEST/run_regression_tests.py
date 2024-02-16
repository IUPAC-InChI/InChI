import subprocess
import logging
from functools import partial
from datetime import datetime
from sdf_pipeline import drivers
from .consumers import regression_consumer
from .config import (
    INCHI_LIB_PATH,
    DATASETS,
    TEST_PATH,
    N_PROCESSES,
    get_molfile_id,
    get_dataset_arg,
    get_progress,
    get_current_time,
)


if __name__ == "__main__":
    dataset = get_dataset_arg()

    subprocess.run(f"{TEST_PATH}/compile_inchi_lib.sh", check=True)

    exit_code = 0
    sdf_paths = DATASETS[dataset]["sdf_paths"]
    log_path = DATASETS[dataset]["log_path"].joinpath(
        f"{datetime.now().strftime('%Y%m%dT%H%M%S')}_regression_test.log"
    )
    logging.basicConfig(filename=log_path, encoding="utf-8", level=logging.INFO)

    n_sdf = len(sdf_paths)
    logging.info(
        f"{get_current_time()}: Starting to process {n_sdf} SDFs on {N_PROCESSES} cores."
    )

    for i, sdf_path in enumerate(sdf_paths):
        exit_code = max(
            exit_code,
            drivers.regression(
                sdf_path=sdf_path,
                reference_path=DATASETS[dataset]["log_path"].joinpath(
                    f"{sdf_path.stem}.regression_reference.sqlite"
                ),
                consumer_function=partial(
                    regression_consumer, inchi_lib_path=INCHI_LIB_PATH
                ),
                get_molfile_id=get_molfile_id(sdf_path),
                number_of_consumer_processes=N_PROCESSES,
            ),
        )
        logging.info(
            f"{get_progress(i + 1, n_sdf)}; Ran regression tests on {sdf_path.name}."
        )

    raise SystemExit(exit_code)
