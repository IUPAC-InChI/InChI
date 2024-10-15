import pytest
import subprocess
from typing import Callable
from pathlib import Path
from dataclasses import dataclass


def pytest_addoption(parser):
    parser.addoption(
        "--exe-path",
        action="store",
        default="INCHI-1-TEST/exes/inchi-1",
        help="Absolute path to the InChI executable.",
    )


@dataclass
class InchiResult:
    stdout: str
    output: str  # InChI and AuxInfo


@pytest.fixture
def run_inchi_exe(request, tmp_path: Path) -> Callable:
    def _run_inchi_exe(molfile_path: str, args: str = "") -> InchiResult:

        exe_path: str = request.config.getoption("--exe-path")
        if not Path(exe_path).exists():
            raise FileNotFoundError(f"InChI executable not found at {exe_path}.")

        result = subprocess.run(
            [exe_path, molfile_path, str(tmp_path.joinpath("output.txt"))]
            + args.split(),
            capture_output=True,
            text=True,
        )

        return InchiResult(
            stdout=result.stderr,
            output=Path(tmp_path.joinpath("output.txt")).read_text(),
        )

    return _run_inchi_exe
