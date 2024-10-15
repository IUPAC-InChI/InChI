import pytest
import subprocess
from typing import Callable
from pathlib import Path


def pytest_addoption(parser):
    parser.addoption(
        "--exe-path",
        action="store",
        default="INCHI-1-TEST/exes/inchi-1",
        help="Absolute path to the InChI executable.",
    )


@pytest.fixture
def run_inchi_exe(request) -> Callable:
    def _run_inchi_exe(
        molfile_path: str, args: str = ""
    ) -> subprocess.CompletedProcess:

        exe_path: str = request.config.getoption("--exe-path")
        if not Path(exe_path).exists():
            raise FileNotFoundError(f"InChI executable not found at {exe_path}.")

        return subprocess.run(
            [exe_path, molfile_path, args],
            capture_output=True,
            text=True,
        )

    return _run_inchi_exe
