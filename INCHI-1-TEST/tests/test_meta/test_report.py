import csv
import json
import pytest
from pathlib import Path
from inchi_tests.report import (
    load_classifications,
    novel_split,
    element_census,
    run_duration,
    build_report_data,
    render_html,
)


PTEN_RECMET = (
    "InChI=1/C2H6N2.2ClH.Pt/c3-1-2-4;;;/h3-4H,1-2H2;2*1H;/q-2;;;+4/p-2"
    "/rC2H6Cl2N2Pt/c3-7(4)5-1-2-6-7/h5-6H,1-2H2"
)


def _write_csv(path, rows):
    fields = [
        "molfile_id",
        "sdf",
        "category",
        "reference_inchi",
        "dev_mi_inchi",
        "recmet_inchi",
    ]
    with open(path, "w", newline="", encoding="utf-8") as f:
        writer = csv.DictWriter(f, fieldnames=fields)
        writer.writeheader()
        writer.writerows(rows)


@pytest.fixture
def classifications_path(tmp_path):
    path = tmp_path / "classifications.csv"
    _write_csv(
        path,
        [
            # metal pathway, MI reproduces the /r layer
            dict(molfile_id="1", sdf="A.sdf.gz", category="recmet_equivalent",
                 reference_inchi="InChI=1S/x", dev_mi_inchi="InChI=1B/y",
                 recmet_inchi=PTEN_RECMET),
            # salt pathway: RecMet emitted no /r layer at all
            dict(molfile_id="2", sdf="A.sdf.gz", category="novel",
                 reference_inchi="InChI=1S/C8H11N.2ClH.Hg/c;;;",
                 dev_mi_inchi="InChI=1B/C8H11N.Cl2Hg/c;1-3-2",
                 recmet_inchi="InChI=1/C8H11N.2ClH.Hg/c;;;"),
            # metal pathway, but the two disagree
            dict(molfile_id="3", sdf="B.sdf.gz", category="novel",
                 reference_inchi="InChI=1S/C12H28Sn",
                 dev_mi_inchi="InChI=1B/C12H28Sn/c1-4",
                 recmet_inchi="InChI=1/C12H28Sn/c;/rC12H28Sn/c9-9"),
        ],
    )
    return path


@pytest.fixture
def summary_path(tmp_path):
    path = tmp_path / "summary.json"
    path.write_text(json.dumps({
        "counts": {"recmet_equivalent": 1, "novel": 2},
        "total": 3,
        "comparison": {"matched": 97, "mismatched": 3, "prefix_only": 97,
                       "key_only": 97, "warning_only": 1, "both_failed": 0},
    }))
    return path


def test_load_classifications(classifications_path):
    rows = load_classifications(classifications_path)
    assert len(rows) == 3
    assert rows[0]["category"] == "recmet_equivalent"


def test_novel_split_uses_the_r_layer_as_the_pathway_proxy(classifications_path):
    rows = load_classifications(classifications_path)
    split = novel_split(rows)
    # No /r layer => the old code broke the bond via salt disconnection, which
    # -RecMet cannot undo.
    assert split["salt_pathway"] == 1
    # An /r layer means metal disconnection, which -RecMet does reverse.
    assert split["metal_pathway"] == 1


def test_element_census_excludes_carbon_and_hydrogen(classifications_path):
    rows = [r for r in load_classifications(classifications_path) if r["category"] == "novel"]
    census = element_census(rows)
    assert census["Hg"] == 1
    assert census["Cl"] == 1
    assert census["Sn"] == 1
    assert "C" not in census and "H" not in census


def test_run_duration_reads_first_and_last_timestamp(tmp_path):
    log = tmp_path / "run.log"
    log.write_text(
        "INFO:root:2026-09-15T11:50:57: Using 'libinchi.so'.\n"
        "INFO:root:2026-09-15T11:50:57: Starting to process 3 SDFs on 16 cores.\n"
        "INFO:root:2026-09-15T11:53:19: Processed 3/3 (100.00%) SDFs; Ran regression on x.\n"
    )
    assert run_duration(log) == 142.0


def test_run_duration_missing_log_is_none(tmp_path):
    assert run_duration(tmp_path / "absent.log") is None


def test_build_report_data_assembles_gates(classifications_path, summary_path):
    data = build_report_data(
        classifications_path=classifications_path,
        summary_path=summary_path,
        baseline_label="v1.07.5 · 11a8798",
        test_label="dev · f75a737",
    )
    assert data["total_structures"] == 100          # matched + mismatched
    assert data["mismatch_rate"] == pytest.approx(3.0)
    assert data["novel"]["salt_pathway"] == 1
    assert data["gates"]["run_b_prefix_gate"] is True     # 97 + 0 == 97
    assert data["gates"]["completeness"] is True          # 97 + 3 == 100


def test_render_html_is_self_contained_and_theme_aware(classifications_path, summary_path):
    data = build_report_data(
        classifications_path=classifications_path,
        summary_path=summary_path,
        baseline_label="v1.07.5 · 11a8798",
        test_label="dev · f75a737",
    )
    html = render_html(data)
    assert "<title>" in html
    # Theme tokens must exist in all three states, or the page renders one theme's
    # text on the other theme's ground.
    assert ":root{" in html.replace(" ", "")
    assert 'prefers-color-scheme: dark' in html
    assert ':root[data-theme="dark"]' in html
    # No external anything except the Google Fonts stylesheet the CSP allows.
    assert "cdnjs" not in html
    assert html.count("<script") == 0
    # Real measured numbers, not placeholders.
    assert "100" in html and "recmet_equivalent" in html
    assert "TODO" not in html and "lorem" not in html.lower()
