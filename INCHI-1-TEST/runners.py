import logging
from pathlib import Path
from functools import partial
from sdf_pipeline import drivers
from .consumers import regression_consumer, invariance_consumer
from .config import (
    DATASETS,
    N_PROCESSES,
    INCHI_LIB_PATH,
    INCHI_REFERENCE_LIB_PATH,
)


def regression(sdf_path: Path, dataset: str) -> int:
    exit_code = drivers.regression(
        sdf_path=sdf_path,
        reference_path=DATASETS[dataset]["log_path"].joinpath(
            f"{sdf_path.stem}.regression_reference.sqlite"
        ),
        consumer_function=partial(regression_consumer, inchi_lib_path=INCHI_LIB_PATH),
        get_molfile_id=DATASETS[dataset]["molfile_id"],
        number_of_consumer_processes=N_PROCESSES,
    )

    return exit_code


def regression_reference(sdf_path: Path, dataset: str) -> int:
    reference_path = DATASETS[dataset]["log_path"].joinpath(
        f"{sdf_path.stem}.regression_reference.sqlite"
    )
    if reference_path.exists():
        logging.info(f"Not re-computing reference for {sdf_path.name}.")

        return 0

    exit_code = drivers.regression_reference(
        sdf_path=sdf_path,
        reference_path=reference_path,
        consumer_function=partial(
            regression_consumer, inchi_lib_path=INCHI_REFERENCE_LIB_PATH
        ),
        get_molfile_id=DATASETS[dataset]["molfile_id"],
        number_of_consumer_processes=N_PROCESSES,
    )

    return exit_code


def invariance(sdf_path: Path, dataset: str) -> int:
    exit_code = drivers.invariance(
        sdf_path=sdf_path,
        consumer_function=partial(invariance_consumer, inchi_lib_path=INCHI_LIB_PATH),
        get_molfile_id=DATASETS[dataset]["molfile_id"],
        number_of_consumer_processes=N_PROCESSES,
    )

    return exit_code
