import json
import pytest
from pathlib import Path
from inchi_tests.campaign import (
    parse_regression_log,
    mismatch_ids_by_sdf,
    parse_comparison_summary,
)


LIB_PATH = "CMake_build/full_build/INCHI-1-SRC/INCHI_API/libinchi/src/lib/libinchi.so"

# Measured on the built libraries for Pt(en)Cl2 (see Task 8).
PTEN_PLAIN = "InChI=1S/C2H6N2.2ClH.Pt/c3-1-2-4;;;/h3-4H,1-2H2;2*1H;/q-2;;;+4/p-2"
PTEN_MI = "InChI=1B/C2H6Cl2N2Pt/c3-7(4)5-1-2-6-7/h5-6H,1-2H2"
PTEN_RECMET = (
    "InChI=1/C2H6N2.2ClH.Pt/c3-1-2-4;;;/h3-4H,1-2H2;2*1H;/q-2;;;+4/p-2"
    "/rC2H6Cl2N2Pt/c3-7(4)5-1-2-6-7/h5-6H,1-2H2"
)


@pytest.fixture
def lib_path():
    path = Path(LIB_PATH)
    if not path.is_file():
        pytest.skip(f"{LIB_PATH} not built; see Task 8 Step 1")
    return str(path)


def _log_line(molfile_id, sdf, current, reference, expectedly=False):
    entry = {
        "time": "2026-09-14T12:00:00",
        "molfile_id": molfile_id,
        "sdf": sdf,
        "info": {"consumer": "campaign-regression", "parameters": "-MolecularInorganics"},
        "diff": {"current": json.dumps(current), "reference": json.dumps(reference)},
    }
    suffix = " expectedly" if expectedly else ""
    return f"INFO:sdf_pipeline:regression test failed{suffix}:{json.dumps(entry)}\n"


def test_parse_regression_log_extracts_raw_mismatches(tmp_path):
    log_path = tmp_path / "20260914T120000_regression_pubchem-compound.run_b.log"
    log_path.write_text(
        "INFO:root:2026-09-14T11:59:59: Using 'libinchi.so'.\n"
        + _log_line(
            "10", "Compound_000000001_000500000.sdf.gz",
            {"inchi": PTEN_MI, "key": "K1", "exit": 0},
            {"inchi": PTEN_PLAIN, "key": "K0", "exit": 1},
        )
        + _log_line(
            "11", "Compound_000000001_000500000.sdf.gz",
            {"inchi": "InChI=1B/X", "key": "K3", "exit": 0},
            {"inchi": "InChI=1S/Y", "key": "K4", "exit": 0},
            expectedly=True,
        )
        + "INFO:root:2026-09-14T12:30:00: Ran regression on Compound_000000001_000500000.sdf.gz.\n"
    )

    mismatches = parse_regression_log(log_path)

    assert len(mismatches) == 2
    assert mismatches[0].molfile_id == "10"
    # The prefix survives into the log, unmodified.
    assert mismatches[0].current["inchi"] == PTEN_MI
    assert mismatches[0].reference["inchi"] == PTEN_PLAIN
    assert mismatches[0].reference["exit"] == 1
    assert mismatches[0].expected is False
    assert mismatches[1].expected is True


def test_mismatch_ids_by_sdf_groups_ids(tmp_path):
    log_path = tmp_path / "log.log"
    raw = {"inchi": "InChI=1S/X", "key": "K", "exit": 0}
    log_path.write_text(
        _log_line("10", "A.sdf.gz", raw, raw)
        + _log_line("11", "A.sdf.gz", raw, raw)
        + _log_line("12", "B.sdf.gz", raw, raw)
    )

    assert mismatch_ids_by_sdf(parse_regression_log(log_path)) == {
        "A.sdf.gz": {"10", "11"},
        "B.sdf.gz": {"12"},
    }


def test_parse_regression_log_ignores_unrelated_lines(tmp_path):
    log_path = tmp_path / "log.log"
    log_path.write_text(
        "INFO:root:Starting to process 3 SDFs on 8 cores.\n"
        "ERROR:sdf_pipeline:could not process X: RuntimeError; boom\n"
    )
    assert parse_regression_log(log_path) == []


def test_parse_comparison_summary(tmp_path):
    log_path = tmp_path / "log.log"
    counts = {
        "matched": 99,
        "mismatched": 1,
        "prefix_only": 97,
        "key_only": 97,
        "warning_only": 3,
        "both_failed": 2,
    }
    log_path.write_text(
        "INFO:root:2026-09-14T12:00:00: Using 'libinchi.so'.\n"
        f"INFO:root:2026-09-14T13:00:00: Comparison summary: {json.dumps(counts)}\n"
    )

    assert parse_comparison_summary(log_path) == counts
    assert parse_comparison_summary(tmp_path / "log.log").get("both_failed") == 2


def test_parse_comparison_summary_absent(tmp_path):
    log_path = tmp_path / "empty.log"
    log_path.write_text("INFO:root:nothing here\n")
    assert parse_comparison_summary(log_path) == {}


import gzip
from inchi_tests.campaign import recompute_subset
from inchi_tests.utils import get_molfile_id_pubchem


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

METHANOL_MOLFILE = """methanol
  test

  2  1  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 C   0  0
    1.0000    0.0000    0.0000 O   0  0
  1  2  1  0
M  END
$$$$
"""


def test_recompute_subset_only_touches_requested_ids(lib_path, tmp_path):
    sdf_path = tmp_path / "tiny.sdf.gz"
    with gzip.open(sdf_path, "wt", encoding="utf-8") as sdf_file:
        sdf_file.write(ETHANOL_MOLFILE)
        sdf_file.write(METHANOL_MOLFILE)

    # `get_molfile_id_pubchem` takes the first whitespace token, which is the title
    # line for these hand-written molfiles.
    results = recompute_subset(
        sdf_paths=[sdf_path],
        ids_by_sdf={"tiny.sdf.gz": {"methanol"}},
        inchi_lib_path=lib_path,
        inchi_api_parameters="",
        get_molfile_id=get_molfile_id_pubchem,
        number_of_processes=1,
    )

    assert set(results) == {"methanol"}
    # Raw, prefix included.
    assert results["methanol"]["inchi"] == "InChI=1S/CH4O/c1-2/h2H,1H3"
    assert results["methanol"]["exit"] == 0


def test_recompute_subset_skips_sdfs_with_no_requested_ids(lib_path, tmp_path):
    sdf_path = tmp_path / "tiny.sdf.gz"
    with gzip.open(sdf_path, "wt", encoding="utf-8") as sdf_file:
        sdf_file.write(ETHANOL_MOLFILE)

    assert (
        recompute_subset(
            sdf_paths=[sdf_path],
            ids_by_sdf={"other.sdf.gz": {"x"}},
            inchi_lib_path=lib_path,
            inchi_api_parameters="",
            get_molfile_id=get_molfile_id_pubchem,
            number_of_processes=1,
        )
        == {}
    )
