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


def test_recompute_subset_merges_results_from_the_process_pool(lib_path, tmp_path):
    # `number_of_processes=1` bypasses the pool entirely, but the campaign runs on
    # the pool path, where `get_molfile_id` and `consumer` are pickled to spawned
    # workers. Several shards with distinct IDs also exercise the per-shard merge.
    sdf_paths = []
    for shard in range(3):
        sdf_path = tmp_path / f"shard{shard}.sdf.gz"
        with gzip.open(sdf_path, "wt", encoding="utf-8") as sdf_file:
            sdf_file.write(ETHANOL_MOLFILE.replace("ethanol", f"ethanol{shard}", 1))
            sdf_file.write(METHANOL_MOLFILE.replace("methanol", f"methanol{shard}", 1))
        sdf_paths.append(sdf_path)

    results = recompute_subset(
        sdf_paths=sdf_paths,
        ids_by_sdf={path.name: {f"ethanol{i}"} for i, path in enumerate(sdf_paths)},
        inchi_lib_path=lib_path,
        inchi_api_parameters="-RecMet",
        get_molfile_id=get_molfile_id_pubchem,
        number_of_processes=3,
    )

    assert set(results) == {"ethanol0", "ethanol1", "ethanol2"}
    for result in results.values():
        # -RecMet clears the standard-format flag, so the prefix is `InChI=1/`.
        assert result["inchi"] == "InChI=1/C2H6O/c1-2-3/h3H,2H2,1H3"
        assert result["exit"] == 0


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


from inchi_tests.campaign import (
    Mismatch,
    classify_mismatches,
    classification_counts,
    reconnected_layer,
)
from inchi_tests.consumers import inchi_body


def _mismatch(molfile_id, current, reference):
    return Mismatch(
        molfile_id=molfile_id, sdf="A.sdf.gz", current=current,
        reference=reference, expected=False,
    )


def test_reconnected_layer_extracts_the_r_layer_without_a_prefix():
    assert reconnected_layer(PTEN_RECMET) == inchi_body(PTEN_MI)
    assert reconnected_layer(PTEN_RECMET) == "C2H6Cl2N2Pt/c3-7(4)5-1-2-6-7/h5-6H,1-2H2"
    # No /r layer: the prefix-stripped InChI, unchanged.
    assert reconnected_layer("InChI=1S/CH4O/c1-2/h2H,1H3") == "CH4O/c1-2/h2H,1H3"


def test_classify_recmet_equivalent():
    mismatches = [
        _mismatch("1", {"inchi": PTEN_MI, "key": "K1", "exit": 0},
                  {"inchi": PTEN_PLAIN, "key": "K0", "exit": 1})
    ]
    recmet = {"1": {"inchi": PTEN_RECMET, "key": "K2", "exit": 1}}

    result = classify_mismatches(mismatches, recmet)

    assert result[0].category == "recmet_equivalent"
    # Raw strings, prefixes intact, are what gets reported.
    assert result[0].recmet_inchi == PTEN_RECMET
    assert result[0].dev_mi_inchi == PTEN_MI
    assert result[0].reference_inchi == PTEN_PLAIN


def test_classify_novel():
    mismatches = [_mismatch("2", {"inchi": "InChI=1B/X/c1", "key": "K", "exit": 0},
                            {"inchi": "InChI=1S/Y/c1", "key": "K", "exit": 0})]
    recmet = {"2": {"inchi": "InChI=1/Z/c1", "key": "K", "exit": 0}}

    assert classify_mismatches(mismatches, recmet)[0].category == "novel"


def test_classify_error_under_mi_takes_precedence_over_recmet():
    mismatches = [_mismatch("3", {"inchi": "", "key": "", "exit": 2},
                            {"inchi": "InChI=1S/X/c1", "key": "K", "exit": 0})]
    recmet = {"3": {"inchi": "", "key": "", "exit": 2}}

    assert classify_mismatches(mismatches, recmet)[0].category == "error_under_mi"


def test_classify_error_in_reference_wins_over_error_under_mi():
    mismatches = [_mismatch("4", {"inchi": "", "key": "", "exit": 2},
                            {"inchi": "", "key": "", "exit": 2})]

    assert classify_mismatches(mismatches, {})[0].category == "error_in_reference"


def test_classify_recmet_failed():
    mismatches = [_mismatch("5b", {"inchi": PTEN_MI, "key": "K", "exit": 0},
                            {"inchi": PTEN_PLAIN, "key": "K", "exit": 1})]
    recmet = {"5b": {"inchi": "", "key": "", "exit": 2}}

    assert classify_mismatches(mismatches, recmet)[0].category == "recmet_failed"


def test_classify_recmet_missing():
    mismatches = [_mismatch("5", {"inchi": "InChI=1B/X/c1", "key": "K", "exit": 0},
                            {"inchi": "InChI=1S/Y/c1", "key": "K", "exit": 0})]

    result = classify_mismatches(mismatches, {})

    assert result[0].category == "recmet_missing"
    assert result[0].recmet_inchi == ""


def test_a_warning_level_exit_is_never_a_failure_category():
    # exit=1 ("Metal was disconnected") must not reach a failure category.
    mismatches = [_mismatch("6", {"inchi": PTEN_MI, "key": "K", "exit": 0},
                            {"inchi": PTEN_PLAIN, "key": "K", "exit": 1})]
    recmet = {"6": {"inchi": PTEN_RECMET, "key": "K", "exit": 1}}

    assert classify_mismatches(mismatches, recmet)[0].category == "recmet_equivalent"


