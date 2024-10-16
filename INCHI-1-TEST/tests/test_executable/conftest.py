import pytest
import subprocess
import re
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
    inchi: str
    aux_info: str


def parse_inchi_from_executable_output(output: str) -> str:
    """The RegEx does not validate the InChI.
    It's meant to be lenient enough to recognize (almost) any InChI in the output.
    See https://chemistry.stackexchange.com/questions/82144/what-is-the-correct-regular-expression-for-inchi.
    """
    inchi_regex = re.compile(r"InChI=1S?/\S+(\s|$)")
    inchi_match = inchi_regex.search(output)
    if inchi_match is None:
        print("Executable output does not contain an InChI.")
        return ""

    return inchi_match.group()


def parse_aux_info_from_executable_output(output: str) -> str:
    aux_info_regex = re.compile(r"AuxInfo=\S+(\s|$)")
    aux_info_match = aux_info_regex.search(output)
    if aux_info_match is None:
        print("Executable output does not contain an AuxInfo.")
        return ""

    return aux_info_match.group()


@pytest.fixture
def run_inchi_exe(request, tmp_path: Path) -> Callable:
    def _run_inchi_exe(molfile: str, args: str = "") -> InchiResult:

        exe_path: str = request.config.getoption("--exe-path")
        if not Path(exe_path).exists():
            raise FileNotFoundError(f"InChI executable not found at {exe_path}.")

        output_path = tmp_path.joinpath("output.txt")
        molfile_path = tmp_path.joinpath("tmp.mol")
        molfile_path.write_text(molfile)

        result = subprocess.run(
            [exe_path, str(molfile_path), str(output_path)] + args.split(),
            capture_output=True,
            text=True,
        )
        output = output_path.read_text()

        return InchiResult(
            stdout=result.stderr,
            inchi=parse_inchi_from_executable_output(output),
            aux_info=parse_aux_info_from_executable_output(output),
        )

    return _run_inchi_exe
