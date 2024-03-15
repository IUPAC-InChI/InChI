import subprocess
import logging
from functools import partial
from datetime import datetime
from sdf_pipeline import drivers
from .consumers import invariance_consumer
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
    subprocess.run(f"{TEST_PATH}/compile_inchi_lib.sh", check=True)

    exit_code = 0
    dataset = get_dataset_arg()

    sdf_paths = DATASETS[dataset]["sdf_paths"]
    n_sdf = len(sdf_paths)
    log_path = DATASETS[dataset]["log_path"].joinpath(
        f"{datetime.now().strftime('%Y%m%dT%H%M%S')}_invariance_test.log"
    )
    logging.basicConfig(filename=log_path, encoding="utf-8", level=logging.INFO)

    logging.info(
        f"{get_current_time()}: Starting to process {n_sdf} SDFs on {N_PROCESSES} cores."
    )
    for i, sdf_path in enumerate(sdf_paths):
        exit_code = max(
            exit_code,
            drivers.invariance(
                sdf_path=sdf_path,
                consumer_function=partial(
                    invariance_consumer, inchi_lib_path=INCHI_LIB_PATH
                ),
                get_molfile_id=get_molfile_id(sdf_path),
                number_of_consumer_processes=N_PROCESSES,
            ),
        )
        logging.info(
            f"{get_progress(i + 1, n_sdf)}; Ran invariance tests on {sdf_path.name}."
        )

    raise SystemExit(exit_code)
