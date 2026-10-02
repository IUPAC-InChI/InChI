import csv
import json
import pytest
from pathlib import Path
from molecular_inorganics.classify import Mismatch
from molecular_inorganics.report import (
    differing_fields,
    load_classifications,
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
            dict(molfile_id="1", sdf="A.sdf.gz", category="reconnected_as_recmet",
                 reference_inchi="InChI=1S/x", dev_mi_inchi="InChI=1B/y",
                 recmet_inchi=PTEN_RECMET),
            # salt pathway: RecMet emitted no /r layer at all
            dict(molfile_id="2", sdf="A.sdf.gz", category="salt_kept_bonded",
                 reference_inchi="InChI=1S/C8H11N.2ClH.Hg/c;;;",
                 dev_mi_inchi="InChI=1B/C8H11N.Cl2Hg/c;1-3-2",
                 recmet_inchi="InChI=1/C8H11N.2ClH.Hg/c;;;"),
            # metal pathway, but the two disagree
            dict(molfile_id="3", sdf="B.sdf.gz", category="reconnected_differently",
                 reference_inchi="InChI=1S/C12H28Sn",
                 dev_mi_inchi="InChI=1B/C12H28Sn/c1-4",
                 recmet_inchi="InChI=1/C12H28Sn/c;/rC12H28Sn/c9-9"),
        ],
    )
    return path


def _log_line(entry_kind: str, molfile_id: str, current: dict, reference: dict) -> str:
    entry = {
        "time": "2026-10-02T10:00:00",
        "molfile_id": molfile_id,
        "sdf": "A.sdf.gz",
        "info": {},
        "diff": {"current": json.dumps(current), "reference": json.dumps(reference)},
    }
    return f"INFO:sdf_pipeline:regression test {entry_kind}:{json.dumps(entry)}\n"


ROW = {"inchi": "InChI=1S/CH4/h1H4", "key": "VNWKTOKETHGBQD-UHFFFAOYSA-N", "exit": 0}


@pytest.fixture
def run_b_log(tmp_path):
    path = tmp_path / "run_b.log"
    path.write_text(
        "INFO:root:2026-10-02T10:00:00: Starting to process 2 SDFs on 16 cores.\n"
        "INFO:root:2026-10-02T10:02:00: Processed 2/2 (100.00%) SDFs; Ran regression on x.\n"
    )
    return path


@pytest.fixture
def run_c_log(tmp_path):
    """A control pass with one unexpected and one expected mismatch."""
    path = tmp_path / "run_c.log"
    path.write_text(
        "INFO:root:2026-10-02T10:00:00: Starting to process 2 SDFs on 16 cores.\n"
        + _log_line("failed", "7", {**ROW, "exit": 1}, ROW)
        + _log_line("failed expectedly", "8", {**ROW, "inchi": "InChI=1S/CH3"}, ROW)
        + "INFO:root:2026-10-02T10:01:00: Comparison summary: "
        '{"matched": 98, "mismatched": 2}\n'
    )
    return path


@pytest.fixture
def summary_path(tmp_path):
    path = tmp_path / "summary.json"
    path.write_text(json.dumps({
        "counts": {"reconnected_as_recmet": 1, "salt_kept_bonded": 1, "reconnected_differently": 1},
        "total": 3,
        "comparison": {"matched": 97, "mismatched": 3, "prefix_only": 97,
                       "key_only": 97, "warning_only": 1, "both_failed": 0},
    }))
    return path


def test_load_classifications(classifications_path):
    rows = load_classifications(classifications_path)
    assert len(rows) == 3
    assert rows[0]["category"] == "reconnected_as_recmet"


def test_element_census_excludes_carbon_and_hydrogen(classifications_path):
    rows = [r for r in load_classifications(classifications_path) if r["category"] != "reconnected_as_recmet"]
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


