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
    stderr: str
    output: str
    log: str
    problem: str


@pytest.fixture
def run_inchi_exe(request, tmp_path: Path) -> Callable:
    def _run_inchi_exe(input: str, args: str = "") -> InchiResult:

        if "ami" in args.lower():
            raise ValueError("'AMI' is not supported by the test wrapper.")

        exe_path: str = request.config.getoption("--exe-path")
        if not Path(exe_path).exists():
            raise FileNotFoundError(f"InChI executable not found at {exe_path}.")

        input_path = tmp_path.joinpath("input")
        input_path.write_text(input)

        output_path = tmp_path.joinpath("output")
        log_path = tmp_path.joinpath("log")
        problem_path = tmp_path.joinpath("problem")

        result = subprocess.run(
            [
                exe_path,
                *[input_path, output_path, log_path, problem_path],
                *args.split(),
            ],
            capture_output=True,
            text=True,
        )

        return InchiResult(
            stdout=result.stdout,
            stderr=result.stderr,
            output=output_path.read_text(),
            log=log_path.read_text(),
            problem=problem_path.read_text(),
        )

    return _run_inchi_exe
