import argparse
from datetime import datetime


def get_current_time() -> str:
    return datetime.now().isoformat(timespec="seconds")


def get_progress(current: int, total: int) -> str:
    return f"{get_current_time()}: Processed {current}/{total} ({current / total * 100:.2f}%) SDFs"


def get_molfile_id_pubchem(molfile: str) -> str:
    return molfile.split()[0].strip()


def get_config_args() -> tuple[str, str]:
    parser = argparse.ArgumentParser(
        description="Choose a test and dataset.",
    )
    parser.add_argument(
        "--test-config",
        type=str,
        required=True,
        help="Specify the path to a test configuration file.",
    )
    parser.add_argument(
        "--data-config",
        type=str,
        required=True,
        help="Specify the path to a dataset configuration file.",
    )
    args = parser.parse_args()

    return (args.test_config, args.data_config)
