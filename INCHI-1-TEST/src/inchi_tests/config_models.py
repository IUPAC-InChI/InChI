import os
import sys
import importlib.util as importutil
from importlib.machinery import ModuleSpec
from pathlib import Path
from typing import Callable, Literal
from pydantic import BaseModel, FilePath, DirectoryPath


class TestConfig(BaseModel):
    # All paths are relative to the root of the repository.

    name: Literal["regression", "regression-reference", "invariance"]
    # Path to the InChI shared library.
    inchi_library_path: FilePath
    # Parameters to pass to the InChI API.
    inchi_api_parameters: str = ""
    # Permute each structure N times.
    n_invariance_runs: int = 10
    # Distribute the test over N separate processes.
    # Default according to https://docs.python.org/3/library/multiprocessing.html#multiprocessing.cpu_count.
    n_processes: int = len(os.sched_getaffinity(0))


class DataConfig(BaseModel):
    # All paths are relative to the root of the repository.

    name: str
    # The references and logs will be written to this directory.
    path: DirectoryPath
    # Path(s) to the SDF file(s).
    sdf_paths: list[FilePath]
    # A Python function that parses the ID from a molfile.
    molfile_id_getter: Callable
    # Set of molfile IDs that are expected to fail for "invariance" and "regression" tests.
    # The failures will be logged, but won't cause the test run to fail.
    # E.g., {"regression": {"foo", "bar"}, "invariance": {"baz"}}.
    expected_failures: dict[str, set[str]] = dict()


def load_config(config_name: str, config_path: Path) -> TestConfig | DataConfig:
    spec: ModuleSpec | None = importutil.spec_from_file_location(
        config_name, config_path
    )
    module = importutil.module_from_spec(spec)
    spec.loader.exec_module(module)
    sys.modules[config_name] = module

    return module.config
