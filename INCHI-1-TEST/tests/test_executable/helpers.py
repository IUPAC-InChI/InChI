from typing import Callable
from pathlib import Path


def tmp_molfile(molfile: Callable) -> Callable:
    """
    Creates a temporary molfile and returns its path.

    `molfile` must return a string.
    """

    def get_molfile_path(tmp_path: Path) -> str:
        molfile_path = tmp_path.joinpath("tmp.mol")
        molfile_path.write_text(molfile())

        return str(molfile_path)

    return get_molfile_path
