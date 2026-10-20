import pytest
from molecular_inorganics.comparators import (
    LegacyComparator,
    OptionComparator,
    PrefixInsensitiveComparator,
)
from molecular_inorganics.identifiers import (
    inchi_body,
    inchi_prefix,
    is_failed,
    key_without_flags,
)

ETHANOL = "C2H6O/c1-2-3/h3H,2H2,1H3"
PT_EN_CL2 = "C2H6Cl2N2Pt/c3-7(4)5-1-2-6-7/h5-6H,1-2H2"


def _result(inchi, key="K", exit=0, message=""):
    return {"inchi": inchi, "key": key, "exit": exit, "message": message}


def test_inchi_body_and_prefix_split_at_the_first_slash():
    for prefix in ("InChI=1S", "InChI=1BB", "InChI=1N", "InChI=1"):
        assert inchi_prefix(f"{prefix}/{ETHANOL}") == prefix
        assert inchi_body(f"{prefix}/{ETHANOL}") == ETHANOL
    assert inchi_body("") == "" and inchi_prefix("") == ""


def test_key_without_flags_drops_the_two_prefix_characters():
    assert key_without_flags("LFQSCWFLJHTTHZ-UHFFFAOYSA-N") == "LFQSCWFLJHTTHZ-UHFFFAOY-N"
    assert key_without_flags("LFQSCWFLJHTTHZ-UHFFFAOYNA-N") == "LFQSCWFLJHTTHZ-UHFFFAOY-N"
    assert key_without_flags("") == ""


def test_is_failed_treats_exit_1_as_a_warning():
    # inchi_api.h: inchi_Ret_WARNING = 1, inchi_Ret_ERROR = 2.
    assert is_failed({"inchi": "InChI=1S/X", "exit": 0}) is False
    assert is_failed({"inchi": "InChI=1S/X", "exit": 1}) is False
    assert is_failed({"inchi": "", "exit": 2}) is True
    assert is_failed({"inchi": "", "exit": 0}) is True
    # A reference row for a molfile that timed out.
    assert is_failed({"timeout_seconds": 60.0}) is True


class TestPrefixInsensitiveComparator:
    def test_ignores_the_prefix_and_counts_it(self):
        comparator = PrefixInsensitiveComparator()

        assert comparator(
            _result(f"InChI=1BB/{ETHANOL}", "LFQSCWFLJHTTHZ-UHFFFAOYBB-N"),
            _result(f"InChI=1S/{ETHANOL}", "LFQSCWFLJHTTHZ-UHFFFAOYSA-N"),
        ) is True
        assert comparator.summary() == {
            "matched": 1, "mismatched": 0, "prefix_only": 1, "key_only": 1,
            "warning_only": 0, "both_failed": 0, "failure_kind_only": 0,
        }

    def test_a_different_body_is_a_mismatch(self):
        comparator = PrefixInsensitiveComparator()

        assert comparator(_result("InChI=1BB/C2H6O/c1-2-3"), _result("InChI=1S/CH4O/c1-2")) is False
        assert comparator.summary()["mismatched"] == 1

    def test_a_failure_on_one_side_only_is_a_mismatch(self):
        comparator = PrefixInsensitiveComparator()

        assert comparator(_result("", "", 2), _result(f"InChI=1S/{ETHANOL}")) is False

    def test_a_timed_out_reference_row_is_a_mismatch_not_an_error(self):
        comparator = PrefixInsensitiveComparator()

        assert comparator(_result(f"InChI=1S/{ETHANOL}"), {"timeout_seconds": 60.0}) is False
        assert comparator.summary()["mismatched"] == 1

    def test_mutual_failures_count_apart_from_warnings(self):
        """Two failures with different codes (2 is an error, 3 a fatal error) differ
        in the kind of failure, not in the warning level."""
        comparator = PrefixInsensitiveComparator()

        assert comparator(
            _result(f"InChI=1BB/{ETHANOL}", "A", 2), _result(f"InChI=1S/{ETHANOL}", "B", 3)
        ) is True
        assert comparator(_result("", "", 2), _result("", "", 2)) is True
        summary = comparator.summary()
        assert summary["both_failed"] == 2
        assert summary["failure_kind_only"] == 1
        assert summary["warning_only"] == summary["prefix_only"] == summary["key_only"] == 0


class TestOptionComparator:
    def test_unaffected_excludes_warning_and_message_changes(self):
        comparator = OptionComparator()
        base = _result(f"InChI=1S/{PT_EN_CL2}", "K-SA", 1, "m")

        comparator({**base, "inchi": f"InChI=1BB/{PT_EN_CL2}", "key": "K-BB"}, base)
        comparator({**base, "inchi": f"InChI=1BB/{PT_EN_CL2}", "exit": 0}, base)
        comparator({**base, "inchi": f"InChI=1BB/{PT_EN_CL2}", "message": ""}, base)
        comparator(dict(base), base)

        summary = comparator.summary()
        assert summary["prefix_only"] == 3
        assert summary["unaffected"] == 1
        assert summary["message_only"] == 1

    def test_changes_without_a_metal_are_counted_apart(self):
        comparator = OptionComparator()
        # Without a metal: prefix, warning level and warning text all changed.
        comparator(
            _result(f"InChI=1BB/{ETHANOL}", exit=0, message=""),
            _result(f"InChI=1S/{ETHANOL}", exit=1, message="Charges were rearranged"),
        )
        # With a metal the same changes are expected and not counted again.
        comparator(
            _result(f"InChI=1BB/{PT_EN_CL2}", exit=0, message=""),
            _result(f"InChI=1S/{PT_EN_CL2}", exit=1, message="Metal was disconnected"),
        )

        summary = comparator.summary()
        assert summary["message_only"] == summary["warning_only"] == 2
        assert summary["without_metal_prefix_changed"] == 1
        assert summary["without_metal_warning_only"] == 1
        assert summary["without_metal_message_only"] == 1

    def test_mismatches_and_mutual_failures_are_not_message_changes(self):
        comparator = OptionComparator()
        comparator(_result("InChI=1BB/X", message="a"), _result("InChI=1S/Y", message="b"))
        comparator(_result("", "", 2, "a"), _result("", "", 2, "b"))

        assert comparator.summary()["message_only"] == 0


class TestLegacyComparator:
    REFERENCE = _result(f"InChI=1S/{ETHANOL}", "LFQSCWFLJHTTHZ-UHFFFAOYSA-N", 0, "")

    def test_accepts_the_non_standard_prefix_and_key_flags(self):
        comparator = LegacyComparator()

        assert comparator(
            {**self.REFERENCE, "inchi": f"InChI=1N/{ETHANOL}", "key": "LFQSCWFLJHTTHZ-UHFFFAOYNA-N"},
            dict(self.REFERENCE),
        ) is True
        assert comparator.summary() == {"matched": 1, "mismatched": 0, "prefix_changed": 1}

    @pytest.mark.parametrize(
        "field, value",
        [
            ("inchi", "InChI=1N/C2H6O/c1-2-3"),
            ("key", "XXXXXXXXXXXXXX-UHFFFAOYNA-N"),
            ("exit", 1),
            ("message", "Charges were rearranged"),
        ],
    )
    def test_is_exact_on_everything_else(self, field, value):
        comparator = LegacyComparator()

        assert comparator({**self.REFERENCE, field: value}, dict(self.REFERENCE)) is False
        assert comparator.summary()["mismatched"] == 1

    def test_a_timed_out_reference_row_is_a_mismatch(self):
        assert LegacyComparator()(dict(self.REFERENCE), {"timeout_seconds": 60.0}) is False
