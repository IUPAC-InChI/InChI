import pytest
from pathlib import Path
from inchi_tests.consumers import campaign_regression_consumer, inchi_body, is_failed
from inchi_tests.comparators import PrefixInsensitiveComparator


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
        pytest.skip(f"{LIB_PATH} not built; see Task 8 Step 1")
    return str(path)


def test_inchi_body_strips_the_version_and_kind_prefix():
    assert inchi_body("InChI=1S/C2H6O/c1-2-3") == "C2H6O/c1-2-3"
    assert inchi_body("InChI=1B/C2H6O/c1-2-3") == "C2H6O/c1-2-3"
    assert inchi_body("InChI=1/C2H6O/c1-2-3") == "C2H6O/c1-2-3"
    assert inchi_body("") == ""


def test_is_failed_treats_exit_1_as_a_warning():
    # inchi_api.h:693-694: inchi_Ret_WARNING = 1, inchi_Ret_ERROR = 2.
    assert is_failed({"inchi": "InChI=1S/X", "exit": 0}) is False
    assert is_failed({"inchi": "InChI=1S/X", "exit": 1}) is False
    assert is_failed({"inchi": "", "exit": 2}) is True
    assert is_failed({"inchi": "", "exit": 0}) is True


def test_campaign_consumer_stores_raw_results(lib_path):
    def consume(options):
        return campaign_regression_consumer(
            ETHANOL_MOLFILE,
            get_molfile_id=lambda molfile: molfile.splitlines()[0].strip(),
            inchi_lib_path=lib_path,
            inchi_api_parameters=options,
        )

    plain = consume("")
    molecular_inorganics = consume("-MolecularInorganics")

    assert set(plain.result) == {"inchi", "key", "exit"}
    assert plain.info.consumer == "campaign-regression"
    assert plain.molfile_id == "ethanol"
    # Raw: prefixes and key flags are kept exactly as the library emitted them.
    assert plain.result["inchi"] == "InChI=1S/C2H6O/c1-2-3/h3H,2H2,1H3"
    assert plain.result["key"] == "LFQSCWFLJHTTHZ-UHFFFAOYSA-N"
    assert molecular_inorganics.result["inchi"] == "InChI=1B/C2H6O/c1-2-3/h3H,2H2,1H3"
    assert molecular_inorganics.result["key"] == "LFQSCWFLJHTTHZ-UHFFFAOYBA-N"


def test_comparator_ignores_the_prefix_and_tallies_it():
    comparator = PrefixInsensitiveComparator()
    current = {
        "inchi": "InChI=1B/C2H6O/c1-2-3/h3H,2H2,1H3",
        "key": "LFQSCWFLJHTTHZ-UHFFFAOYBA-N",
        "exit": 0,
    }
    reference = {
        "inchi": "InChI=1S/C2H6O/c1-2-3/h3H,2H2,1H3",
        "key": "LFQSCWFLJHTTHZ-UHFFFAOYSA-N",
        "exit": 0,
    }

    assert comparator(current, reference) is True
    assert comparator.summary() == {
        "matched": 1,
        "mismatched": 0,
        "prefix_only": 1,
        "key_only": 1,
        "warning_only": 0,
        "both_failed": 0,
    }


def test_comparator_tallies_warning_only_differences():
    comparator = PrefixInsensitiveComparator()
    body = "C2H6Cl2N2Pt/c3-7(4)5-1-2-6-7/h5-6H,1-2H2"

    assert comparator(
        {"inchi": f"InChI=1B/{body}", "key": "K", "exit": 0},
        {"inchi": f"InChI=1B/{body}", "key": "K", "exit": 1},
    ) is True
    assert comparator.summary()["warning_only"] == 1
    assert comparator.summary()["prefix_only"] == 0


def test_comparator_reports_a_real_body_difference():
    comparator = PrefixInsensitiveComparator()

    assert comparator(
        {"inchi": "InChI=1B/C2H6O/c1-2-3", "key": "A", "exit": 0},
        {"inchi": "InChI=1S/CH4O/c1-2", "key": "B", "exit": 0},
    ) is False
    assert comparator.summary() == {
        "matched": 0,
        "mismatched": 1,
        "prefix_only": 0,
        "key_only": 0,
        "warning_only": 0,
        "both_failed": 0,
    }


def test_comparator_counts_a_mutual_failure_separately():
    # Measured: an unparseable record returns exit=2, inchi="" under no options,
    # -MolecularInorganics and -RecMet alike. Every field compares equal, so this
    # must not be counted as a prefix-only difference.
    comparator = PrefixInsensitiveComparator()
    failure = {"inchi": "", "key": "", "exit": 2}

    assert comparator(dict(failure), dict(failure)) is True
    assert comparator.summary() == {
        "matched": 1,
        "mismatched": 0,
        "prefix_only": 0,
        "key_only": 0,
        "warning_only": 0,
        "both_failed": 1,
    }


def test_comparator_treats_a_failure_flip_as_a_mismatch():
    comparator = PrefixInsensitiveComparator()

    assert comparator(
        {"inchi": "", "key": "", "exit": 2},
        {"inchi": "InChI=1S/C2H6O/c1-2-3", "key": "B", "exit": 0},
    ) is False
    assert comparator.summary()["mismatched"] == 1


def test_identical_raw_results_tally_nothing_extra():
    comparator = PrefixInsensitiveComparator()
    result = {"inchi": "InChI=1S/C2H6O/c1-2-3", "key": "A", "exit": 0}

    assert comparator(dict(result), dict(result)) is True
    assert comparator.summary() == {
        "matched": 1,
        "mismatched": 0,
        "prefix_only": 0,
        "key_only": 0,
        "warning_only": 0,
        "both_failed": 0,
    }
