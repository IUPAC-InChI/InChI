import subprocess
import logging
from datetime import datetime
from . import runners
from .config import (
    get_args,
    get_current_time,
    get_progress,
    DATASETS,
    TEST_TYPES,
    N_PROCESSES,
)


if __name__ == "__main__":
    test, dataset = get_args()

    subprocess.run(TEST_TYPES[test]["script"], check=True)

    exit_code = 0
    sdf_paths = DATASETS[dataset]["sdf_paths"]
    n_sdf = len(sdf_paths)
    log_path = DATASETS[dataset]["log_path"].joinpath(
        f"{datetime.now().strftime('%Y%m%dT%H%M%S')}_{test}_{dataset}.log"
    )
    logging.basicConfig(filename=log_path, encoding="utf-8", level=logging.INFO)

    logging.info(
        f"{get_current_time()}: Starting to process {n_sdf} SDFs on {N_PROCESSES} cores."
    )

    for i, sdf_path in enumerate(sdf_paths):
        try:
            exit_code = max(
                exit_code, getattr(runners, test.replace("-", "_"))(sdf_path, dataset)
            )
            logging.info(
                f"{get_progress(i + 1, n_sdf)}; Ran {test} on {sdf_path.name}."
            )
        except Exception as exception:
            logging.info(
                f"{get_progress(i + 1, n_sdf)}; Aborted {test} on {sdf_path.name} due to {type(exception).__name__}; {exception}."
            )

    raise SystemExit(exit_code)
