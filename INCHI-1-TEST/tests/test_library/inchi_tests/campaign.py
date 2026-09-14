"""Analysis tooling for the PubChem MolecularInorganics regression campaign.

Three runs over the same PubChem SDFs:

  A (reference) v1.07.5, no options
  B             dev, -MolecularInorganics
  C             dev, no options

Results are stored raw, prefixes included. Comparison ignores the
version-and-kind prefix (`comparators.PrefixInsensitiveComparator`), and every
A-vs-B mismatch is re-checked against v1.07.5 `-RecMet` by comparing that run's
reconnected (`/r`) layer with the `-MolecularInorganics` InChI minus its prefix.
"""

import json
import re
from collections import Counter, defaultdict
from pathlib import Path
from pydantic import BaseModel

_FAILURE_PATTERN = re.compile(
    r"^INFO:sdf_pipeline:regression test failed( expectedly)?:(?P<entry>\{.*\})$"
)
_SUMMARY_PATTERN = re.compile(r"Comparison summary: (?P<counts>\{.*\})$")


class Mismatch(BaseModel):
    molfile_id: str
    sdf: str
    current: dict
    reference: dict
    expected: bool


def parse_regression_log(log_path: Path) -> list[Mismatch]:
    mismatches: list[Mismatch] = []

    with open(log_path, "r", encoding="utf-8") as log_file:
        for line in log_file:
            match = _FAILURE_PATTERN.match(line.rstrip("\n"))
            if not match:
                continue
            entry = json.loads(match.group("entry"))
            mismatches.append(
                Mismatch(
                    molfile_id=entry["molfile_id"],
                    sdf=entry["sdf"],
                    current=json.loads(entry["diff"]["current"]),
                    reference=json.loads(entry["diff"]["reference"]),
                    expected=bool(match.group(1)),
                )
            )

    return mismatches


def mismatch_ids_by_sdf(mismatches: list[Mismatch]) -> dict[str, set[str]]:
    grouped: dict[str, set[str]] = defaultdict(set)

    for mismatch in mismatches:
        grouped[mismatch.sdf].add(mismatch.molfile_id)

    return dict(grouped)


def parse_comparison_summary(log_path: Path) -> dict[str, int]:
    """The tally of differences the comparison deliberately ignored."""
    counts: dict[str, int] = {}

    with open(log_path, "r", encoding="utf-8") as log_file:
        for line in log_file:
            match = _SUMMARY_PATTERN.search(line.rstrip("\n"))
            if match:
                counts = json.loads(match.group("counts"))

    return counts
