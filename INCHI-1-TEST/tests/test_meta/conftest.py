from pathlib import Path

import pytest


def pytest_addoption(parser):
    parser.addoption(
        "--lib-path",
        action="store",
        default="CMake_build/full_build/INCHI-1-SRC/INCHI_API/libinchi/src/lib/libinchi.so",
        help="Absolute path to the InChI shared library.",
    )


@pytest.fixture
def inchi_lib_path(request) -> str:
    lib_path = Path(request.config.getoption("--lib-path"))
    if not lib_path.exists():
        raise FileNotFoundError(f"InChI library not found at {lib_path}.")
    return str(lib_path)
