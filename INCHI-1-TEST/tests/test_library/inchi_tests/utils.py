import argparse
import re
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


TAG_PATTERN = re.compile(r"^[A-Za-z0-9_]*$")


class TagValidator(argparse.Action):
    def __call__(self, parser, namespace, values, option_string=None):
        if not TAG_PATTERN.match(values):
            parser.error(
                f"{option_string}: '{values}' must contain only letters, digits, and underscores."
            )
        setattr(namespace, self.dest, values)


def reference_filename(sdf_stem: str, run_tag: str) -> str:
    tag_suffix = f".{run_tag}" if run_tag else ""
    return f"{sdf_stem}{tag_suffix}.regression_reference.sqlite"


def log_filename(timestamp: str, test: str, dataset: str, log_tag: str) -> str:
    tag_suffix = f".{log_tag}" if log_tag else ""
    return f"{timestamp}_{test}_{dataset}{tag_suffix}.log"


def get_config_args() -> tuple[str, str, str, str, str, str]:
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
    parser.add_argument(
        "--inchi-api-parameters",
        type=str,
        default="",
        help=(
            "Space-separated InChI options passed to the library. Must be given in "
            "'--inchi-api-parameters=-MolecularInorganics' form: a space-separated "
            "value beginning with '-' is parsed as an unknown option."
        ),
    )
    parser.add_argument(
        "--run-tag",
        type=str,
        default="",
        action=TagValidator,
        help=(
            "Namespace for the regression reference files, e.g. 'ref_1075'. Runs "
            "compared against the same reference share this tag. A non-empty tag "
            "switches the run into campaign mode: raw results, prefix-insensitive "
            "comparison. Letters, digits, underscores."
        ),
    )
    parser.add_argument(
        "--log-tag",
        type=str,
        default="",
        action=TagValidator,
        help="Namespace for the log file, e.g. 'run_b_dev_mi'. Distinct per run.",
    )
    args = parser.parse_args()

    return (
        args.test,
        args.lib_path,
        args.data_config,
        args.inchi_api_parameters,
        args.run_tag,
        args.log_tag,
    )
