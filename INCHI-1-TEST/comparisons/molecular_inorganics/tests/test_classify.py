import json
import pytest
from pathlib import Path
from molecular_inorganics.classify import (
    parse_regression_log,
    mismatch_ids_by_sdf,
    parse_comparison_summary,
)


LIB_PATH = "CMake_build/full_build/INCHI-1-SRC/INCHI_API/libinchi/src/lib/libinchi.so"

# Measured on the built libraries for Pt(en)Cl2.
PTEN_PLAIN = "InChI=1S/C2H6N2.2ClH.Pt/c3-1-2-4;;;/h3-4H,1-2H2;2*1H;/q-2;;;+4/p-2"
PTEN_MI = "InChI=1BB/C2H6Cl2N2Pt/c3-7(4)5-1-2-6-7/h5-6H,1-2H2"
PTEN_RECMET = (
    "InChI=1/C2H6N2.2ClH.Pt/c3-1-2-4;;;/h3-4H,1-2H2;2*1H;/q-2;;;+4/p-2"
    "/rC2H6Cl2N2Pt/c3-7(4)5-1-2-6-7/h5-6H,1-2H2"
)


@pytest.fixture
def lib_path():
    path = Path(LIB_PATH)
    if not path.is_file():
        pytest.skip(f"{LIB_PATH} not built; run ./INCHI-1-TEST/build_with_cmake.sh all")
    return str(path)


def _log_line(molfile_id, sdf, current, reference, expectedly=False):
    entry = {
        "time": "2026-09-14T12:00:00",
        "molfile_id": molfile_id,
        "sdf": sdf,
        "info": {"consumer": "raw-regression", "parameters": "-MolecularInorganics"},
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
            {"inchi": "InChI=1BB/X", "key": "K3", "exit": 0},
            {"inchi": "InChI=1S/Y", "key": "K4", "exit": 0},
            expectedly=True,
        )
        + "INFO:root:2026-09-14T12:30:00: Ran regression on Compound_000000001_000500000.sdf.gz.\n"
    )

    mismatches = parse_regression_log(log_path)

    assert len(mismatches) == 2
    assert mismatches[0].molfile_id == "10"
    # The log keeps the prefix unchanged.
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
from molecular_inorganics.classify import recompute_subset
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
    # `number_of_processes=1` bypasses the pool entirely, but the comparison runs on
    # the pool path, where `get_molfile_id` and `consumer` are pickled to spawned
    # workers. Several SDFs with distinct IDs also exercise the merge across SDFs.
    sdf_paths = []
    for index in range(3):
        sdf_path = tmp_path / f"test{index}.sdf.gz"
        with gzip.open(sdf_path, "wt", encoding="utf-8") as sdf_file:
            sdf_file.write(ETHANOL_MOLFILE.replace("ethanol", f"ethanol{index}", 1))
            sdf_file.write(METHANOL_MOLFILE.replace("methanol", f"methanol{index}", 1))
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
        # -RecMet produces non-standard output: `InChI=1N/` since #280.
        assert result["inchi"] == "InChI=1N/C2H6O/c1-2-3/h3H,2H2,1H3"
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


from molecular_inorganics.classify import (
    Mismatch,
    classify_mismatches,
    classification_counts,
    reconnected_layer,
)
from molecular_inorganics.identifiers import inchi_body


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


def test_classify_reconnected_as_recmet():
    mismatches = [
        _mismatch("1", {"inchi": PTEN_MI, "key": "K1", "exit": 0},
                  {"inchi": PTEN_PLAIN, "key": "K0", "exit": 1})
    ]
    recmet = {"1": {"inchi": PTEN_RECMET, "key": "K2", "exit": 1}}

    result = classify_mismatches(mismatches, recmet)

    assert result[0].category == "reconnected_as_recmet"
    # Raw strings, prefixes intact, are what gets reported.
    assert result[0].recmet_inchi == PTEN_RECMET
    assert result[0].dev_mi_inchi == PTEN_MI
    assert result[0].reference_inchi == PTEN_PLAIN


