from inchi_tests.config_models import TestConfig
from pathlib import Path


config = TestConfig(
    name="invariance",
    inchi_library_path=Path("CMake_build/libinchi_build/lib/libinchi.so"),
)
