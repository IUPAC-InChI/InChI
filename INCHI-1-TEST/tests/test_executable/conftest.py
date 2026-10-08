import pytest
import subprocess
from sys import platform
from typing import Callable
from pathlib import Path
from dataclasses import dataclass


def pytest_addoption(parser):
    parser.addoption(
        "--exe-path",
        action="store",
        default="CMake_build/full_build/INCHI-1-SRC/INCHI_EXE/inchi-1/src/bin/inchi-1",
        help="Absolute path to the InChI executable.",
    )


@dataclass
class InchiResult:
    stdout: str
    stderr: str
    output: str
    log: str
    problem: str


def adapt_args_to_platform(args: str) -> list[str]:
    args_platform_agnostic = [
        arg[1:] if arg.startswith(("-", "/")) else arg for arg in args.split()
    ]
    if platform == "win32":
        return [f"/{arg}" for arg in args_platform_agnostic]

    return [f"-{arg}" for arg in args_platform_agnostic]


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
                *adapt_args_to_platform(args),
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


@pytest.fixture
def run_inchi_exe_stdin(request) -> Callable:
    """Run the executable feeding the structure via stdin (no input file arg).

    Mirrors the classic Unix invocation "cat file.mol | inchi-1 -STDIO ...".
    Only options are passed on the command line; output/log go to the standard
    streams, so the result is captured from stdout/stderr.
    """

    def _run_inchi_exe_stdin(input: str, args: str = "") -> InchiResult:

        if "ami" in args.lower():
            raise ValueError("'AMI' is not supported by the test wrapper.")

        exe_path: str = request.config.getoption("--exe-path")
        if not Path(exe_path).exists():
            raise FileNotFoundError(f"InChI executable not found at {exe_path}.")

        result = subprocess.run(
            [exe_path, *adapt_args_to_platform(args)],
            input=input,
            capture_output=True,
            text=True,
        )

        return InchiResult(
            stdout=result.stdout,
            stderr=result.stderr,
            output="",
            log="",
            problem="",
        )

    return _run_inchi_exe_stdin