@pytest.mark.parametrize(
    "message, recmet_inchi, category",
    [
        # The route is the one Run A states, not the /r proxy.
        ("Metal was disconnected", "InChI=1/Z/c1/rW/c2", "reconnected_differently"),
        ("Metal was disconnected; Salt was disconnected", "InChI=1/Z/c1/rW/c2", "reconnected_differently"),
        ("Salt was disconnected", "InChI=1/Z/c1", "salt_kept_bonded"),
        # The proxy would call this salt: no /r layer. 1.07.5 disconnected nothing.
        ("Proton(s) added/removed", "InChI=1/Z/c1", "changed_without_disconnection"),
    ],
)
def test_classify_what_recmet_does_not_reproduce_by_stated_route(message, recmet_inchi, category):
    # Y is yttrium: the structure has a metal.
    mismatches = [_mismatch("2", {"inchi": "InChI=1BB/X/c1", "key": "K", "exit": 0},
                            {"inchi": "InChI=1S/Y/c1", "key": "K", "exit": 0, "message": message})]
    recmet = {"2": {"inchi": recmet_inchi, "key": "K", "exit": 0, "message": message}}

    assert classify_mismatches(mismatches, recmet)[0].category == category


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
    mismatches = [_mismatch("5", {"inchi": "InChI=1BB/X/c1", "key": "K", "exit": 0},
                            {"inchi": "InChI=1S/Y/c1", "key": "K", "exit": 0})]

    result = classify_mismatches(mismatches, {})

    assert result[0].category == "recmet_missing"
    assert result[0].recmet_inchi == ""


def test_a_warning_level_exit_is_never_a_failure_category():
    # exit=1 ("Metal was disconnected") must not reach a failure category.
    mismatches = [_mismatch("6", {"inchi": PTEN_MI, "key": "K", "exit": 0},
                            {"inchi": PTEN_PLAIN, "key": "K", "exit": 1})]
    recmet = {"6": {"inchi": PTEN_RECMET, "key": "K", "exit": 1}}

    assert classify_mismatches(mismatches, recmet)[0].category == "reconnected_as_recmet"


def test_classification_counts():
    mismatches = [
        _mismatch("1", {"inchi": PTEN_MI, "key": "K", "exit": 0},
                  {"inchi": PTEN_PLAIN, "key": "K", "exit": 1}),
        _mismatch("2", {"inchi": "InChI=1BB/X/c1", "key": "K", "exit": 0},
                  {"inchi": "InChI=1S/Y/c1", "key": "K", "exit": 0}),
    ]
    recmet = {
        "1": {"inchi": PTEN_RECMET, "key": "K", "exit": 1},
        "2": {"inchi": "InChI=1/Z/c1", "key": "K", "exit": 0},
    }

    assert classification_counts(classify_mismatches(mismatches, recmet)) == {
        "reconnected_as_recmet": 1,
        "changed_without_disconnection": 1,
    }


import csv
from molecular_inorganics.classify import (
    Classification,
    a_vs_b_summary,
    write_classifications,
)


def write_report(classifications, output_dir, comparison=None):
    """What `classify.main` writes for A vs B, into one directory."""
    write_classifications(classifications, output_dir / "classifications.csv")
    (output_dir / "summary.json").write_text(
        json.dumps(a_vs_b_summary(classifications, comparison)), encoding="utf-8"
    )


