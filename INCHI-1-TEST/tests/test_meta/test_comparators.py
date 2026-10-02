import pytest
from inchi_tests.consumers import inchi_body, is_failed
from inchi_tests.comparators import (
    PrefixInsensitiveComparator,
    compare_ignoring_prefix,
    key_without_flags,
)


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
        "failure_kind_only": 0,
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
        "failure_kind_only": 0,
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
        "failure_kind_only": 0,
    }


def test_a_failure_with_output_is_counted_as_a_failure_not_a_warning():
    """`is_failed` is `exit >= 2` *or* an empty InChI, so a failure can carry output.

    Two runs failing on the same body with different error codes -- inchi_Ret_ERROR
    (2) against inchi_Ret_FATAL (3) -- differ in failure kind, not warning level.
    Counting that as `warning_only` inflated the figure the report presents as
    changed warnings."""
    comparator = PrefixInsensitiveComparator()
    body = "C2H6O/c1-2-3"

    assert comparator(
        {"inchi": f"InChI=1B/{body}", "key": "A", "exit": 2},
        {"inchi": f"InChI=1S/{body}", "key": "B", "exit": 3},
    ) is True
    summary = comparator.summary()
    assert summary["both_failed"] == 1
    assert summary["failure_kind_only"] == 1
    assert summary["warning_only"] == 0
    # No prefix or key tally either: neither is meaningful for a failed structure.
    assert summary["prefix_only"] == 0
    assert summary["key_only"] == 0


def test_mutual_failure_on_the_same_code_tallies_no_kind_difference():
    comparator = PrefixInsensitiveComparator()
    failed = {"inchi": "InChI=1B/C2H6O/c1-2-3", "key": "A", "exit": 2}

    assert comparator(dict(failed), dict(failed)) is True
    assert comparator.summary()["both_failed"] == 1
    assert comparator.summary()["failure_kind_only"] == 0


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
        "failure_kind_only": 0,
    }


# A row as the CI references store it (`consumers.regression_consumer`).
CI_ROW = {
    "inchi": "InChI=1S/ClH.Na/h1H;/q;+1/p-1",
    "key": "FAPWRFPIFSIZLT-UHFFFAOYSA-M",
    "aux": "AuxInfo=1/0/N:1;2/rA:2ClNa/rB:/rC:;;",
    "log": "",
    "message": "",
    "exit": 0,
}


def test_key_without_flags_drops_the_standard_and_version_characters():
    assert key_without_flags("FAPWRFPIFSIZLT-UHFFFAOYSA-M") == "FAPWRFPIFSIZLT-UHFFFAOY-M"
    assert key_without_flags("FAPWRFPIFSIZLT-UHFFFAOYSB-M") == "FAPWRFPIFSIZLT-UHFFFAOY-M"
    assert key_without_flags("") == ""


def test_compare_ignoring_prefix_accepts_the_1sb_prefix_and_key_flag():
    # #280 changes only these two fields for ionic NaCl.
    current = {
        **CI_ROW,
        "inchi": "InChI=1SB/ClH.Na/h1H;/q;+1/p-1",
        "key": "FAPWRFPIFSIZLT-UHFFFAOYSB-M",
    }

    assert compare_ignoring_prefix(current, CI_ROW) is True
    assert compare_ignoring_prefix(dict(CI_ROW), CI_ROW) is True


@pytest.mark.parametrize(
    "field, value",
    [
        ("inchi", "InChI=1S/ClH.Na/h1H;/q;+1"),
        ("key", "FAPWRFPIFSIZLT-UHFFFAOYSA-N"),
        ("key", "XXXXXXXXXXXXXX-UHFFFAOYSA-M"),
        ("aux", "AuxInfo=1/0/N:2;1/rA:2ClNa/rB:/rC:;;"),
        ("log", "Warning (Metal was disconnected)"),
        ("message", "Metal was disconnected"),
        ("exit", 1),
    ],
)
def test_compare_ignoring_prefix_is_exact_on_everything_else(field, value):
    """Unlike `PrefixInsensitiveComparator`, which ignores all of these but the body."""
    current = {**CI_ROW, "inchi": "InChI=1SB/ClH.Na/h1H;/q;+1/p-1", field: value}

    assert compare_ignoring_prefix(current, CI_ROW) is False


def test_compare_ignoring_prefix_treats_a_failure_flip_as_a_mismatch():
    failed = {**CI_ROW, "inchi": "", "key": "", "aux": "", "exit": 2}

    assert compare_ignoring_prefix(failed, CI_ROW) is False
    assert compare_ignoring_prefix(failed, dict(failed)) is True
