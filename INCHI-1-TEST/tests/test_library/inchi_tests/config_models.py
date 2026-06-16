from typing import Callable

from pydantic import BaseModel, DirectoryPath, FilePath


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
