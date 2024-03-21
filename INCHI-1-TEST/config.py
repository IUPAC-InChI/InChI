import re
import os
import argparse
from typing import Final, Callable, Generator, Any
from pathlib import Path
from datetime import datetime
from sdf_pipeline.core import read_records_from_gzipped_sdf


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


def get_args() -> tuple[str, str]:
    parser = argparse.ArgumentParser(
        description="Choose a test and dataset.",
    )
    parser.add_argument(
        "test",
        choices=TEST_TYPES.keys(),
        type=str,
        help=f"Choose a test from {set(TEST_TYPES.keys())}",
    )
    parser.add_argument(
        "dataset",
        choices=DATASETS.keys(),
        type=str,
        help=f"Choose a dataset from {set(DATASETS.keys())}",
    )
    args = parser.parse_args()

    return args.test, args.dataset


def get_dataset_arg() -> str:
    parser = argparse.ArgumentParser(
        description="Choose a dataset.",
    )
    parser.add_argument(
        "dataset",
        choices=DATASETS.keys(),
        type=str,
        help=f"Choose a dataset from {set(DATASETS.keys())}",
    )
    args = parser.parse_args()

    return args.dataset


def get_current_time() -> str:
    return datetime.now().isoformat(timespec="seconds")


def get_progress(current: int, total: int) -> str:
    return f"{get_current_time()}: Processed {current}/{total} ({current / total * 100:.2f}%) SDFs"


def select_molfiles_from_sdf(
    sdf_path: Path, molfile_ids: set[str]
) -> Generator[tuple[str, str], None, None]:
    _get_molfile_id = get_molfile_id(sdf_path)

    for molfile in read_records_from_gzipped_sdf(sdf_path):
        molfile_id = _get_molfile_id(molfile)
        if molfile_id in molfile_ids:
            yield molfile_id, molfile

    return None


INCHI_API_PARAMETERS: Final[str] = ""
TEST_PATH: Final[Path] = Path(__file__).parent.absolute()
INCHI_LIB_PATH: Final[Path] = TEST_PATH.joinpath("libinchi.so.dev")
INCHI_REFERENCE_LIB_PATH: Final[Path] = TEST_PATH.joinpath("libinchi.so.v1.06")
N_INVARIANCE_RUNS: Final[int] = 10
N_PROCESSES: Final[int] = len(
    os.sched_getaffinity(0)
)  # https://docs.python.org/3/library/multiprocessing.html#multiprocessing.cpu_count

DATASETS: Final[dict[str, dict[str, Any]]] = {
    "ci": {
        "sdf_paths": sorted(TEST_PATH.joinpath("data/ci").glob("*.sdf.gz")),
        "log_path": TEST_PATH.joinpath("data/ci/"),
    },
    "pubchem-compound": {
        "sdf_paths": sorted(
            TEST_PATH.joinpath("data/pubchem/compound").glob("*.sdf.gz")
        ),
        "log_path": TEST_PATH.joinpath("data/pubchem/compound"),
        "download_path": "Compound/CURRENT-Full",
    },
    "pubchem-compound3d": {
        "sdf_paths": sorted(
            TEST_PATH.joinpath("data/pubchem/compound3d").glob("*.sdf.gz")
        ),
        "log_path": TEST_PATH.joinpath("data/pubchem/compound3d"),
        "download_path": "Compound_3D/01_conf_per_cmpd",
    },
    "pubchem-substance": {
        "sdf_paths": sorted(
            TEST_PATH.joinpath("data/pubchem/substance").glob("*.sdf.gz")
        ),
        "log_path": TEST_PATH.joinpath("data/pubchem/substance"),
        "download_path": "Substance/CURRENT-Full",
    },
}

TEST_TYPES: Final[dict[str, dict[str, str]]] = {
    "regression": {
        "script": f"{TEST_PATH}/compile_inchi_lib.sh",
    },
    "regression-reference": {
        "script": f"{TEST_PATH}/compile_reference_inchi_lib.sh",
    },
    "invariance": {
        "script": f"{TEST_PATH}/compile_inchi_lib.sh",
    },
}