def test_write_report_emits_csv_and_summary(tmp_path):
    classifications = [
        Classification(molfile_id="1", sdf="A.sdf.gz", category="salt_kept_bonded",
                       reference_inchi="InChI=1S/X/c1", dev_mi_inchi="InChI=1BB/Y/c1",
                       recmet_inchi="InChI=1/Z/c1"),
        Classification(molfile_id="2", sdf="A.sdf.gz", category="reconnected_as_recmet",
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
    assert rows[0]["category"] == "salt_kept_bonded"
    # Raw InChIs with prefixes reach the CSV.
    assert rows[1]["recmet_inchi"] == PTEN_RECMET
    assert rows[1]["dev_mi_inchi"].startswith("InChI=1BB/")

    summary = json.loads((tmp_path / "summary.json").read_text(encoding="utf-8"))
    assert summary["counts"] == {"salt_kept_bonded": 1, "reconnected_as_recmet": 1}
    assert summary["total"] == 2
    assert summary["comparison"] == comparison_summary


METAL = "Metal was disconnected"
SALT = "Salt was disconnected"


def _classification(molfile_id, category, recmet_inchi="", reference_message=""):
    return Classification(
        molfile_id=molfile_id, sdf="A.sdf.gz", category=category,
        reference_inchi="InChI=1S/x", dev_mi_inchi="InChI=1BB/x",
        recmet_inchi=recmet_inchi, reference_message=reference_message,
    )


def test_write_id_lists_one_file_per_cause(tmp_path):
    from molecular_inorganics.classify import write_id_lists, CATEGORIES

    classifications = [
        _classification("300", "reconnected_as_recmet", PTEN_RECMET),
        _classification("42", "reconnected_as_recmet", PTEN_RECMET),
        _classification("7", "reconnected_differently", PTEN_RECMET, METAL),
        _classification("1000", "salt_kept_bonded", "InChI=1/no-r-layer", SALT),
        _classification("11", "changed_without_disconnection", "InChI=1/no-r-layer", ""),
        _classification("9", "error_under_mi"),
    ]

    counts = write_id_lists(classifications, tmp_path)

    ids_dir = tmp_path / "ids"
    # Every cause gets a file, even when empty, so downstream tooling can rely
    # on the set of filenames rather than probing for them.
    assert {p.name for p in ids_dir.iterdir()} == {f"{c}.txt" for c in CATEGORIES}

    # Numeric IDs sort numerically, not as strings: 42 before 300.
    assert (ids_dir / "reconnected_as_recmet.txt").read_text() == "42\n300\n"
    assert (ids_dir / "reconnected_differently.txt").read_text() == "7\n"
    assert (ids_dir / "salt_kept_bonded.txt").read_text() == "1000\n"
    assert (ids_dir / "changed_without_disconnection.txt").read_text() == "11\n"
    assert (ids_dir / "error_under_mi.txt").read_text() == "9\n"
    assert (ids_dir / "recmet_missing.txt").read_text() == ""

    assert counts["reconnected_as_recmet"] == 2
    assert counts["reconnected_differently"] == 1
    assert counts["recmet_missing"] == 0


def test_write_id_lists_handles_non_numeric_ids(tmp_path):
    from molecular_inorganics.classify import write_id_lists

    write_id_lists(
        [
            _classification("_Elements.#070", "salt_kept_bonded", "", SALT),
            _classification("mcule-42", "salt_kept_bonded", "", SALT),
        ],
        tmp_path,
    )

    assert (tmp_path / "ids" / "salt_kept_bonded.txt").read_text() == (
        "_Elements.#070\nmcule-42\n"
    )


def test_classification_carries_keys_and_exit_codes():
    # The log holds key and exit for both sides; without them on the row the CSV
    # cannot answer "which mismatches changed the InChIKey" without parsing logs.
    mismatches = [
        Mismatch(
            molfile_id="7",
            sdf="A.sdf.gz",
            current={"inchi": PTEN_MI, "key": "AAA-UHFFFAOYBA-N", "exit": 0},
            reference={"inchi": PTEN_PLAIN, "key": "BBB-UHFFFAOYSA-N", "exit": 1},
            expected=False,
        )
    ]
    recmet = {"7": {"inchi": PTEN_RECMET, "key": "CCC-UHFFFAOYNA-N", "exit": 1}}

    result = classify_mismatches(mismatches, recmet)[0]

    assert result.reference_key == "BBB-UHFFFAOYSA-N"
    assert result.dev_mi_key == "AAA-UHFFFAOYBA-N"
    assert result.recmet_key == "CCC-UHFFFAOYNA-N"
    assert result.reference_exit == 1
    assert result.dev_mi_exit == 0
    assert result.recmet_exit == 1


def test_missing_recmet_leaves_key_and_exit_blank():
    mismatches = [
        Mismatch(
            molfile_id="8", sdf="A.sdf.gz",
            current={"inchi": "InChI=1BB/X", "key": "K1", "exit": 0},
            reference={"inchi": "InChI=1S/Y", "key": "K0", "exit": 0},
            expected=False,
        )
    ]

    result = classify_mismatches(mismatches, {})[0]

    assert result.category == "recmet_missing"
    assert result.recmet_key == ""
    assert result.recmet_exit is None


def test_csv_columns_include_keys_and_exits(tmp_path):
    from molecular_inorganics.classify import CSV_FIELDS

    for field in ("reference_key", "dev_mi_key", "recmet_key",
                  "reference_exit", "dev_mi_exit", "recmet_exit"):
        assert field in CSV_FIELDS

    write_report(
        [
            Classification(
                molfile_id="9", sdf="A.sdf.gz", category="salt_kept_bonded",
                reference_inchi="InChI=1S/Y", dev_mi_inchi="InChI=1BB/X",
                recmet_inchi="InChI=1/Z",
                reference_key="K0", dev_mi_key="K1", recmet_key="K2",
                reference_exit=1, dev_mi_exit=0, recmet_exit=1,
            )
        ],
        tmp_path,
    )

    with open(tmp_path / "classifications.csv", newline="", encoding="utf-8") as f:
        row = next(csv.DictReader(f))
    assert row["dev_mi_key"] == "K1"
    assert row["reference_exit"] == "1"
    assert row["dev_mi_exit"] == "0"


import re
from molecular_inorganics.classify import (
    METALS,
    has_metal,
    route_check,
)

ELDATA = Path(__file__).parents[4] / "INCHI-1-SRC/INCHI_BASE/src/eldata.c"


def test_metals_match_the_element_table_of_the_library():
    """`METALS` must be what `is_el_a_metal` returns true for, nothing else."""
    rows = re.findall(
        r'^\s*\{\s*"([A-Za-z]{1,3})"\s*,\s*[\d.]+\s*,\s*[\d.]+\s*,\s*[\d.]+\s*,\s*(\w+)\s*,',
        ELDATA.read_text(encoding="utf-8"),
        re.M,
    )
    assert rows, f"no element rows parsed from {ELDATA}"
    assert METALS == {symbol for symbol, kind in rows if kind in ("METAL", "METAL2")}


def test_has_metal_reads_the_formula_layer_only():
    assert has_metal(PTEN_MI)
    assert has_metal("InChI=1S/ClH.Na/h1H;/q;+1/p-1")
    # Ge and As are not metals in InChI's element table.
    assert not has_metal("InChI=1S/AsH3/h1H3")
    assert not has_metal("InChI=1S/C7H11N2/c1-9(2)6-7-3-4-8-5-7/h3-6,8H,1-2H3/q+1")
    assert not has_metal("")


def test_a_mismatch_without_a_metal_is_its_own_category():
    # Mcule 7099468173: MI has no mechanism to change a structure without a metal.
    mismatches = [_mismatch(
        "7099468173",
        {"inchi": "InChI=1BB/C7H11N2/c1-9(2)6-7-3-4-8-5-7/h3-6,8H,1-2H3/q+1", "key": "K", "exit": 0},
        {"inchi": "InChI=1S/C7H10N2/c1-9(2)6-7-3-4-8-5-7/h3-6H,1-2H3/p+1", "key": "K", "exit": 0},
    )]

    classification = classify_mismatches(mismatches, {})[0]
    assert classification.category == "changed_without_metal"
    # Not `no_disconnection`, which on a structure with a metal is a finding.
    assert classification.route_check == "no_metal"


def test_a_failure_wins_over_changed_without_metal():
    mismatches = [_mismatch("1", {"inchi": "", "key": "", "exit": 2},
                            {"inchi": "InChI=1S/CH4/h1H4", "key": "K", "exit": 0})]

    assert classify_mismatches(mismatches, {})[0].category == "error_under_mi"


@pytest.mark.parametrize(
    "inchi, message, expected",
    [
        (PTEN_RECMET, "Metal was disconnected", "agrees"),
        (PTEN_RECMET, "Salt was disconnected", "disagrees"),
        ("InChI=1/C8H11N.2ClH.Hg/c;;;", "Salt was disconnected", "agrees"),
        ("InChI=1/C8H11N.2ClH.Hg/c;;;", "Metal was disconnected", "disagrees"),
        (PTEN_RECMET, "Metal was disconnected; Salt was disconnected", "agrees"),
        ("InChI=1/C8H11N.2ClH.Hg/c;;;", "Metal was disconnected; Salt was disconnected", "disagrees"),
        ("InChI=1/C8H11N.2ClH.Hg/c;;;", "", "no_disconnection"),
        # Cut off at 256 characters before "Metal was disconnected".
        (PTEN_RECMET, "Charge not recognized: M  CHG  8  49 -37  51 -37...; W(1)", "message_truncated"),
    ],
)
def test_route_check_compares_the_r_proxy_with_the_baseline_message(inchi, message, expected):
    assert route_check({"inchi": inchi, "key": "K", "exit": 1, "message": message}) == expected


def test_route_check_without_a_usable_recmet_result_is_not_checked():
    assert route_check(None) == "not_checked"
    assert route_check({"inchi": "", "key": "", "exit": 2, "message": ""}) == "not_checked"


def test_classification_carries_the_message_and_route_check(tmp_path):
    mismatches = [_mismatch("1", {"inchi": PTEN_MI, "key": "K1", "exit": 0},
                            {"inchi": PTEN_PLAIN, "key": "K2", "exit": 1})]
    recmet = {"1": {"inchi": PTEN_RECMET, "key": "K3", "exit": 1,
                    "message": "Metal was disconnected"}}

    classification = classify_mismatches(mismatches, recmet)[0]
    assert classification.recmet_message == "Metal was disconnected"
    assert classification.route_check == "agrees"

    write_report([classification], tmp_path)
    summary = json.loads((tmp_path / "summary.json").read_text(encoding="utf-8"))
    assert summary["route_check"] == {"agrees": 1}
    with open(tmp_path / "classifications.csv", newline="", encoding="utf-8") as csv_file:
        assert next(csv.DictReader(csv_file))["route_check"] == "agrees"


def test_messages_of_all_three_sides_reach_the_classification():
    from molecular_inorganics.classify import explain_failures

    mismatches = [
        _mismatch("1", {"inchi": PTEN_MI, "key": "K1", "exit": 0, "message": ""},
                  {"inchi": PTEN_PLAIN, "key": "K2", "exit": 1, "message": "Metal was disconnected"}),
        _mismatch("2", {"inchi": "", "key": "", "exit": 2, "message": "Unknown element"},
                  {"inchi": PTEN_PLAIN, "key": "K2", "exit": 1, "message": "Metal was disconnected"}),
    ]
    recmet = {"1": {"inchi": PTEN_RECMET, "key": "K3", "exit": 1, "message": "Metal was disconnected"}}

    first, second = classify_mismatches(mismatches, recmet)
    assert first.reference_message == "Metal was disconnected"
    assert first.dev_mi_message == ""
    assert first.recmet_message == "Metal was disconnected"
    # Failures under MI are explained from the stored message, no re-run.
    assert explain_failures([first, second]) == {"2": "Unknown element"}


def test_prefix_breakdown_counts_each_category_by_prefix_change(tmp_path):
    from molecular_inorganics.classify import prefix_breakdown

    def row(molfile_id, reference, current, category):
        return Classification(molfile_id=molfile_id, sdf="A.sdf.gz", category=category,
                              reference_inchi=reference, dev_mi_inchi=current,
                              recmet_inchi="")

    rows = [
        row("1", PTEN_PLAIN, PTEN_MI, "reconnected_as_recmet"),
        row("2", "InChI=1S/C8H11N.2ClH.Hg/c;;;", "InChI=1BB/C8H11N.Cl2Hg/c;1-3-2", "salt_kept_bonded"),
        # Prefix kept: the #280 case of a change without a metal.
        row("3", "InChI=1S/C7H10N2/c1", "InChI=1S/C7H11N2/c1", "changed_without_metal"),
        # A failed side has no prefix and is left out.
        row("4", "", PTEN_MI, "error_in_reference"),
    ]

    breakdown = prefix_breakdown(rows)
    assert breakdown["prefix_changed"]["reconnected_as_recmet"] == 1
    assert breakdown["prefix_changed"]["salt_kept_bonded"] == 1
    assert breakdown["prefix_unchanged"]["changed_without_metal"] == 1
    assert sum(breakdown["prefix_changed"].values()) + sum(breakdown["prefix_unchanged"].values()) == 3

    write_report(rows, tmp_path)
    summary = json.loads((tmp_path / "summary.json").read_text(encoding="utf-8"))
    assert summary["prefix"]["prefix_changed"]["salt_kept_bonded"] == 1


from molecular_inorganics.classify import (
    LEGACY_CATEGORIES,
    a_vs_c_summary,
    classify_legacy_mismatches,
    split_message,
    write_id_lists,
)

STAR = "C26H41N3O19Zz2.Gd/c30-14(31)5-27"


def _legacy_mismatch(molfile_id, legacy, reference):
    return Mismatch(molfile_id=molfile_id, sdf="A.sdf.gz", current=legacy,
                    reference=reference, expected=False)


def test_a_vs_c_mismatches_are_explained_by_npzz_or_not():
    failed = {"inchi": "", "key": "", "exit": 2, "message": "Unknown element"}
    legacy = {"inchi": f"InChI=1N/{STAR}", "key": "IROITDSZMKWTSF-UHFFFAOYNA-L", "exit": 0, "message": ""}
    mismatches = [
        # 1.07.5 rejected it; with -NPZz it gives the legacy result (prefix aside).
        _legacy_mismatch("6", legacy, failed),
        # -NPZz gives another InChI, for no known reason.
        _legacy_mismatch("7", legacy, failed),
        # No -NPZz result at all.
        _legacy_mismatch("8", legacy, failed),
    ]
    npzz = {
        "6": {"inchi": f"InChI=1B/{STAR}", "key": "IROITDSZMKWTSF-UHFFFAOYBA-L", "exit": 0, "message": ""},
        "7": {"inchi": "InChI=1B/C26H41N3O19Zz2.Gd/c1", "key": "IROITDSZMKWTSF-UHFFFAOYBA-L", "exit": 0, "message": ""},
    }

    rows = classify_legacy_mismatches(mismatches, npzz, a_vs_b_ids={"6", "7"})

    assert [row.category for row in rows] == ["explained_by_npzz", "changed_otherwise", "unexplained"]
    assert [row.in_a_vs_b for row in rows] == [True, True, False]
    assert rows[0].npzz_inchi.startswith("InChI=1B/")
    assert rows[2].npzz_inchi == ""

    summary = a_vs_c_summary(rows, {"matched": 97, "mismatched": 3})
    assert summary["counts"] == {"explained_by_npzz": 1, "changed_otherwise": 1, "unexplained": 1}
    assert summary["cross_check"] == {"also_in_a_vs_b": 2, "warning_only": 0, "only_in_a_vs_c": 1}
    assert summary["comparison"]["mismatched"] == 3


def test_a_vs_c_cross_check_leaves_out_warning_only_differences():
    """Run B does not compare warnings, so a difference only in the message
    cannot show up there."""
    inchi = "InChI=1N/356CH4.197H2/h356*1H4;197*1H"
    legacy = {"inchi": inchi, "key": "K", "exit": 1, "message": "V3000 star atoms ignored if MolecularInorganics or NPZz parameter not used"}
    reference = {"inchi": "InChI=1S/356CH4.197H2/h356*1H4;197*1H", "key": "K", "exit": 1, "message": "V3000 star atoms ignored"}
    rows = classify_legacy_mismatches(
        [_legacy_mismatch("9", legacy, reference)], {"9": {**reference, "inchi": inchi}}, a_vs_b_ids=set()
    )

    assert rows[0].category == "message_text_changed"
    assert a_vs_c_summary(rows)["cross_check"] == {"also_in_a_vs_b": 0, "warning_only": 1, "only_in_a_vs_c": 0}


def test_a_vs_c_id_lists_have_one_file_per_category(tmp_path):
    failed = {"inchi": "", "key": "", "exit": 2, "message": ""}
    legacy = {"inchi": f"InChI=1N/{STAR}", "key": "K", "exit": 0, "message": ""}
    rows = classify_legacy_mismatches(
        [_legacy_mismatch("6", legacy, failed)], {}, a_vs_b_ids=set()
    )

    counts = write_id_lists(rows, tmp_path, LEGACY_CATEGORIES)

    assert counts == {category: 0 for category in LEGACY_CATEGORIES} | {"unexplained": 1}
    assert (tmp_path / "ids" / "unexplained.txt").read_text() == "6\n"
    assert (tmp_path / "ids" / "explained_by_npzz.txt").read_text() == ""


KMNO4 = "K.Mn.H2O.3O/h;;1H2;;;/q2*+1;;;;/p-1"


def _result(inchi, message, exit=1, key="AAAAAAAAAAAAAA-BBBBBBBBSA-N"):
    return {"inchi": inchi, "key": key, "exit": exit, "message": message}


@pytest.mark.parametrize(
    ("legacy", "npzz", "category"),
    [
        (
            _result(f"InChI=1N/{KMNO4}", "Accepted unusual valence(s): N(4); Metal was disconnected"),
            _result(f"InChI=1S/{KMNO4}", "Accepted unusual valence(s): Mn(7); N(4); Metal was disconnected"),
            "mn7_warning_dropped",
        ),
        (
            _result(f"InChI=1N/{KMNO4}", "Metal was disconnected"),
            _result(f"InChI=1S/{KMNO4}", "Accepted unusual valence(s): Mn(7); Metal was disconnected"),
            "mn7_warning_dropped",
        ),
        (
            _result("InChI=1N/C2Yb2/c1-2", "Accepted unusual valence(s): Yb(19); Yb(22); Metal was disconnected"),
            _result("", "", exit=2, key=""),
            "accepted_above_old_maxval",
        ),
        # A long message is cut off after the last valence.
        (
            _result("InChI=1N/BH26/h1H26", "Accepted unusual valence(s): B(15); B(20); B(21)..."),
            _result("", "", exit=2, key=""),
            "accepted_above_old_maxval",
        ),
        # No valence warning, but the /c layer gives atom 1 21 bonds.
        (
            _result("InChI=1N/I22/c1(2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21)22", "", exit=0),
            _result("", "", exit=2, key=""),
            "accepted_above_old_maxval",
        ),
        (
            _result("InChI=1N/I3/c1-2-3", "", exit=0),
            _result("", "", exit=2, key=""),
            "accepted_otherwise",
        ),
        (
            _result(f"InChI=1N/{KMNO4}", "V3000 star atoms ignored if MolecularInorganics or NPZz parameter not used"),
            _result(f"InChI=1S/{KMNO4}", "V3000 star atoms ignored"),
            "message_text_changed",
        ),
        (
            _result(f"InChI=1N/{KMNO4}", "Omitted undefined stereo"),
            _result(f"InChI=1S/{KMNO4}", "V3000 enhanced stereo read/stored but ignored; Omitted undefined stereo"),
            "message_text_changed",
        ),
        (
            _result("InChI=1N/C2H5O.Na/c1-2-3;", "Proton(s) added/removed"),
            _result("InChI=1S/C2H6O.Na/c1-2-3;/h3H,2H2,1H3;", "Metal was disconnected; Proton(s) added/removed"),
            "metal_not_disconnected",
        ),
        (
            _result("InChI=1N/C2H5O.Na/c1-2-3;", "Metal was disconnected"),
            _result("InChI=1S/C2H6O.Na/c1-2-3;/h3H,2H2,1H3;", "Metal was disconnected"),
            "changed_otherwise",
        ),
        # 1.07.5 gave seven neutral Ca implicit H, then disconnected them.
        (
            _result("InChI=1N/12Ca.ClH.Fe/h;;;;;;;;;;;;1H;/q;;;;;;;5*+2;;+1/p-1", "Metal was disconnected"),
            _result("InChI=1S/12Ca.ClH.Fe.14H/h;;;;;;;;;;;;1H;;;;;;;;;;;;;;;/q;;;;;;;5*+2;;+1;;;;;;;;;;;;;;/p-1",
                    "Metal was disconnected"),
            "no_hydrogen_on_metal",
        ),
        # The hydrogen was the Na's only bond, so Run C disconnects nothing.
        (
            _result("InChI=1N/C2H6O.Na/c1-2-3;/h3H,2H2,1H3;", "Not chiral"),
            _result("InChI=1S/C2H6O.Na.H/c1-2-3;;/h3H,2H2,1H3;;", "Metal was disconnected; Not chiral"),
            "no_hydrogen_on_metal",
        ),
        # Bare H, but another layer changed too.
        (
            _result("InChI=1N/C2H6O.Na/c1-2-3;/h3H,2H2,1H3;/q;+1", "Metal was disconnected"),
            _result("InChI=1S/C2H6O.Na.H/c1-2-3;;/h3H,2H2,1H3;;", "Metal was disconnected"),
            "changed_otherwise",
        ),
        # Same InChI, but a message change no rule covers.
        (
            _result(f"InChI=1N/{KMNO4}", "Not chiral"),
            _result(f"InChI=1S/{KMNO4}", "Proton(s) added/removed"),
            "unexplained",
        ),
        # The Mn(7) warning dropped, but the exit code changed too.
        (
            _result(f"InChI=1N/{KMNO4}", "", exit=0),
            _result(f"InChI=1S/{KMNO4}", "Accepted unusual valence(s): Mn(7)"),
            "unexplained",
        ),
    ],
)
def test_a_vs_c_mismatches_get_the_category_of_their_cause(legacy, npzz, category):
    reference = _result(npzz["inchi"], npzz["message"], npzz["exit"], npzz["key"])
    rows = classify_legacy_mismatches(
        [_legacy_mismatch("1", legacy, reference)], {"1": npzz}, a_vs_b_ids=set()
    )

    assert rows[0].category == category


def test_split_message_separates_the_valence_list():
    valences, others = split_message(
        "Charge not recognized: M  CHG  1 2 -1; Accepted unusual valence(s): Mn(7); "
        "Mn+1(6); C-1(4); Metal was disconnected; Not chiral"
    )

    assert valences == ["Mn(7)", "Mn+1(6)", "C-1(4)"]
    assert others == ["Charge not recognized: M  CHG  1 2 -1", "Metal was disconnected", "Not chiral"]


def test_a_timed_out_reference_row_is_classified_not_a_crash():
    """A molfile that timed out in Run A has a reference row without an InChI."""
    rows = classify_mismatches(
        [_mismatch("1", {"inchi": PTEN_MI, "key": "K", "exit": 0, "message": ""},
                   {"timeout_seconds": 60.0})],
        {},
    )

    assert rows[0].category == "error_in_reference"


from molecular_inorganics.classify import DIFFERENCES, a_vs_b_summary, difference_kind
from molecular_inorganics.identifiers import metal_bonds, most_bonds


def test_metal_bonds_counts_bonds_per_metal_and_partner():
    # Hexachloroplatinate: six Pt-Cl bonds, one component.
    assert metal_bonds("Cl6Pt/c1-7(2,3,4,5)6") == (1, {("Pt", "Cl"): 6})
    # Bu3SnH as MI writes it: the H (atom 14) is bonded to Sn in /c.
    components, bonds = metal_bonds("C12H28Sn/c1-4-7-10-13(14,11-8-5-2)12-9-6-3/h4-12H2,1-3H3")
    assert components == 1 and bonds == {("Sn", "C"): 3, ("Sn", "H"): 1}
    # Repeated components on both sides line up.
    assert metal_bonds("2C2H2O4.Zn/c2*3-1(4)2(5)6;") == (3, {})
    # A /c layer that does not fit the formula.
    assert metal_bonds("Cl2Pt/c1-3-9") is None


def test_most_bonds_counts_each_component_separately():
    assert most_bonds("Cl6Pt/c1-7(2,3,4,5)6") == 6
    # Atom 6 has four neighbours in each component, not eight in total.
    assert most_bonds("C6H13NO2.C5H11NO2/c1-6(2,5(8)9)7(3)4;1-5(2,6-3)4(7)8") == 4
    assert most_bonds("2C2H2O4.Zn/c2*3-1(4)2(5)6;") == 3
    assert most_bonds("Na") == 0


@pytest.mark.parametrize(
    ("dev_mi", "recmet", "kind"),
    [
        # 3139146: only the Sn-H bond is extra.
        ("InChI=1BB/C12H28Sn/c1-4-7-10-13(14,11-8-5-2)12-9-6-3/h4-12H2,1-3H3",
         "InChI=1/3C4H9.Sn.H/c3*1-3-4-2;;/h3*1,3-4H2,2H3;;/rC12H28Sn/c1-4-7-10-13(11-8-5-2)12-9-6-3/h13H,4-12H2,1-3H3",
         "metal_hydrogen_bonded"),
        # 71150: PtCl6 drawn as Pt4+ with four Cl-; MI keeps two Pt-Cl bonds.
        ("InChI=1BB/Cl2Pt.4ClH/c1-3-2;;;;/h;4*1H/q+4;;;;/p-4",
         "InChI=1/6ClH.Pt/h6*1H;/q;;;;;;+6/p-6/rCl6Pt/c1-7(2,3,4,5)6",
         "partly_disconnected"),
        # 227945: K cut off as K+.
        ("InChI=1BB/C15H15O10.K/c1-21-11(16)6-7(12(17)22-2)9(14(19)24-4)10(15(20)25-5)8(6)13(18)23-3;/h1-5H3;/q-1;+1",
         "InChI=1/C15H15O10.K/c1-21-11(16)6-7(12(17)22-2)9(14(19)24-4)10(15(20)25-5)8(6)13(18)23-3;/h1-5H3;"
         "/rC15H15KO10/c1-22-10(17)6-7(11(18)23-2)9(13(20)25-4)15(16,14(21)26-5)8(6)12(19)24-3/h1-5H3",
         "group_1_2_disconnected"),
        # 794909: MI keeps two Fe-Cl bonds 1.07.5 broke as a salt.
        ("InChI=1BB/2C5H5.Cl4Fe2O.Cl2Fe/c2*1-2-4-5-3-1;1-5(2)7-6(3)4;1-3-2/h2*1-5H;;",
         "InChI=1/2C5H5.6ClH.3Fe.O/c2*1-2-4-5-3-1;;;;;;;;;;/h2*1-5H;6*1H;;;;/q;;;;;;;;3*+2;/p-6"
         "/r2C5H5.Cl4Fe2O.2ClH.Fe/c2*1-2-4-5-3-1;1-5(2)7-6(3)4;;;/h2*1-5H;;2*1H;/q;;;;;+2/p-2",
         "more_bonds_kept"),
        # Same bonds to metals, different hydrogens.
        ("InChI=1BB/Cl2Pt/c1-3-2/q+1", "InChI=1/2ClH.Pt/rCl2Pt/c1-3-2", "same_metal_bonds"),
        ("InChI=1BB/Cl2Pt/c1-3-9", "InChI=1/2ClH.Pt/rCl2Pt/c1-3-2", "not_parsed"),
    ],
)
def test_difference_kind_reads_how_the_metal_bonds_differ(dev_mi, recmet, kind):
    assert kind in DIFFERENCES
    assert difference_kind(dev_mi, recmet) == kind


def test_reconnected_differently_gets_a_difference_and_its_own_id_files(tmp_path):
    reference = {"inchi": "InChI=1S/6ClH.Pt/h6*1H;/q;;;;;;+6/p-6", "key": "K", "exit": 1,
                 "message": "Metal was disconnected"}
    recmet = {"inchi": "InChI=1/6ClH.Pt/h6*1H;/q;;;;;;+6/p-6/rCl6Pt/c1-7(2,3,4,5)6", "key": "K",
              "exit": 1, "message": "Metal was disconnected"}
    rows = classify_mismatches(
        [_mismatch("71150", {"inchi": "InChI=1BB/Cl2Pt.4ClH/c1-3-2;;;;/h;4*1H/q+4;;;;/p-4",
                             "key": "K", "exit": 0}, reference),
         _mismatch("1", {"inchi": PTEN_MI, "key": "K1", "exit": 0},
                   {"inchi": PTEN_PLAIN, "key": "K0", "exit": 1})],
        {"71150": recmet, "1": {"inchi": PTEN_RECMET, "key": "K2", "exit": 1}},
    )

    assert [(row.category, row.difference) for row in rows] == [
        ("reconnected_differently", "partly_disconnected"), ("reconnected_as_recmet", "")]
    assert a_vs_b_summary(rows)["differences"] == {"partly_disconnected": 1}
    counts = write_id_lists(rows, tmp_path, DIFFERENCES, "difference", "ids/reconnected_differently")
    assert counts["partly_disconnected"] == 1 and sum(counts.values()) == 1
    assert (tmp_path / "ids" / "reconnected_differently" / "partly_disconnected.txt").read_text() == "71150\n"
    assert (tmp_path / "ids" / "reconnected_differently" / "mixed.txt").read_text() == ""