def test_classification_counts():
    mismatches = [
        _mismatch("1", {"inchi": PTEN_MI, "key": "K", "exit": 0},
                  {"inchi": PTEN_PLAIN, "key": "K", "exit": 1}),
        _mismatch("2", {"inchi": "InChI=1B/X/c1", "key": "K", "exit": 0},
                  {"inchi": "InChI=1S/Y/c1", "key": "K", "exit": 0}),
    ]
    recmet = {
        "1": {"inchi": PTEN_RECMET, "key": "K", "exit": 1},
        "2": {"inchi": "InChI=1/Z/c1", "key": "K", "exit": 0},
    }

    assert classification_counts(classify_mismatches(mismatches, recmet)) == {
        "recmet_equivalent": 1,
        "novel": 1,
    }


import csv
from inchi_tests.campaign import Classification, write_report


def test_write_report_emits_csv_and_summary(tmp_path):
    classifications = [
        Classification(molfile_id="1", sdf="A.sdf.gz", category="novel",
                       reference_inchi="InChI=1S/X/c1", dev_mi_inchi="InChI=1B/Y/c1",
                       recmet_inchi="InChI=1/Z/c1"),
        Classification(molfile_id="2", sdf="A.sdf.gz", category="recmet_equivalent",
                       reference_inchi=PTEN_PLAIN, dev_mi_inchi=PTEN_MI,
                       recmet_inchi=PTEN_RECMET),
    ]
    comparison_summary = {
        "matched": 999,
        "mismatched": 2,
        "prefix_only": 990,
        "key_only": 990,
        "warning_only": 7,
        "both_failed": 9,
    }

    write_report(classifications, tmp_path, comparison_summary)

    with open(tmp_path / "classifications.csv", newline="", encoding="utf-8") as csv_file:
        rows = list(csv.DictReader(csv_file))
    assert [row["molfile_id"] for row in rows] == ["1", "2"]
    assert rows[0]["category"] == "novel"
    # Raw InChIs with prefixes reach the CSV.
    assert rows[1]["recmet_inchi"] == PTEN_RECMET
    assert rows[1]["dev_mi_inchi"].startswith("InChI=1B/")

    summary = json.loads((tmp_path / "summary.json").read_text(encoding="utf-8"))
    assert summary["counts"] == {"novel": 1, "recmet_equivalent": 1}
    assert summary["total"] == 2
    assert summary["comparison"] == comparison_summary


def _classification(molfile_id, category, recmet_inchi=""):
    return Classification(
        molfile_id=molfile_id, sdf="A.sdf.gz", category=category,
        reference_inchi="InChI=1S/x", dev_mi_inchi="InChI=1B/x",
        recmet_inchi=recmet_inchi,
    )


def test_cause_of_splits_novel_by_disconnection_pathway():
    from inchi_tests.campaign import cause_of

    # An /r layer means the old code disconnected the metal and -RecMet put it
    # back; without one it used salt disconnection, which -RecMet cannot undo.
    assert cause_of(_classification("1", "novel", PTEN_RECMET)) == "novel_metal_pathway"
    assert cause_of(_classification("2", "novel", "InChI=1/C8H11N.2ClH.Hg")) == "novel_salt_pathway"
    assert cause_of(_classification("3", "novel", "")) == "novel_salt_pathway"
    assert cause_of(_classification("4", "recmet_equivalent", PTEN_RECMET)) == "recmet_equivalent"
    assert cause_of(_classification("5", "error_under_mi")) == "error_under_mi"


def test_write_id_lists_one_file_per_cause(tmp_path):
    from inchi_tests.campaign import write_id_lists, ID_LIST_CAUSES

    classifications = [
        _classification("300", "recmet_equivalent", PTEN_RECMET),
        _classification("42", "recmet_equivalent", PTEN_RECMET),
        _classification("7", "novel", PTEN_RECMET),
        _classification("1000", "novel", "InChI=1/no-r-layer"),
        _classification("9", "error_under_mi"),
    ]

    counts = write_id_lists(classifications, tmp_path)

    ids_dir = tmp_path / "ids"
    # Every cause gets a file, even when empty, so downstream tooling can rely
    # on the set of filenames rather than probing for them.
    assert {p.name for p in ids_dir.iterdir()} == {f"{c}.txt" for c in ID_LIST_CAUSES}

    # Numeric IDs sort numerically, not as strings: 42 before 300.
    assert (ids_dir / "recmet_equivalent.txt").read_text() == "42\n300\n"
    assert (ids_dir / "novel_metal_pathway.txt").read_text() == "7\n"
    assert (ids_dir / "novel_salt_pathway.txt").read_text() == "1000\n"
    assert (ids_dir / "error_under_mi.txt").read_text() == "9\n"
    assert (ids_dir / "recmet_missing.txt").read_text() == ""

    assert counts["recmet_equivalent"] == 2
    assert counts["novel_metal_pathway"] == 1
    assert counts["recmet_missing"] == 0


def test_write_id_lists_handles_non_numeric_ids(tmp_path):
    from inchi_tests.campaign import write_id_lists

    write_id_lists(
        [
            _classification("_Elements.#070", "novel", ""),
            _classification("mcule-42", "novel", ""),
        ],
        tmp_path,
    )

    assert (tmp_path / "ids" / "novel_salt_pathway.txt").read_text() == (
        "_Elements.#070\nmcule-42\n"
    )
