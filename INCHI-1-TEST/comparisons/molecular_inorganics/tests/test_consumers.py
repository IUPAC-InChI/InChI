import pytest
from pathlib import Path
from molecular_inorganics.consumers import RESULT_FIELDS, raw_regression_consumer


LIB_PATH = "CMake_build/full_build/INCHI-1-SRC/INCHI_API/libinchi/src/lib/libinchi.so"

ETHANOL_MOLFILE = """ethanol
  test

  3  2  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 C   0  0
    1.0000    0.0000    0.0000 C   0  0
    2.0000    0.0000    0.0000 O   0  0
  1  2  1  0
  2  3  1  0
M  END
$$$$
"""


@pytest.fixture
def lib_path():
    path = Path(LIB_PATH)
    if not path.is_file():
        pytest.skip(f"{LIB_PATH} not built; run ./INCHI-1-TEST/build_with_cmake.sh all")
    return str(path)


def test_raw_consumer_stores_raw_results(lib_path):
    def consume(options):
        return raw_regression_consumer(
            ETHANOL_MOLFILE,
            get_molfile_id=lambda molfile: molfile.splitlines()[0].strip(),
            inchi_lib_path=lib_path,
            inchi_api_parameters=options,
        )

    plain = consume("")
    molecular_inorganics = consume("-MolecularInorganics")

    assert set(plain.result) == RESULT_FIELDS == {"inchi", "key", "exit", "message"}
    assert plain.info.consumer == "raw-regression"
    assert plain.molfile_id == "ethanol"
    # Raw: prefixes and key flags are kept exactly as the library emitted them.
    assert plain.result["inchi"] == "InChI=1S/C2H6O/c1-2-3/h3H,2H2,1H3"
    assert plain.result["key"] == "LFQSCWFLJHTTHZ-UHFFFAOYSA-N"
    assert molecular_inorganics.result["inchi"] == "InChI=1B/C2H6O/c1-2-3/h3H,2H2,1H3"
    assert molecular_inorganics.result["key"] == "LFQSCWFLJHTTHZ-UHFFFAOYBA-N"