def test_build_report_data_keeps_the_comparisons_apart(
    classifications_path, summary_path, run_b_log, run_c_log
):
    data = build_report_data(
        classifications_path=classifications_path,
        summary_path=summary_path,
        baseline_label="v1.07.5 · 11a8798",
        test_label="dev · f75a737",
        run_b_log=run_b_log,
        run_c_log=run_c_log,
        expected_structures=100,
    )
    assert set(data) >= {"reference", "a_vs_c", "a_vs_b"}

    option = data["a_vs_b"]
    assert option["total_structures"] == 100          # matched + mismatched
    assert option["mismatch_rate"] == pytest.approx(3.0)
    assert option["categories"]["salt_kept_bonded"] == 1
    assert option["not_reproduced"] == 2
    assert option["gates"]["prefix"] is True          # 97 + 0 == 97
    assert option["gates"]["completeness"] is True    # 97 + 3 + 0 == 100

    control = data["a_vs_c"]
    assert control["matched"] == 98
    assert control["unexpected"] == 1
    assert control["expected"] == 1
    assert control["gates"]["clean"] is False
    assert control["gates"]["completeness"] is True   # 98 + 2 + 0 == 100


def test_control_lists_which_fields_differ(classifications_path, summary_path, run_c_log):
    data = build_report_data(
        classifications_path=classifications_path,
        summary_path=summary_path,
        baseline_label="base",
        test_label="test",
        run_c_log=run_c_log,
    )
    control = data["a_vs_c"]
    assert [row["fields"] for row in control["mismatches"]] == [["exit"], ["inchi"]]
    assert control["field_counts"] == {"exit": 1, "inchi": 1}


def test_differing_fields_covers_a_timed_out_reference_row():
    # A reference row for a molfile that timed out in Run A has no InChI fields.
    mismatch = Mismatch(
        molfile_id="1", sdf="A.sdf.gz", current=ROW,
        reference={"timeout_seconds": 60.0}, expected=False,
    )
    assert differing_fields(mismatch) == ["exit", "inchi", "key", "timeout_seconds"]


def test_completeness_is_checked_against_an_independent_count(
    classifications_path, summary_path, run_b_log
):
    """The gate must be able to fail.

    It once read `matched + mismatched == total` with `total` defined as their
    sum -- a tautology that rendered green on precisely the short-total runs it
    exists to catch, such as a run where shards aborted."""
    data = build_report_data(
        classifications_path=classifications_path,
        summary_path=summary_path,
        baseline_label="base",
        test_label="test",
        run_b_log=run_b_log,
        expected_structures=120,  # 20 structures never reached the comparison
    )
    assert data["a_vs_b"]["gates"]["completeness"] is False


def test_completeness_counts_timed_out_molfiles(
    classifications_path, summary_path, tmp_path
):
    """A timed-out molfile is never compared but is still a reference row."""
    log = tmp_path / "run_b.log"
    log.write_text(
        'INFO:sdf_pipeline:timed out expectedly:{"molfile_id": "141382403"}\n'
    )
    data = build_report_data(
        classifications_path=classifications_path,
        summary_path=summary_path,
        baseline_label="base",
        test_label="test",
        run_b_log=log,
        expected_structures=101,  # 97 + 3 compared, 1 timed out
    )
    assert data["a_vs_b"]["timeouts"] == 1
    assert data["a_vs_b"]["gates"]["completeness"] is True


@pytest.mark.parametrize("with_count", [True, False])
def test_completeness_needs_both_the_count_and_the_log(
    classifications_path, summary_path, run_b_log, with_count
):
    # Without the reference row count there is nothing independent to check
    # against; without the log the timeouts are unknown.
    data = build_report_data(
        classifications_path=classifications_path,
        summary_path=summary_path,
        baseline_label="base",
        test_label="test",
        run_b_log=None if with_count else run_b_log,
        expected_structures=100 if with_count else None,
    )
    assert data["a_vs_b"]["gates"]["completeness"] is None


def test_an_unmeasured_comparison_is_not_a_100_percent_mismatch(
    classifications_path, tmp_path
):
    """A missing summary line must not read as a measured total.

    Defaulting to 0 matched and len(rows) mismatched put the headline at exactly
    100.00% changed over a total that was only the mismatch count, and passed the
    prefix gate on 0 + 0 == 0."""
    summary_path = tmp_path / "summary_no_comparison.json"
    summary_path.write_text(json.dumps({"counts": {"reconnected_as_recmet": 1}}))

    data = build_report_data(
        classifications_path=classifications_path,
        summary_path=summary_path,
        baseline_label="base",
        test_label="test",
        expected_structures=100,
    )
    option = data["a_vs_b"]
    assert option["matched"] is None
    assert option["mismatched"] is None
    assert option["total_structures"] is None
    assert option["mismatch_rate"] is None
    assert option["gates"]["prefix"] is None
    assert option["gates"]["completeness"] is None
    # And it still renders, with dashes where the numbers would be.
    assert "—" in render_html(data)


