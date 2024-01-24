import re
import os
import argparse
from typing import Final, Callable
from pathlib import Path
from datetime import datetime


def get_molfile_id(sdf_path: Path) -> Callable:
    if "mcule" in sdf_path.stem:
        return _get_mcule_id
    elif "inchi" in sdf_path.stem:
        return _get_inchi_id
    return _get_pubchem_id


def _get_inchi_id(molfile: str) -> str:
    molfile_id = molfile.splitlines()[-3].strip()

    return molfile_id


def _get_mcule_id(molfile: str) -> str:
    molfile_id_pattern = re.compile(r"<Mcule_ID>(.*?)>", re.DOTALL)
    molfile_id_match = molfile_id_pattern.search(molfile)
    molfile_id = molfile_id_match.group(1).strip() if molfile_id_match else ""

    return molfile_id


def _get_pubchem_id(molfile: str) -> str:
    return molfile.split()[0].strip()


def get_dataset_arg() -> str:
    parser = argparse.ArgumentParser(
        description="Choose a dataset from {ci, pubchem_compound, pubchem_compound3d, pubchem_substance}",
    )
    parser.add_argument(
        "dataset",
        choices=["ci", "pubchem_compound", "pubchem_compound3d", "pubchem_substance"],
        type=str,
        nargs=1,
        help="Choose a dataset from {ci, pubchem_compound, pubchem_compound3d, pubchem_substance}",
    )
    (dataset,) = parser.parse_args().dataset

    return dataset


def get_current_time():
    return datetime.now().isoformat(timespec="seconds")


def get_progress(current: int, total: int) -> str:
    return f"{get_current_time()}: Processed {current}/{total} ({current / total * 100:.2f}%) SDFs"


INCHI_API_PARAMETERS: Final[str] = ""
TEST_PATH: Final[Path] = Path(__file__).parent.absolute()
INCHI_LIB_PATH: Final[Path] = TEST_PATH.joinpath("libinchi.so.dev")
INCHI_REFERENCE_LIB_PATH: Final[Path] = TEST_PATH.joinpath("libinchi.so.1.06.00")
N_PROCESSES: Final[int] = len(
    os.sched_getaffinity(0)
)  # https://docs.python.org/3/library/multiprocessing.html#multiprocessing.cpu_count

DATASETS: Final[dict] = {
    "ci": {
        "sdf_paths": sorted(TEST_PATH.joinpath("data/ci").glob("*.sdf.gz")),
        "log_path": TEST_PATH.joinpath("data/ci/"),
    },
    "pubchem_compound": {
        "sdf_paths": sorted(
            TEST_PATH.joinpath("data/pubchem/compound").glob("*.sdf.gz")
        ),
        "log_path": TEST_PATH.joinpath("data/pubchem/compound"),
        "download_path": "Compound/CURRENT-Full",
    },
    "pubchem_compound3d": {
        "sdf_paths": sorted(
            TEST_PATH.joinpath("data/pubchem/compound3d").glob("*.sdf.gz")
        ),
        "log_path": TEST_PATH.joinpath("data/pubchem/compound3d"),
        "download_path": "Compound_3D/01_conf_per_cmpd",
    },
    "pubchem_substance": {
        "sdf_paths": sorted(
            TEST_PATH.joinpath("data/pubchem/substance").glob("*.sdf.gz")
        ),
        "log_path": TEST_PATH.joinpath("data/pubchem/substance"),
        "download_path": "Substance/CURRENT-Full",
    },
}
