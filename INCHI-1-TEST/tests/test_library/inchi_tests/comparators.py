"""Comparison rules for regression runs.

Passed to `sdf_pipeline.drivers.regression` as its `compare` hook, so results are
stored raw and normalised only when they are compared."""

from collections import Counter
from inchi_tests.consumers import inchi_body, is_failed


def key_without_flags(key: str) -> str:
    """The InChIKey minus the two characters that encode the InChI prefix.

    The second block ends in the standard/non-standard flag (`S`/`N`) and the
    version character (`A` for version 1), e.g. `LFQSCWFLJHTTHZ-UHFFFAOYSA-N`. The
    hash blocks are computed from the InChI without its prefix, so they do not
    move when only the prefix does. A failed run has no key; it is left as is."""
    if len(key) != 27:
        return key

    return key[:23] + key[25:]


def _without_prefix(result: dict) -> dict:
    return {
        **result,
        "inchi": inchi_body(result["inchi"]),
        "key": key_without_flags(result["key"]),
    }


def compare_ignoring_prefix(current: dict, reference: dict) -> bool:
    """The CI regression rule: byte-for-byte, except for the InChI prefix.

    Decided in the review of #280 (30.09.2026): making MolecularInorganics the
    default changes the standard prefix from `1S` to `1SB`, and that alone must not
    fail every regression test. Only the prefix and the key characters encoding it
    are ignored; `aux`, `log`, `message` and `exit` must still match exactly.

    Deliberately stricter than `PrefixInsensitiveComparator`, which compares only
    the InChI body and the failure state."""
    return _without_prefix(current) == _without_prefix(reference)


# TODO: rename "prefix" here and in `inchi_body` once we've settled on names for
# the constituent parts of the prefix, i.e., the `1`, `B`, and `B` in `1BB`.
class PrefixInsensitiveComparator:
    """Compares InChI results ignoring the version-and-kind prefix.

    Two results match when their InChI bodies are equal and both either failed or
    did not. Differences that the rule deliberately ignores are counted, so a run
    can report how much of its output changed in prefix, key flag, warning level or
    failure kind only. The comparator is called from the driver's main loop, one
    structure at a time, so the counts are exact."""

    def __init__(self) -> None:
        self.counts: Counter = Counter(
            {
                "matched": 0,
                "mismatched": 0,
                "prefix_only": 0,
                "key_only": 0,
                "warning_only": 0,
                "both_failed": 0,
                "failure_kind_only": 0,
            }
        )

    def __call__(self, current: dict, reference: dict) -> bool:
        is_match = inchi_body(current["inchi"]) == inchi_body(
            reference["inchi"]
        ) and is_failed(current) == is_failed(reference)

        if not is_match:
            self.counts["mismatched"] += 1

            return False

        self.counts["matched"] += 1
        if is_failed(current):
            # Both runs failed -- the match test above established that they agree
            # on it. `is_failed` covers an empty InChI *and* `exit >= 2` with output,
            # and neither tally below applies to those: there is no prefix to differ
            # when there is no InChI, and an exit code difference here is a failure
            # kind (inchi_Ret_ERROR 2 vs inchi_Ret_FATAL 3), not a warning flip.
            # Counting it as one would inflate the figure the report presents as
            # changed warnings.
            self.counts["both_failed"] += 1
            if current["exit"] != reference["exit"]:
                self.counts["failure_kind_only"] += 1

            return True

        if current["inchi"] != reference["inchi"]:
            self.counts["prefix_only"] += 1
        if current["key"] != reference["key"]:
            self.counts["key_only"] += 1
        if current["exit"] != reference["exit"]:
            self.counts["warning_only"] += 1

        return True

    def summary(self) -> dict[str, int]:
        return dict(self.counts)