def test_no_aborted_gates_are_per_pass(
    classifications_path, summary_path, run_b_log, tmp_path
):
    aborted = tmp_path / "run_c.log"
    aborted.write_text("ERROR:root:1/1; Aborted regression on x.sdf.gz due to X.\n")
    data = build_report_data(
        classifications_path=classifications_path,
        summary_path=summary_path,
        baseline_label="base",
        test_label="test",
        run_a_log=tmp_path / "absent.log",
        run_b_log=run_b_log,
        run_c_log=aborted,
    )
    # A log that could not be opened is not evidence that nothing aborted.
    assert data["reference"]["no_aborted"] is None
    assert data["a_vs_b"]["gates"]["no_aborted"] is True
    assert data["a_vs_c"]["gates"]["no_aborted"] is False


def test_run_c_not_measured_renders_as_such(classifications_path, summary_path):
    """No Run C log means the control was not measured for this report.

    Rendering that as a green zero asserts a check nobody performed -- the README's
    'rebuild the report from an existing run' invocation passes no logs."""
    data = build_report_data(
        classifications_path=classifications_path,
        summary_path=summary_path,
        baseline_label="base",
        test_label="test",
    )
    assert set(data["a_vs_c"]["gates"].values()) == {None}
    html = render_html(data)
    assert "Not measured" in html
    assert "reproduces the baseline exactly" not in html


def _part(html: str, part_id: str) -> str:
    start = html.index(f'id="{part_id}"')
    end = html.find('<section class="part ', start + 1)
    return html[start : end if end != -1 else len(html)]


def test_each_part_holds_only_its_own_comparison(
    classifications_path, summary_path, run_b_log, run_c_log
):
    html = render_html(
        build_report_data(
            classifications_path=classifications_path,
            summary_path=summary_path,
            baseline_label="base",
            test_label="test",
            run_b_log=run_b_log,
            run_c_log=run_c_log,
            expected_structures=100,
        )
    )
    control, option = _part(html, "a-vs-c"), _part(html, "a-vs-b")

    assert "A vs B" not in control and "prefix_only" not in control
    assert "A vs C" not in option and "unexpected mismatches" not in option
    # Every sub-heading names its comparison.
    assert control.count("<h3>") == control.count('<span class="tag">A vs C</span>')
    assert option.count("<h3>") == option.count('<span class="tag">A vs B</span>')
    # The control's mismatches are listed in its own part.
    assert "(expected)" in control


def test_specimen_captions_are_not_double_escaped(classifications_path, summary_path):
    data = build_report_data(
        classifications_path=classifications_path,
        summary_path=summary_path,
        baseline_label="base",
        test_label="test",
    )
    html = render_html(data)
    assert "&amp;mdash;" not in html
    assert "no /r layer — salt disconnection is not reversible by RecMet" in html


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
    assert html.count("--ctl:") == 3
    # No external anything except the Google Fonts stylesheet the CSP allows.
    assert "cdnjs" not in html
    assert html.count("<script") == 0
    # Real measured numbers, not placeholders.
    assert "100" in html and "reconnected_as_recmet" in html
    assert "TODO" not in html and "lorem" not in html.lower()


def test_without_metal_and_route_gates_need_a_classification_that_checked_them(
    classifications_path, summary_path
):
    # The fixture's summary predates both checks: not checked, never a pass.
    data = build_report_data(
        classifications_path=classifications_path,
        summary_path=summary_path,
        baseline_label="base",
        test_label="test",
    )
    assert data["a_vs_b"]["gates"]["changed_without_metal"] is None
    assert data["a_vs_b"]["gates"]["route"] is None
    assert "Route check: not checked" in render_html(data)


