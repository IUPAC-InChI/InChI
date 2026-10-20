"""How a pass's results are compared with Run A's reference.

`run.py` calls a comparator once per structure with the current and the
reference result (`inchi`, `key`, `exit`, `message`). It returns whether they
match and counts the differences it accepts, which `run.py` logs as the pass's
`Comparison summary`.

- Run B (Molecular Inorganics on) uses `OptionComparator`: the InChI body and
  the failure state must match; the prefix, the InChIKey, the warning level and
  the message are counted, not compared.
- Run C (`-LegacyMetalHandling`) uses `LegacyComparator`: every field must match
  except the prefix and the two InChIKey characters that mirror it, since the
  legacy handling produces non-standard `InChI=1N/` output.
"""

from collections import Counter
from molecular_inorganics.identifiers import (
    has_metal,
    inchi_body,
    is_failed,
    key_without_flags,
)


class PrefixInsensitiveComparator:
    """Compares the InChI body and the failure state, ignoring the prefix.

    Two results match when their InChI bodies are equal and both either failed or
    did not. The differences the rule accepts are counted, so a pass can report
    how much of its output changed only in the prefix, the InChIKey, the warning
    level or the kind of failure."""

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
        # `.get`: a reference row for a molfile that timed out has no InChI.
        is_match = inchi_body(current.get("inchi", "")) == inchi_body(
            reference.get("inchi", "")
        ) and is_failed(current) == is_failed(reference)

        if not is_match:
            self.counts["mismatched"] += 1

            return False

        self.counts["matched"] += 1
        if is_failed(current) or is_failed(reference):
            # Both failed. There is no prefix to compare without an InChI, and a
            # different exit code is a different kind of failure (2 is an error,
            # 3 a fatal error), not a changed warning level.
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


class OptionComparator(PrefixInsensitiveComparator):
    """Run B's rule, plus counts of the changes Molecular Inorganics should not make.

    `prefix_only` counts every matched structure whose InChI differs in its
    prefix, even when its warning changed too; `unaffected` is the subset whose
    exit code and message are identical. A matched structure whose warning text
    changed is counted as `message_only`.

    Molecular Inorganics only changes how bonds to metals are treated, so on a
    structure without a metal a changed prefix (`without_metal_prefix_changed`),
    warning level (`without_metal_warning_only`) or warning text
    (`without_metal_message_only`) is unexpected. A metal-free structure whose
    InChI body changed is a mismatch, classified as `changed_without_metal` by
    `classify.py`."""

    def __init__(self) -> None:
        super().__init__()
        self.counts.update(
            {
                "message_only": 0,
                "unaffected": 0,
                "without_metal_prefix_changed": 0,
                "without_metal_warning_only": 0,
                "without_metal_message_only": 0,
            }
        )

    def __call__(self, current: dict, reference: dict) -> bool:
        is_match = super().__call__(current, reference)
        if not is_match or is_failed(current):
            return is_match

        prefix_changed = current["inchi"] != reference["inchi"]
        message_changed = current.get("message") != reference.get("message")
        if message_changed:
            self.counts["message_only"] += 1
        if prefix_changed and current["exit"] == reference["exit"] and not message_changed:
            self.counts["unaffected"] += 1
        if not has_metal(current["inchi"]) and not has_metal(reference["inchi"]):
            if prefix_changed:
                self.counts["without_metal_prefix_changed"] += 1
            if current["exit"] != reference["exit"]:
                self.counts["without_metal_warning_only"] += 1
            if message_changed:
                self.counts["without_metal_message_only"] += 1

        return is_match


def _without_prefix(result: dict) -> dict:
    if "inchi" not in result:
        # A reference row for a molfile that timed out.
        return result

    return {
        **result,
        "inchi": inchi_body(result["inchi"]),
        "key": key_without_flags(result["key"]),
    }


class LegacyComparator:
    """Run C's rule: everything must match except the prefix and the key flags.

    Counts `matched`, `mismatched` and `prefix_changed`, the matched structures
    whose prefix differs (all of them with an InChI, since the legacy handling
    produces `InChI=1N/`)."""

    def __init__(self) -> None:
        self.counts = {"matched": 0, "mismatched": 0, "prefix_changed": 0}

    def __call__(self, current: dict, reference: dict) -> bool:
        is_match = _without_prefix(current) == _without_prefix(reference)
        self.counts["matched" if is_match else "mismatched"] += 1
        if is_match and current["inchi"] != reference["inchi"]:
            self.counts["prefix_changed"] += 1

        return is_match

    def summary(self) -> dict[str, int]:
        return dict(self.counts)
