import argparse
from pathlib import Path


DATASETS = ["compound", "compound3d", "substance"]
DOWNLOAD_PATHS = {
    "compound": "Compound/CURRENT-Full",
    "compound3d": "Compound_3D/01_conf_per_cmpd",
    "substance": "Substance/CURRENT-Full",
}
PUBCHEM_DIR = Path(__file__).parent.absolute()


def get_dataset_arg() -> str:
    parser = argparse.ArgumentParser(
        description="Choose a dataset.",
    )
    parser.add_argument(
        "dataset",
        choices=DATASETS,
        type=str,
        help=f"Choose a dataset from {set(DATASETS)}",
    )
    args = parser.parse_args()

    return args.dataset