def test_changes_without_a_metal_and_route_disagreements_fail_their_gates(
    classifications_path, tmp_path
):
    summary_path = tmp_path / "summary.json"
    summary_path.write_text(json.dumps({
        "counts": {"reconnected_as_recmet": 1, "salt_kept_bonded": 1, "reconnected_differently": 1, "changed_without_metal": 4},
        "route_check": {"agrees": 2, "disagrees": 1},
        "comparison": {"matched": 93, "mismatched": 7},
    }))
    data = build_report_data(
        classifications_path=classifications_path,
        summary_path=summary_path,
        baseline_label="base",
        test_label="test",
    )
    option = data["a_vs_b"]
    assert option["gates"]["changed_without_metal"] is False
    assert option["gates"]["route"] is False
    assert option["route"] == {
        "agrees": 2, "disagrees": 1, "no_disconnection": 0, "no_metal": 0, "not_checked": 0
    }

    html = _part(render_html(data), "a-vs-b")
    assert "4 structures without a metal changed their InChI." in html
    assert "ids/changed_without_metal.txt" in html


def test_message_only_is_shown_in_the_option_part(classifications_path, tmp_path):
    summary_path = tmp_path / "summary.json"
    summary_path.write_text(json.dumps({
        "counts": {"reconnected_as_recmet": 1, "salt_kept_bonded": 1, "reconnected_differently": 1},
        "comparison": {"matched": 97, "mismatched": 3, "message_only": 12},
    }))
    data = build_report_data(
        classifications_path=classifications_path,
        summary_path=summary_path,
        baseline_label="base",
        test_label="test",
    )
    option = _part(render_html(data), "a-vs-b")
    assert '<td>message_only</td><td class="n">12</td>' in option


def test_warning_changes_without_a_metal_fail_their_own_gate(classifications_path, tmp_path):
    summary_path = tmp_path / "summary.json"
    summary_path.write_text(json.dumps({
        "counts": {"reconnected_as_recmet": 1, "salt_kept_bonded": 1, "reconnected_differently": 1},
        "route_check": {"agrees": 3},
        "comparison": {"matched": 97, "mismatched": 3,
                       "without_metal_warning_only": 2, "without_metal_message_only": 5},
    }))
    data = build_report_data(
        classifications_path=classifications_path,
        summary_path=summary_path,
        baseline_label="base",
        test_label="test",
    )
    option = data["a_vs_b"]
    assert option["without_metal_warnings"] == 7
    assert option["gates"]["without_metal_warnings"] is False
    assert option["gates"]["changed_without_metal"] is True
    assert "7 warning changes on structures without a metal" in _part(render_html(data), "a-vs-b")


def test_without_metal_warning_gate_is_not_checked_on_an_older_log(
    classifications_path, summary_path
):
    data = build_report_data(
        classifications_path=classifications_path,
        summary_path=summary_path,
        baseline_label="base",
        test_label="test",
    )
    assert data["a_vs_b"]["gates"]["without_metal_warnings"] is None


def test_prefix_section_names_each_outcome(classifications_path, tmp_path):
    changed = {"reconnected_as_recmet": 40, "reconnected_differently": 7, "salt_kept_bonded": 11,
               "changed_without_disconnection": 2}
    unchanged = {"changed_without_metal": 5}
    summary_path = tmp_path / "summary.json"
    summary_path.write_text(json.dumps({
        "counts": {"reconnected_as_recmet": 1, "salt_kept_bonded": 1, "reconnected_differently": 1},
        "prefix": {"prefix_changed": changed, "prefix_unchanged": unchanged},
        "comparison": {"matched": 97, "mismatched": 65, "prefix_only": 90, "unaffected": 85},
    }))
    data = build_report_data(
        classifications_path=classifications_path,
        summary_path=summary_path,
        baseline_label="base",
        test_label="test",
    )
    option = _part(render_html(data), "a-vs-b")
    section = option[option.index("Prefix changes"):option.index("What the categories mean")]

    def row(name):
        start = section.index(f'<td class="mono">{name}</td>')
        return section[start:section.index("</tr>", start)]

    assert '<td class="n">85</td>' in row("unaffected")
    assert '<td class="n">5</td>' in row("unaffected_warning_changed")
    assert '<td class="n">40</td>' in row("reconnected_as_recmet")
    assert '<td class="n">11</td>' in row("salt_kept_bonded")
    assert '<td class="n">5</td>' in row("changed_without_metal")


def test_prefix_section_is_not_checked_on_older_output(classifications_path, summary_path):
    data = build_report_data(
        classifications_path=classifications_path,
        summary_path=summary_path,
        baseline_label="base",
        test_label="test",
    )
    option = _part(render_html(data), "a-vs-b")
    assert "Prefix changes" in option
    assert '<td class="n">—</td>' in option
