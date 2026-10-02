import argparse
from datetime import datetime
from pathlib import Path


def get_current_time() -> str:
    return datetime.now().isoformat(timespec="seconds")


def get_progress(current: int, total: int) -> str:
    return f"{get_current_time()}: Processed {current}/{total} ({current / total * 100:.2f}%) SDFs"


def get_molfile_id_pubchem(molfile: str) -> str:
    return molfile.split()[0].strip()


class PathValidator(argparse.Action):
    def __call__(self, parser, namespace, values, option_string=None):
        if not Path(values).is_file():
            parser.error(f"{option_string}: '{values}' is not a valid path.")
        setattr(namespace, self.dest, values)


def get_config_args() -> tuple[str, str, str]:
    parser = argparse.ArgumentParser(
        description="Choose a test, InChI library, and dataset.",
    )
    parser.add_argument(
        "--test",
        type=str,
        required=True,
        choices=["regression", "regression-reference", "invariance"],
        help="Specify the kind of test to run.",
    )
    parser.add_argument(
        "--lib-path",
        type=str,
        required=True,
        action=PathValidator,
        help="Specify the path to the InChI library.",
    )
    parser.add_argument(
        "--data-config",
        type=str,
        required=True,
        action=PathValidator,
        help="Specify the path to a dataset configuration file.",
    )
    args = parser.parse_args()

    return (args.test, args.lib_path, args.data_config)
