from inchi_tests.config_models import TestConfig
from pathlib import Path


config = TestConfig(
    name="regression-reference",
    inchi_library_path=Path("CMake_build/libinchi_build/lib/v1_6_0/libinchi.so.v1.06"),
)
