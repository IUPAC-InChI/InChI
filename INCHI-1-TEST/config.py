import re
import os
import argparse
from typing import Final, Any
from pathlib import Path
from datetime import datetime


def get_molfile_id_ci(molfile: str) -> str:
    molfile_id = ""

    mcule_id_patterns = re.compile(r"<Mcule_ID>(.*?)>", re.DOTALL)
    mcule_id_match = mcule_id_patterns.search(molfile)
    if mcule_id_match:
        # mcule test set.
        molfile_id = mcule_id_match.group(1).strip()
    else:
        # InChI legacy test set.
        molfile_id = molfile.splitlines()[-3].strip()

    return molfile_id


def get_molfile_id_pubchem(molfile: str) -> str:
    return molfile.split()[0].strip()


def get_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Choose a test and dataset.",
    )
    test_types = ["regression", "invariance", "regression-reference"]
    parser.add_argument(
        "test",
        choices=test_types,
        type=str,
        help=f"Choose a test from {test_types}",
    )
    parser.add_argument(
        "dataset",
        choices=DATASETS.keys(),
        type=str,
        help=f"Choose a dataset from {set(DATASETS.keys())}",
    )
    parser.add_argument(
        "--library-path",
        type=Path,
        help=f"InChI library location. Default: {INCHI_LIB_PATH}",
    )
    
    return parser.parse_args()


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
        "molfile_id": get_molfile_id_ci,
        "expected_failures": {
            "regression": {
                "PUBCHEM_COMPOUND_CID-20733713",
                "PUBCHEM_COMPOUND_CID-166625356",
                "PUBCHEM_COMPOUND_CID-102182973",
                "PUBCHEM_COMPOUND_CID-92178689",
            },
            "invariance": {"D4h-allene", "PUBCHEM_COMPOUND_CID-102182973"},
        },
    },
    "pubchem-compound": {
        "sdf_paths": sorted(
            TEST_PATH.joinpath("data/pubchem/compound").glob("*.sdf.gz")
        ),
        "log_path": TEST_PATH.joinpath("data/pubchem/compound"),
        "molfile_id": get_molfile_id_pubchem,
        "download_path": "Compound/CURRENT-Full",
    },
    "pubchem-compound3d": {
        "sdf_paths": sorted(
            TEST_PATH.joinpath("data/pubchem/compound3d").glob("*.sdf.gz")
        ),
        "log_path": TEST_PATH.joinpath("data/pubchem/compound3d"),
        "molfile_id": get_molfile_id_pubchem,
        "download_path": "Compound_3D/01_conf_per_cmpd",
    },
    "pubchem-substance": {
        "sdf_paths": sorted(
            TEST_PATH.joinpath("data/pubchem/substance").glob("*.sdf.gz")
        ),
        "log_path": TEST_PATH.joinpath("data/pubchem/substance"),
        "molfile_id": get_molfile_id_pubchem,
        "download_path": "Substance/CURRENT-Full",
    },
}
