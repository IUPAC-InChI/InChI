"""Render the comparison report from the output of `classify.py`.

Turns `classifications.csv`, `summary.json` and the three run logs into a
self-contained HTML page, so every run of the comparison produces the same
report without hand-assembly.

The page is written for readers who are not InChI developers: it opens with the
findings in plain words, explains the background and every check, and keeps the
category names, tallies and exact rules in a technical section of each part.

The page keeps the two comparisons strictly apart. Run A, the reference, has a
block of its own; A vs C (the control, `-LegacyMetalHandling`) and A vs B
(Molecular Inorganics) each get a part with their own figures, gates, blind spots
and cost, built only from that comparison's data.

Every category is named for what happened to the structure (see `classify.py`);
the disconnection route behind them is the one Run A's own warning states, and
the `/r` layer of the `-RecMet` re-run is checked against it per structure.
"""

import argparse
import csv
import json
import re
from collections import Counter
from datetime import datetime
from html import escape
from pathlib import Path

from molecular_inorganics.classify import (
    CATEGORIES,
    LEGACY_CATEGORIES,
    ROUTE_CHECKS,
    has_reconnected_layer,
    parse_comparison_summary,
)

_TIMESTAMP = re.compile(r"(\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2})")
_ELEMENT = re.compile(r"[A-Z][a-z]?")
_ORGANIC = {"C", "H"}

# Log lines, as `sdf_pipeline` and `run.py` write them.
ABORTED = "Aborted "
TIMED_OUT = "sdf_pipeline:timed out"
REFERENCE_TIMED_OUT = "sdf_pipeline:reference timed out"


def load_classifications(path: Path) -> list[dict]:
    with open(path, newline="", encoding="utf-8") as csv_file:
        return list(csv.DictReader(csv_file))


# Mismatches -RecMet does not reproduce: the ones a stated mechanism must explain.
NOT_REPRODUCED = (
    "reconnected_differently",
    "salt_kept_bonded",
    "changed_without_disconnection",
)


def element_census(rows: list[dict], limit: int = 14) -> dict[str, int]:
    """Count non-organic elements across the MI formulae of the given rows."""
    census: Counter = Counter()

    for row in rows:
        inchi = row["dev_mi_inchi"]
        if "/" not in inchi:
            continue
        formula_layer = inchi.split("/", 1)[1].split("/")[0]
        for fragment in formula_layer.split("."):
            for element in _ELEMENT.findall(fragment):
                if element not in _ORGANIC:
                    census[element] += 1

    return dict(census.most_common(limit))


def run_duration(log_path: Path) -> float | None:
    """Seconds between the first and last timestamped line of a run log."""
    try:
        with open(log_path, encoding="utf-8") as log_file:
            stamps = _TIMESTAMP.findall(log_file.read())
    except FileNotFoundError:
        return None

    if len(stamps) < 2:
        return None

    first = datetime.fromisoformat(stamps[0])
    last = datetime.fromisoformat(stamps[-1])

    return (last - first).total_seconds()


def count_log_lines(log_path: Path, needle: str) -> int | None:
    try:
        with open(log_path, encoding="utf-8") as log_file:
            return sum(needle in line for line in log_file)
    except FileNotFoundError:
        return None


# Light only: the report is read on screen and printed, so one palette serves both.
_CSS = """
:root{
  color-scheme:light;
  --bg:#F7F8FA; --surface:#FFFFFF; --surface-2:#F0F3F7;
  --ink:#1B2029; --muted:#5B6576; --line:#DCE1E8; --line-soft:#E9EDF2;
  --base:#2F5D8C; --mi:#9A5B2B; --ctl:#2E6F73;
  --good:#2E7D5B; --good-soft:#E5F2EB;
  --warn:#8F6400; --warn-soft:#FBF0D6;
  --flag:#A33A3A; --flag-soft:#F9E6E6;
  --na-soft:#EDEFF3;
  --serif:"IBM Plex Serif",Georgia,serif;
  --sans:"IBM Plex Sans",-apple-system,BlinkMacSystemFont,"Segoe UI",sans-serif;
  --mono:"IBM Plex Mono",ui-monospace,Menlo,monospace;
}
*{box-sizing:border-box}
html{background:var(--bg)}
body{background:var(--bg);color:var(--ink);font-family:"IBM Plex Sans",sans-serif;font-size:16px;
  line-height:1.6;margin:0;padding:0 16px}
.wrap{max-width:860px;margin:0 auto;padding:48px 0 72px;display:flex;
  flex-direction:column;gap:40px}
h1,h2,h3{font-family:var(--serif);margin:0;line-height:1.25;font-weight:600}
h1{font-size:clamp(28px,5vw,38px);letter-spacing:-.01em}
h2{font-size:24px}
h3{font-size:18px;display:flex;flex-wrap:wrap;align-items:baseline;gap:4px 10px}
p{margin:0} ul{margin:0;padding-left:1.2em} li+li{margin-top:6px}
section{display:flex;flex-direction:column;gap:14px}
code,.mono{font-family:var(--mono);font-size:.86em}
code{background:var(--surface-2);padding:1px 5px;border-radius:3px}
.code{font-family:var(--mono);font-size:11.5px;color:var(--muted);font-weight:400}
.eyebrow{font-size:12px;letter-spacing:.12em;text-transform:uppercase;color:var(--muted);
  font-weight:600}
.muted{color:var(--muted)}

header{display:flex;flex-direction:column;gap:14px;border-bottom:2px solid var(--ink);
  padding-bottom:22px}
.lede{font-size:18px;color:var(--muted);max-width:64ch}
.meta{display:flex;flex-wrap:wrap;gap:6px 26px;font-size:13.5px;color:var(--muted)}
.meta b{color:var(--ink);font-weight:600}

.box{background:var(--surface);border:1px solid var(--line);border-radius:6px;padding:20px 22px;
  display:flex;flex-direction:column;gap:12px}
.box.good{border-left:5px solid var(--good)}
.box.flag{border-left:5px solid var(--flag)}
.box.na{border-left:5px solid var(--muted)}
.box ul{font-size:16px}

.runs{display:grid;grid-template-columns:repeat(auto-fit,minmax(220px,1fr));gap:12px}
.run{background:var(--surface);border:1px solid var(--line);border-top:4px solid var(--part);
  border-radius:6px;padding:14px 16px;display:flex;flex-direction:column;gap:6px;font-size:14.5px}
.run b{font-size:16px}

.figs{display:grid;grid-template-columns:repeat(auto-fit,minmax(190px,1fr));gap:12px}
.fig{background:var(--surface);border:1px solid var(--line);border-radius:6px;padding:16px 18px;
  display:flex;flex-direction:column;gap:6px}
.fig .n{font-family:var(--serif);font-size:32px;font-weight:600;line-height:1.05;
  font-variant-numeric:tabular-nums}
.fig .k{font-size:14px;font-weight:600}
.fig .d{font-size:13.5px;color:var(--muted);line-height:1.5}
.fig.is-good .n{color:var(--good)} .fig.is-mi .n{color:var(--mi)} .fig.is-base .n{color:var(--base)}
.fig.is-flag .n{color:var(--flag)} .fig.is-muted .n{color:var(--muted)}

.pill{display:inline-block;font-size:12px;font-weight:600;padding:2px 9px;border-radius:999px;
  white-space:nowrap;line-height:1.5}
.pill.pass,.pill.expected{background:var(--good-soft);color:var(--good)}
.pill.fail,.pill.unexpected{background:var(--flag-soft);color:var(--flag)}
.pill.review{background:var(--warn-soft);color:var(--warn)}
.pill.na,.pill.neutral{background:var(--na-soft);color:var(--muted)}

table{border-collapse:collapse;width:100%;font-size:14px;background:var(--surface)}
.scroll{border:1px solid var(--line);border-radius:6px;overflow:hidden}
th,td{padding:9px 12px;text-align:left;vertical-align:top;border-bottom:1px solid var(--line-soft)}
thead th{background:var(--surface-2);font-size:12.5px;color:var(--muted);font-weight:600}
tbody tr:last-child td{border-bottom:none}
td.n,th.n{text-align:right;font-variant-numeric:tabular-nums;white-space:nowrap}
td .code{display:block;margin-top:2px}
.checks td:first-child{width:1%;white-space:nowrap}
.checks .tech{display:block;font-family:var(--mono);font-size:11.5px;color:var(--muted);margin-top:2px}

.stack{display:flex;height:22px;border-radius:4px;overflow:hidden;background:var(--surface-2)}
.stack span{display:block;height:100%;min-width:2px}
.swatch{display:inline-block;width:11px;height:11px;border-radius:2px;margin-right:7px;
  vertical-align:baseline}

.specimen{background:var(--surface);border:1px solid var(--line);border-left:4px solid var(--mi);
  border-radius:6px;overflow:hidden}
.specimen .cid{padding:11px 16px 4px;font-size:15px}
.specimen .why{padding:0 16px 10px;font-size:14px;color:var(--muted);
  border-bottom:1px solid var(--line-soft)}
.specimen dl{margin:0}
.specimen .row{display:grid;grid-template-columns:150px 1fr;gap:12px;padding:8px 16px}
.specimen .row+.row{border-top:1px solid var(--line-soft)}
.specimen dt{font-size:13px;color:var(--muted)}
.specimen dd{margin:0;font-family:var(--mono);font-size:12px;word-break:break-all}
.absent{color:var(--muted);font-style:italic;font-family:"IBM Plex Sans",sans-serif;font-size:13px}
.v{font-family:var(--mono);font-size:11.5px;word-break:break-all;display:block}
.v b{font-weight:500;color:var(--muted)}
table.diff,table.breakdown{table-layout:fixed}
table.diff .code{word-break:break-all}

.census{display:flex;flex-wrap:wrap;gap:6px}
.el{display:inline-block;border:1px solid var(--line);background:var(--surface);border-radius:4px;
  padding:3px 9px;font-size:13px}
.el b{font-weight:600} .el span{color:var(--muted);font-size:12px;margin-left:4px}

.callout{background:var(--surface);border:1px solid var(--line);border-left:5px solid var(--base);
  border-radius:6px;padding:14px 18px;display:flex;flex-direction:column;gap:6px;font-size:14.5px}
.callout.flagged{border-left-color:var(--flag)}
.callout.review{border-left-color:var(--warn)}

.part{display:flex;flex-direction:column;gap:28px;border-top:4px solid var(--part);padding-top:22px}
.part-a,.run-a{--part:var(--base)} .part-c,.run-c{--part:var(--ctl)} .part-b,.run-b{--part:var(--mi)}
.part-g{--part:var(--line)}
.part-head{display:flex;flex-direction:column;gap:10px}
.badge{align-self:flex-start;font-size:12.5px;font-weight:600;letter-spacing:.04em;color:#fff;
  background:var(--part);padding:3px 10px;border-radius:4px;white-space:nowrap}
.tag{font-family:"IBM Plex Sans",sans-serif;font-size:11px;font-weight:600;letter-spacing:.06em;color:var(--part);
  border:1px solid var(--part);border-radius:3px;padding:0 6px;white-space:nowrap}
.question{font-size:17px;max-width:66ch}
.answer{font-size:16px}
.tech-divider{display:flex;align-items:center;gap:12px;color:var(--muted);font-size:12px;
  letter-spacing:.12em;text-transform:uppercase;font-weight:600}
.tech-divider::before,.tech-divider::after{content:"";flex:1;border-top:1px dashed var(--line)}
.tech-intro{font-size:14px;color:var(--muted)}
.technical{font-size:14.5px}
.summary{display:grid;grid-template-columns:repeat(auto-fit,minmax(260px,1fr));gap:14px}
.card{background:var(--surface);border:1px solid var(--line);border-top:4px solid var(--part);
  border-radius:6px;padding:16px 20px;display:flex;flex-direction:column;gap:8px;
  text-decoration:none;color:inherit}
.card .n{font-family:var(--serif);font-size:32px;font-weight:600;line-height:1;color:var(--part);
  font-variant-numeric:tabular-nums}
.card .n.is-good{color:var(--good)} .card .n.is-flag{color:var(--flag)} .card .n.is-muted{color:var(--muted)}
.card .d{font-size:14px;color:var(--muted)}
dl.glossary{margin:0;display:flex;flex-direction:column;gap:0;background:var(--surface);
  border:1px solid var(--line);border-radius:6px}
dl.glossary div{padding:10px 16px;display:grid;grid-template-columns:190px 1fr;gap:12px}
dl.glossary div+div{border-top:1px solid var(--line-soft)}
dl.glossary dt{font-weight:600} dl.glossary dd{margin:0;font-size:14.5px}
footer{border-top:1px solid var(--line);padding-top:18px;font-size:13px;color:var(--muted);
  display:flex;flex-direction:column;gap:6px}

@media (max-width:600px){
  .specimen .row,dl.glossary div{grid-template-columns:1fr;gap:2px}
  .scroll{overflow-x:auto}
}

@page{size:A4;margin:16mm 14mm 18mm;
  @bottom-left{content:"InChI \\00B7  MolecularInorganics comparison";font-family:"IBM Plex Sans",sans-serif;
    font-size:8.5pt;color:#5B6576}
  @bottom-right{content:"Page " counter(page) " of " counter(pages);font-family:"IBM Plex Sans",sans-serif;
    font-size:8.5pt;color:#5B6576}}
/* Printing from the browser (or saving as PDF there) gives each comparison its
   own pages. */
@media print{
  html,body{background:#fff}
  body{font-size:10.5pt;padding:0}
  .wrap{padding:0;max-width:none}
  .part{break-before:page}
  .part-a{break-before:auto}
  .fig,.card,.run,.specimen,.callout,.box,tr,dl.glossary div{break-inside:avoid}
  h2,h3{break-after:avoid}
  a{color:inherit;text-decoration:none}
}
"""


_EXAMPLE_CATEGORIES = ("reconnected_as_recmet",) + NOT_REPRODUCED


def _pick_examples(rows: list[dict]) -> dict[str, dict | None]:
    """The first row of each mechanism, so the choice is deterministic."""
    examples: dict[str, dict | None] = {key: None for key in _EXAMPLE_CATEGORIES}

    for row in rows:
        if row["category"] in examples and not examples[row["category"]]:
            examples[row["category"]] = row
        if all(examples.values()):
            break

    return examples


def _fmt(n) -> str:
    return f"{n:,}" if isinstance(n, int) else ("—" if n is None else f"{n:.2f}")


def _pct(value: float | None, places: int = 2) -> str:
    """A percentage, or an em dash where there is no measurement to render."""
    return "—" if value is None else f"{value:.{places}f}%"


def _duration(seconds: float | None) -> str:
    if seconds is None:
        return "—"
    minutes, secs = divmod(int(seconds), 60)

    return f"{minutes} min {secs:02d} s" if minutes else f"{secs} s"

def _is_zero(count: int | None) -> bool | None:
    """A gate over a count that may not have been obtainable.

    `None` propagates as `None` -- not checked -- rather than collapsing to a pass.
    A log that could not be opened is not evidence that nothing went wrong in it."""
    return None if count is None else count == 0


def _share(part: int | None, whole: int | None) -> float | None:
    return part / whole * 100 if part is not None and whole else None


def _readable(log_path: Path | None) -> bool:
    return log_path is not None and Path(log_path).is_file()


def _no_aborted(log_path: Path | None) -> bool | None:
    """Whether no SDF was aborted in this pass, or `None` if its log is unreadable."""
    return _is_zero(count_log_lines(log_path, ABORTED)) if log_path else None


def _timeouts(log_path: Path | None) -> int | None:
    return count_log_lines(log_path, TIMED_OUT) if log_path else None


def _completeness(
    matched: int | None,
    mismatched: int | None,
    timeouts: int | None,
    expected_structures: int | None,
) -> bool | None:
    """Every reference row was either compared or timed out.

    Checked against the number of rows in the reference databases, which is
    counted independently of the comparator. Checking `matched + mismatched`
    against a total defined as their sum, as this once did, always passes, also on
    the incomplete runs the gate exists to catch. A molfile that timed out never
    reaches the comparator, so without the timeout count from the pass's own log
    the gate cannot be checked."""
    if None in (matched, mismatched, timeouts, expected_structures):
        return None

    return matched + mismatched + timeouts == expected_structures


def build_reference_data(
    run_a_log: Path | None, expected_structures: int | None
) -> dict:
    """Run A on its own: the rows both comparisons are measured against."""
    return {
        "rows": expected_structures,
        "timeouts": count_log_lines(run_a_log, REFERENCE_TIMED_OUT)
        if run_a_log
        else None,
        "no_aborted": _no_aborted(run_a_log),
        "duration": run_duration(run_a_log) if run_a_log else None,
    }


def build_control_data(
    run_c_log: Path | None,
    summary: dict | None,
    rows: list[dict],
    expected_structures: int | None,
) -> dict:
    """A vs C: the test build with `-LegacyMetalHandling` against Run A.

    The counts come from the A vs C section of `summary.json`, the timeouts,
    aborts and cost from Run C's own log. Without them the control was not
    measured for this report, and every figure and gate says so instead of
    rendering a zero nobody counted."""
    comparison = (summary or {}).get("comparison", {})
    measured = "matched" in comparison and "mismatched" in comparison
    matched = comparison["matched"] if measured else None
    mismatched = comparison["mismatched"] if measured else None
    categories = (
        {category: summary["counts"].get(category, 0) for category in LEGACY_CATEGORIES}
        if summary
        else None
    )
    cross_check = summary.get("cross_check") if summary else None
    timeouts = _timeouts(run_c_log)
    # Unexplained first: those are the ones to look at.
    listed = sorted(rows, key=lambda row: row["category"] != "unexplained")

    return {
        "measured": measured,
        "matched": matched,
        "mismatched": mismatched,
        "prefix_changed": comparison.get("prefix_changed"),
        "categories": categories,
        "cross_check": cross_check,
        "timeouts": timeouts,
        "rows": listed,
        "gates": {
            "explained": _is_zero(categories["unexplained"]) if categories else None,
            "cross_check": _is_zero(cross_check["only_in_a_vs_c"])
            if cross_check
            else None,
            "completeness": _completeness(
                matched, mismatched, timeouts, expected_structures
            ),
            "no_aborted": _no_aborted(run_c_log),
        },
        "duration": run_duration(run_c_log) if _readable(run_c_log) else None,
    }


def build_option_data(
    rows: list[dict],
    summary: dict,
    run_b_log: Path | None,
    expected_structures: int | None,
) -> dict:
    """A vs B: the test build with Molecular Inorganics on, prefix-insensitive."""
    comparison = summary.get("comparison", {})
    counts = summary.get("counts", {})

    # No summary line in the run log means the comparison was never measured,
    # e.g. because the log was truncated. Falling
    # back to 0 matched and len(rows) mismatched would make the total the mismatch
    # count, put the headline at exactly 100.00% changed, and pass the prefix gate
    # on 0 + 0 == 0 -- a measured-looking report of a measurement that never
    # happened. Carry the absence through as None instead and render it as such.
    measured = "matched" in comparison and "mismatched" in comparison
    matched = comparison["matched"] if measured else None
    mismatched = comparison["mismatched"] if measured else None
    total = matched + mismatched if measured else None
    timeouts = _timeouts(run_b_log)

    # The metal-free category and the route check exist only in classifications
    # written since they were added; an older summary has neither, and must not
    # read as zero changes without a metal or a passed check.
    checked = "route_check" in summary
    route = {key: summary["route_check"].get(key, 0) for key in ROUTE_CHECKS} if checked else None
    categories = {category: counts.get(category, 0) for category in CATEGORIES}
    without_metal = categories["changed_without_metal"] if checked else None
    # Matched metal-free structures whose warning level or text changed; only
    # logged by passes run since the tally was added.
    without_metal_warnings = (
        comparison["without_metal_warning_only"] + comparison["without_metal_message_only"]
        if "without_metal_warning_only" in comparison
        and "without_metal_message_only" in comparison
        else None
    )
    not_reproduced = sum(categories[category] for category in NOT_REPRODUCED)

    return {
        "total_structures": total,
        "matched": matched,
        "mismatched": mismatched,
        "mismatch_rate": _share(mismatched, total),
        "timeouts": timeouts,
        "comparison": comparison,
        "counts": counts,
        "categories": categories,
        "reconnected_share": _share(categories["reconnected_as_recmet"], mismatched),
        "not_reproduced": not_reproduced,
        "census": element_census(
            [row for row in rows if row["category"] in NOT_REPRODUCED]
        ),
        "examples": _pick_examples(rows),
        "changed_without_metal": without_metal,
        "without_metal_warnings": without_metal_warnings,
        # Classifications and logs from before the breakdown have neither part.
        "prefix": summary.get("prefix"),
        "route": route,
        "gates": {
            # MI only changes how bonds to metals are treated.
            "changed_without_metal": _is_zero(without_metal),
            "without_metal_warnings": _is_zero(without_metal_warnings),
            # The `/r` layer names the route the baseline's own warning names.
            "route": _is_zero(route["disagrees"]) if route else None,
            # Molecular Inorganics sets the `1BB` prefix only on structures it
            # processes, which have a metal: no prefix may change without one,
            # neither on a matched structure nor on a mismatch.
            "prefix": _is_zero(
                comparison["without_metal_prefix_changed"]
                + summary["prefix"]["prefix_changed"].get("changed_without_metal", 0)
            )
            if "without_metal_prefix_changed" in comparison and summary.get("prefix")
            else None,
            "completeness": _completeness(
                matched, mismatched, timeouts, expected_structures
            ),
            "no_aborted": _no_aborted(run_b_log),
        },
        "duration": run_duration(run_b_log) if run_b_log else None,
    }


def build_report_data(
    comparison_dir: Path,
    baseline_label: str,
    test_label: str,
    run_a_log: Path | None = None,
    run_b_log: Path | None = None,
    run_c_log: Path | None = None,
    expected_structures: int | None = None,
) -> dict:
    """The report's data, kept apart by comparison.

    `comparison_dir` is `classify.py`'s output: `summary.json` with one section
    per comparison, and `a_vs_b/` and `a_vs_c/` with their classifications.
    `reference` is Run A on its own, `a_vs_c` the control and `a_vs_b` the
    Molecular Inorganics pass. Nothing is shared between the two comparisons
    except the reference row count they are both checked against."""
    comparison_dir = Path(comparison_dir)
    summary = json.loads((comparison_dir / "summary.json").read_text(encoding="utf-8"))

    def rows(side: str) -> list[dict]:
        path = comparison_dir / side / "classifications.csv"
        return load_classifications(path) if path.is_file() else []

    return {
        "generated": datetime.now().isoformat(timespec="seconds"),
        # run_comparison.sh writes each dataset to a directory named after it.
        "dataset": comparison_dir.name,
        "baseline_label": baseline_label,
        "test_label": test_label,
        "reference": build_reference_data(run_a_log, expected_structures),
        "a_vs_c": build_control_data(
            run_c_log, summary.get("a_vs_c"), rows("a_vs_c"), expected_structures
        ),
        "a_vs_b": build_option_data(
            rows("a_vs_b"), summary.get("a_vs_b", {}), run_b_log, expected_structures
        ),
    }


# What each category means to a reader who does not know the code: a plain name,
# one sentence of explanation, and whether the change is expected. The category's
# own name is shown next to it, so the specialist can still find it in the CSV.
_PLAIN = {
    "reconnected_as_recmet": (
        "Metal bond kept, as expected",
        "The old version cut the bond to the metal. Its optional reconnect setting "
        "(RecMet) restores that bond, and the new version gives exactly that result.",
        "expected",
    ),
    "salt_kept_bonded": (
        "Salt kept together, as expected",
        "The old version split a metal salt into separate ions, which its reconnect "
        "setting cannot undo. The new version keeps the bond as drawn.",
        "expected",
    ),
    "reconnected_differently": (
        "Metal bond kept, but differently",
        "Both versions reconnect the metal, yet the results still differ. "
        "A chemist should look at these.",
        "review",
    ),
    "changed_without_disconnection": (
        "Changed, although nothing was cut",
        "The structure has a metal, but the old version did not cut any bond to it, "
        "and the InChI changed all the same. No known mechanism explains this.",
        "review",
    ),
    "changed_without_metal": (
        "Changed without any metal",
        "Neither version sees a metal, yet the InChI changed. The new handling only "
        "concerns metals, so this should never happen.",
        "unexpected",
    ),
    "error_under_mi": (
        "Only the old version gives an InChI",
        "The new version rejects a structure that the old version accepted.",
        "review",
    ),
    "error_in_reference": (
        "Only the new version gives an InChI",
        "The old version rejected the structure and the new version accepts it, "
        "for example structures with placeholder atoms (<code>*</code>).",
        "neutral",
    ),
    "recmet_failed": (
        "Could not be cross-checked",
        "The old version&rsquo;s reconnect setting produced no InChI to compare with.",
        "neutral",
    ),
    "recmet_missing": (
        "Could not be cross-checked",
        "There is no result from the old version&rsquo;s reconnect setting.",
        "neutral",
    ),
}

_KIND_LABELS = {
    "expected": "Expected",
    "review": "Needs review",
    "unexpected": "Should not happen",
    "neutral": "Other",
}

# Segment colours of the breakdown bar, in display order: expected first.
_CATEGORY_COLOURS = {
    "reconnected_as_recmet": "#2F5D8C",
    "salt_kept_bonded": "#7FA6CF",
    "reconnected_differently": "#D39A2A",
    "changed_without_disconnection": "#8F6400",
    "error_under_mi": "#7A5C99",
    "changed_without_metal": "#A33A3A",
    "error_in_reference": "#8C96A5",
    "recmet_failed": "#C3C9D2",
    "recmet_missing": "#DCE0E6",
}

# Mismatches a reader should know need a closer look.
_NEEDS_REVIEW = tuple(key for key, (_, _, kind) in _PLAIN.items() if kind == "review")


def _status(value) -> str:
    """A check's outcome in words: passed, failed, or not checked."""
    if value is None:
        return '<span class="pill na">Not checked</span>'

    return (
        '<span class="pill pass">Passed</span>'
        if value
        else '<span class="pill fail">Failed</span>'
    )


def _gates(rows: list[tuple]) -> str:
    """Checks as a table: outcome, what is checked in words, and the exact rule."""
    body = "".join(
        f"<tr><td>{_status(value)}</td><td>{plain}"
        f'<span class="tech">{rule}</span></td></tr>'
        for value, plain, rule in rows
    )

    return f"""<div class="scroll"><table class="checks">
      <thead><tr><th>result</th><th>check</th></tr></thead>
      <tbody>{body}</tbody></table></div>"""


def _fig(value: str, label: str, note: str, tone: str = "") -> str:
    return (
        f'<div class="fig {tone}"><div class="n">{value}</div>'
        f'<div class="k">{label}</div><div class="d">{note}</div></div>'
    )


def _one_in(part: int | None, whole: int | None) -> str:
    """'about 1 in 80', the same share as a percentage in words."""
    if not part or not whole:
        return ""

    return f", about 1 in {_fmt(round(whole / part))}"


def _verdict_c(control: dict) -> tuple[str, str]:
    """Tone and caption for the control. Absent means not measured, never zero."""
    categories = control["categories"]
    if categories is None:
        return "is-muted", "Not measured: this report was rendered without A vs C results."
    if categories["unexplained"] == 0:
        return (
            "is-good",
            "Every difference from the baseline has a known cause: the old version "
            "gives the same result with <code>-NPZz</code>, which "
            "<code>-LegacyMetalHandling</code> switches on.",
        )

    return (
        "is-flag",
        "Differences the legacy handling should not make. Until they are explained, "
        "differences under Molecular Inorganics cannot all be attributed to it.",
    )


def _specimen(title: str, row: dict | None, recmet_note: str = "") -> str:
    if not row:
        return ""
    name, why, kind = _PLAIN[title]
    recmet = (
        escape(row["recmet_inchi"][:200])
        if row["recmet_inchi"]
        else '<span class="absent">not computed</span>'
    )
    if recmet_note and not has_reconnected_layer(row["recmet_inchi"]):
        recmet = f'<span class="absent">{escape(recmet_note)}</span>'

    return f"""
  <div class="specimen">
    <div class="cid"><b>{name}</b> <span class="pill {kind}">{_KIND_LABELS[kind]}</span>
      <span class="code">{escape(title)} &middot; ID {escape(row["molfile_id"])}</span></div>
    <p class="why">{why}</p>
    <dl>
      <div class="row"><dt>old version</dt><dd>{escape(row["reference_inchi"][:200])}</dd></div>
      <div class="row"><dt>new version</dt><dd>{escape(row["dev_mi_inchi"][:200])}</dd></div>
      <div class="row"><dt>old version + RecMet</dt><dd>{recmet}</dd></div>
    </dl>
  </div>"""


_MAX_LISTED = 25


def _result_cell(row: dict, side: str) -> str:
    inchi = escape(row[f"{side}_inchi"][:200]) or '<span class="absent">no InChI</span>'
    details = f'exit {escape(str(row[f"{side}_exit"]))}'
    if row[f"{side}_message"]:
        details += f" · {escape(row[f'{side}_message'][:120])}"

    return f'<span class="v">{inchi}</span><span class="v"><b>{details}</b></span>'


def _control_mismatches(control: dict) -> str:
    rows = control["rows"]
    if not rows:
        return ""
    listed = rows[:_MAX_LISTED]
    more = len(rows) - len(listed)
    body = "".join(
        f'<tr><td><span class="v">{escape(row["molfile_id"])}</span>'
        f'<span class="code">{escape(row["sdf"])}</span>'
        f'<span class="code">{escape(row["category"])}</span></td>'
        f'<td>{_result_cell(row, "reference")}</td>'
        f'<td>{_result_cell(row, "legacy")}</td>'
        f'<td>{_result_cell(row, "npzz")}</td></tr>'
        for row in listed
    )
    more_note = (
        f'<p class="tech-intro">{_fmt(more)} more in <code>a_vs_c/classifications.csv</code>.</p>'
        if more
        else ""
    )

    return f"""
  <section class="technical">
    <h3><span class="tag">A vs C</span> The differences, one by one</h3>
    <p class="tech-intro">Unexplained ones first. The last column is the old version run
    again with <code>-NPZz</code>.</p>
    <div class="scroll"><table class="diff">
      <colgroup><col style="width:16%"><col style="width:18%"><col style="width:33%"><col style="width:33%"></colgroup>
      <thead><tr><th>ID</th><th>A &middot; old version</th><th>C &middot; <code>-LegacyMetalHandling</code></th><th>old version <code>-NPZz</code></th></tr></thead>
      <tbody>{body}</tbody></table></div>
    {more_note}
  </section>"""


_ROUTE_LABELS = {
    "agrees": "proxy and message name the same route",
    "disagrees": "proxy and message name different routes",
    "no_disconnection": "the baseline disconnected nothing",
    "no_metal": "no metal, so no route (<code>changed_without_metal</code>)",
    "not_checked": "no usable <code>-RecMet</code> result",
}


def _route_table(route: dict | None) -> str:
    if route is None:
        return (
            '<p class="tech-intro">Route check: not checked. These classifications predate it.</p>'
        )
    body = "".join(
        f'<tr><td class="mono">{escape(key)}</td><td>{_ROUTE_LABELS[key]}</td>'
        f'<td class="n">{_fmt(route[key])}</td></tr>'
        for key in ROUTE_CHECKS
    )

    return f"""<div class="scroll"><table>
      <thead><tr><th>route check</th><th>meaning</th><th class="n">count</th></tr></thead>
      <tbody>{body}</tbody></table></div>"""


_CATEGORY_LABELS = {
    "changed_without_metal": "no metal, yet the InChI changed",
    "reconnected_as_recmet": "MI reproduces what 1.07.5 <code>-RecMet</code> reconnected",
    "reconnected_differently": "both reconnect the metal, with different results",
    "salt_kept_bonded": "1.07.5 disconnected a salt, <code>-RecMet</code> cannot undo it, MI keeps the bond",
    "changed_without_disconnection": "1.07.5 broke no bond to the metal, yet the InChI changed",
    "error_under_mi": "MI produced no InChI, 1.07.5 did",
    "error_in_reference": "1.07.5 produced no InChI, MI did",
    "recmet_failed": "<code>-RecMet</code> produced no InChI",
    "recmet_missing": "no <code>-RecMet</code> result",
}


def _prefix_section(option: dict) -> str:
    """Every outcome of A vs B, split by whether the prefix changed."""
    c = option["comparison"]
    p = option["prefix"]
    unaffected = c.get("unaffected")
    warning_changed = (
        c["prefix_only"] - unaffected
        if unaffected is not None and "prefix_only" in c
        else None
    )

    def side(name: str, category: str):
        return p[name].get(category, 0) if p else None

    rows = [
        ("unaffected", "same InChI apart from the prefix and the key flag",
         unaffected, None),
        ("unaffected_warning_changed", "same InChI, but the warning level or text changed",
         warning_changed, None),
    ] + [
        (category, _CATEGORY_LABELS[category],
         side("prefix_changed", category), side("prefix_unchanged", category))
        for category in CATEGORIES
        if not category.startswith(("error_", "recmet_"))
    ]
    body = "".join(
        f'<tr><td class="mono">{name}</td><td>{label}</td>'
        f'<td class="n">{_fmt(changed)}</td>'
        f'<td class="n">{"" if name.startswith("unaffected") else _fmt(unchanged)}</td></tr>'
        for name, label, changed, unchanged in rows
    )

    return f"""  <section class="technical">
    <h3><span class="tag">A vs B</span> Prefix changes</h3>
    <p class="tech-intro">Each outcome, by whether the prefix differs from Run A&rsquo;s. The two
    <span class="mono">unaffected</span> rows come from Run B&rsquo;s tallies of matched
    structures; the rest are the classified mismatches. A mismatch where either side failed has
    no prefix and is left out.</p>
    <div class="scroll"><table>
      <thead><tr><th>outcome</th><th>meaning</th><th class="n">prefix changed</th><th class="n">prefix unchanged</th></tr></thead>
      <tbody>{body}</tbody></table></div>
  </section>
"""


def _without_metal_callout(changed: int | None, warnings: int | None) -> str:
    """Flag any change to a structure without a metal, which MI cannot explain."""
    parts = []
    if changed:
        parts.append(
            f"<b>{_fmt(changed)} structures without a metal changed their InChI.</b> "
            "Their IDs are in <code>ids/changed_without_metal.txt</code>."
        )
    if warnings:
        parts.append(
            f"<b>{_fmt(warnings)} warning changes on structures without a metal</b> "
            "whose InChI is unchanged (<span class=\"mono\">without_metal_warning_only</span> "
            "plus <span class=\"mono\">without_metal_message_only</span>)."
        )
    if not parts:
        return ""

    return f"""<div class="callout flagged">
      <div class="eyebrow">unexpected</div>
      <p>{" ".join(parts)} Molecular Inorganics only changes how bonds to metals are
      treated, so it has no mechanism to change these.</p>
    </div>"""


def _review_callout(categories: dict) -> str:
    count = sum(categories[category] for category in _NEEDS_REVIEW)
    if not count:
        return ""
    listed = ", ".join(
        f"{_fmt(categories[category])} &ldquo;{_PLAIN[category][0].lower()}&rdquo;"
        for category in _NEEDS_REVIEW
        if categories[category]
    )

    return f"""<div class="callout review">
      <div class="eyebrow">needs chemical review</div>
      <p><b>{_fmt(count)} changed structures have no explanation yet</b>: {listed}.
      The IDs are in <code>a_vs_b/ids/</code>, one file per category.</p>
    </div>"""


def _breakdown(option: dict) -> str:
    """The mismatches as one stacked bar and a legend in plain words."""
    k = option["categories"]
    mismatched = option["mismatched"]
    present = [category for category in _CATEGORY_COLOURS if k.get(category)]
    whole = sum(k[category] for category in present)
    bar = "".join(
        f'<span style="width:{k[category] / whole * 100:.3f}%;'
        f'background:{_CATEGORY_COLOURS[category]}"></span>'
        for category in present
    ) if whole else ""
    rows = "".join(
        f'<tr><td><span class="swatch" style="background:{_CATEGORY_COLOURS[category]}"></span>'
        f'<b>{_PLAIN[category][0]}</b><span class="code">{category}</span></td>'
        f'<td>{_PLAIN[category][1]}</td>'
        f'<td><span class="pill {_PLAIN[category][2]}">{_KIND_LABELS[_PLAIN[category][2]]}</span></td>'
        f'<td class="n">{_fmt(k[category])}</td>'
        f'<td class="n">{_pct(_share(k[category], mismatched), 1)}</td></tr>'
        for category in _CATEGORY_COLOURS
    )

    return f"""
    <div class="stack" role="img" aria-label="Changed structures by kind of change">{bar}</div>
    <div class="scroll"><table class="breakdown">
      <colgroup><col style="width:27%"><col style="width:37%"><col style="width:15%"><col style="width:12%"><col style="width:9%"></colgroup>
      <thead><tr><th>kind of change</th><th>what it means</th><th>assessment</th>
      <th class="n">structures</th><th class="n">share</th></tr></thead>
      <tbody>{rows}</tbody></table></div>"""


def _render_reference(data: dict) -> str:
    ref = data["reference"]

    return f"""
<section class="part part-a" id="reference">
  <div class="part-head">
    <span class="badge">Run A &middot; the reference</span>
    <h2>The old version, as released</h2>
    <p class="question">{escape(data["baseline_label"])} with no options. Its results are what
    the two comparisons below measure against.</p>
  </div>
  <div class="figs">
    {_fig(_fmt(ref["rows"]), "structures", "Stored as the reference, counted independently of either comparison.")}
    {_fig(_fmt(ref["timeouts"]), "too slow", "Took longer than the time limit; stored as a timeout instead of an InChI.")}
    {_fig(_duration(ref["duration"]), "run time", "Wall-clock time of Run A.")}
  </div>
  {_gates([(ref["no_aborted"], "All input files were read to the end", "no aborted SDFs &middot; Run A")])}
</section>"""


def _render_control(data: dict) -> str:
    control = data["a_vs_c"]
    tone, caption = _verdict_c(control)
    gates = control["gates"]
    categories = control["categories"] or {}
    cross_check = control["cross_check"] or {}

    return f"""
<section class="part part-c" id="a-vs-c">
  <div class="part-head">
    <span class="badge">A vs C &middot; control</span>
    <h2>Switched off, does the new version give the old results?</h2>
    <p class="question">The new version can still treat metals the old way with the option
    <code>-LegacyMetalHandling</code>. Run C uses it, so it should reproduce Run A on every
    structure. Only the label at the start of each InChI may differ (<code>InChI=1N/</code>
    instead of <code>InChI=1S/</code>), because that label now says which handling was used.</p>
  </div>

  <section>
    <h3><span class="tag">A vs C</span> Answer</h3>
    <div class="figs">
      {_fig(_fmt(categories.get("unexplained")), "unexplained differences", caption, tone)}
      {_fig(_fmt(categories.get("explained_by_npzz")), "differences with a known cause", "Structures with placeholder atoms (<code>*</code>). The option also allows these, and the old version gives the same result once told to allow them (<code>-NPZz</code>).")}
      {_fig(_fmt(control["matched"]), "identical", "Same result as the old version, apart from the label at the start.", "is-good")}
      {_fig(_fmt(control["timeouts"]), "too slow", "Never compared. Expected ones are listed in the data config.")}
    </div>
  </section>

  <section>
    <h3><span class="tag">A vs C</span> Checks</h3>
    {_gates([
        (gates["explained"], "Every difference has a known cause", "unexplained = 0"),
        (gates["cross_check"], "Each of these differences also shows up with the new handling on", f"also mismatched in Run B: {_fmt(cross_check.get('also_in_a_vs_b'))}, only here: {_fmt(cross_check.get('only_in_a_vs_c'))} &middot; #259 cross-check"),
        (gates["completeness"], "Every structure was compared", "same + mismatched + timed out = reference rows"),
        (gates["no_aborted"], "All input files were read to the end", "no aborted SDFs &middot; Run C"),
    ])}
  </section>

  <div class="tech-divider">technical details</div>

  <section class="technical">
    <h3><span class="tag">A vs C</span> How it is compared</h3>
    <p>Run C: {escape(data["test_label"])} with <code>-LegacyMetalHandling</code>
    (column 3 of #259). Compared with Run A on every stored field (<code>inchi</code>,
    <code>key</code>, <code>exit</code>, <code>message</code>) except the version prefix and the
    two InChIKey characters that mirror it: the legacy handling produces non-standard
    <code>InChI=1N/</code>. It also implies <code>NPZz</code>, which Run A does not use, so each
    mismatch is re-computed with the baseline and <code>-NPZz</code>.</p>
  </section>
{_control_mismatches(control)}
  <section class="technical">
    <h3><span class="tag">A vs C</span> What this cannot see</h3>
    <p>AuxInfo and the full log are not stored, so a change in either is invisible
    here. A control without unexplained differences shows the same InChI body, InChIKey
    hash, return code and warning message as the baseline on every structure, not
    identical output.</p>
  </section>

  <section class="technical">
    <h3><span class="tag">A vs C</span> Cost</h3>
    <p>Run C took {_duration(control["duration"])}.</p>
  </section>
</section>"""


def _render_option(data: dict) -> str:
    option = data["a_vs_b"]
    c = option["comparison"]
    k = option["categories"]
    gates = option["gates"]
    census = "".join(
        f'<span class="el"><b>{escape(el)}</b><span>{_fmt(count)}</span></span>'
        for el, count in option["census"].items()
    )
    expected = k["reconnected_as_recmet"] + k["salt_kept_bonded"]

    return f"""
<section class="part part-b" id="a-vs-b">
  <div class="part-head">
    <span class="badge">A vs B &middot; Molecular Inorganics</span>
    <h2>What does the new metal handling change?</h2>
    <p class="question">Run B is the new version as it will be used: Molecular Inorganics is
    on by default. Its InChI is compared with the old version&rsquo;s, leaving out the label
    at the start, which is meant to change: <code>InChI=1BB/</code> marks a structure the new
    handling processed, every other structure keeps <code>InChI=1S/</code>.</p>
  </div>

  <section>
    <h3><span class="tag">A vs B</span> Answer</h3>
    <div class="figs">
      {_fig(_pct(option["mismatch_rate"]), "changed", f"{_fmt(option['mismatched'])} of {_fmt(option['total_structures'])} structures get a different InChI{_one_in(option['mismatched'], option['total_structures'])}.", "is-mi")}
      {_fig(_pct(_share(expected, option["mismatched"]), 1), "of the changes are expected", f"{_fmt(expected)} structures: the metal or salt bond is kept, as intended.", "is-good")}
      {_fig(_fmt(sum(k[category] for category in _NEEDS_REVIEW)), "need review", "Changes without a known explanation, listed below.", "is-flag" if any(k[category] for category in _NEEDS_REVIEW) else "is-good")}
      {_fig(_fmt(option["timeouts"]), "too slow", "Never compared. Expected ones are listed in the data config.")}
    </div>
  </section>

  <section>
    <h3><span class="tag">A vs B</span> Kinds of change</h3>
    <p>Every changed structure is sorted into one kind, by comparing it with what the old
    version did to it. The old version cut bonds to metals in two ways and warned about it
    (&ldquo;Metal was disconnected&rdquo; or &ldquo;Salt was disconnected&rdquo;); its optional
    reconnect setting, RecMet, can undo only the first.</p>
    {_breakdown(option)}
{_without_metal_callout(option["changed_without_metal"], option["without_metal_warnings"])}
{_review_callout(k)}
  </section>

  <section>
    <h3><span class="tag">A vs B</span> Examples</h3>
    <p>The first structure of each kind, as InChI strings. Read the strings layer by layer:
    the part after the first <code>/</code> is the formula, and a dot separates disconnected
    pieces.</p>
{_specimen("reconnected_as_recmet", option["examples"]["reconnected_as_recmet"])}
{_specimen("salt_kept_bonded", option["examples"]["salt_kept_bonded"], "no /r layer — salt disconnection is not reversible by RecMet")}
{_specimen("reconnected_differently", option["examples"]["reconnected_differently"])}
{_specimen("changed_without_disconnection", option["examples"]["changed_without_disconnection"], "no /r layer — 1.07.5 disconnected nothing")}
  </section>

  <section>
    <h3><span class="tag">A vs B</span> Elements involved</h3>
    <p>Elements other than carbon and hydrogen in the changes the reconnect setting does not
    reproduce (&ldquo;reconnected differently&rdquo;, &ldquo;salt kept together&rdquo; and
    &ldquo;changed, although nothing was cut&rdquo;), with the number of structures:</p>
    <div class="census">{census or '<span class="absent">none</span>'}</div>
  </section>

  <section>
    <h3><span class="tag">A vs B</span> Checks</h3>
    {_gates([
        (gates["changed_without_metal"], "No structure without a metal changed its InChI", "changed_without_metal = 0"),
        (gates["without_metal_warnings"], "No structure without a metal changed its warnings", "warning changes without a metal = 0"),
        (gates["route"], "The old version&rsquo;s warning agrees with what RecMet shows", "<span class=\"mono\">/r</span> proxy disagrees with the baseline message = 0"),
        (gates["prefix"], "Only structures with a metal get a new label", "prefix changes without a metal = 0"),
        (gates["completeness"], "Every structure was compared", "matched + mismatched + timed out = reference rows"),
        (gates["no_aborted"], "All input files were read to the end", "no aborted SDFs &middot; Run B"),
    ])}
  </section>

  <div class="tech-divider">technical details</div>

  <section class="technical">
    <h3><span class="tag">A vs B</span> Tallies</h3>
    <p>Run B: {escape(data["test_label"])} without options (column 2 of #259). Compared with
    Run A on the InChI body and the failure state only. The prefix, InChIKey, warning level and
    message are counted, not compared.</p>
    <div class="scroll"><table>
      <thead><tr><th>tally</th><th class="n">count</th></tr></thead>
      <tbody>
        <tr><td>matched</td><td class="n">{_fmt(c.get("matched"))}</td></tr>
        <tr><td>mismatched</td><td class="n">{_fmt(c.get("mismatched"))}</td></tr>
        <tr><td>prefix_only</td><td class="n">{_fmt(c.get("prefix_only"))}</td></tr>
        <tr><td>key_only</td><td class="n">{_fmt(c.get("key_only"))}</td></tr>
        <tr><td>warning_only</td><td class="n">{_fmt(c.get("warning_only"))}</td></tr>
        <tr><td>message_only</td><td class="n">{_fmt(c.get("message_only"))}</td></tr>
        <tr><td>unaffected</td><td class="n">{_fmt(c.get("unaffected"))}</td></tr>
        <tr><td>without_metal_prefix_changed</td><td class="n">{_fmt(c.get("without_metal_prefix_changed"))}</td></tr>
        <tr><td>without_metal_warning_only</td><td class="n">{_fmt(c.get("without_metal_warning_only"))}</td></tr>
        <tr><td>without_metal_message_only</td><td class="n">{_fmt(c.get("without_metal_message_only"))}</td></tr>
        <tr><td>both_failed</td><td class="n">{_fmt(c.get("both_failed"))}</td></tr>
        <tr><td>failure_kind_only</td><td class="n">{_fmt(c.get("failure_kind_only"))}</td></tr>
      </tbody></table></div>
  </section>

{_prefix_section(option)}
  <section class="technical">
    <h3><span class="tag">A vs B</span> What the categories mean</h3>
    <p><b>Molecular Inorganics does not create bonds. It declines to break the ones the molfile
    already has.</b> The old code breaks them by two different routes, and <code>-RecMet</code>
    only reverses one. The route is the one 1.07.5 states in its own Run A warning,
    &ldquo;Metal was disconnected&rdquo; or &ldquo;Salt was disconnected&rdquo;:</p>
    <div class="scroll"><table>
      <thead><tr><th>category</th><th>route taken by 1.07.5</th><th class="n">count</th><th>RecMet</th></tr></thead>
      <tbody>
        <tr><td class="mono">reconnected_as_recmet</td><td>metal disconnection; MI equals the restored bonds</td>
          <td class="n">{_fmt(k["reconnected_as_recmet"])}</td><td>reverses it, emits <span class="mono">/r</span></td></tr>
        <tr><td class="mono">reconnected_differently</td><td>metal disconnection; the two disagree</td>
          <td class="n">{_fmt(k["reconnected_differently"])}</td><td>reverses it, differently</td></tr>
        <tr><td class="mono">salt_kept_bonded</td><td>salt disconnection</td>
          <td class="n">{_fmt(k["salt_kept_bonded"])}</td><td>cannot undo it, no <span class="mono">/r</span></td></tr>
        <tr><td class="mono">changed_without_disconnection</td><td>none; 1.07.5 broke no bond to the metal</td>
          <td class="n">{_fmt(k["changed_without_disconnection"])}</td><td>nothing to reverse</td></tr>
      </tbody></table></div>
    <p>The <span class="mono">/r</span> layer is the structural evidence for the route, and it is
    checked against the stated route per structure: &ldquo;Metal was disconnected&rdquo; should
    come with an <span class="mono">/r</span> layer, &ldquo;Salt was disconnected&rdquo; alone
    without one.</p>
{_route_table(option["route"])}
  </section>

  <section class="technical">
    <h3><span class="tag">A vs B</span> What this cannot see</h3>
    <p>The comparison ignores the prefix and the InChIKey flags, so their changes are only
    counted: {_fmt(c.get("prefix_only"))} matched structures changed their prefix. A mismatch
    count means &ldquo;the InChI body differs&rdquo;, never &ldquo;the output is
    unchanged&rdquo;. MI failures on structures the
    baseline also rejected are invisible (<span class="mono">both_failed</span>:
    {_fmt(c.get("both_failed"))}). On a matched structure a changed warning level is
    counted as <span class="mono">warning_only</span> ({_fmt(c.get("warning_only"))}) and a
    changed warning text as <span class="mono">message_only</span>
    ({_fmt(c.get("message_only"))}); neither fails the comparison.
    <span class="mono">aux</span> and <span class="mono">log</span> are not stored at all.
    Where both runs failed, a differing error code is a
    failure <em>kind</em> rather than a warning flip and is counted separately
    (<span class="mono">failure_kind_only</span>: {_fmt(c.get("failure_kind_only"))}).</p>
  </section>

  <section class="technical">
    <h3><span class="tag">A vs B</span> Cost</h3>
    <p>Run B took {_duration(option["duration"])}.</p>
  </section>
</section>"""


_GLOSSARY = (
    ("InChI", "A text code that identifies a chemical structure, like a barcode for "
     "molecules. Two records with the same InChI describe the same compound."),
    ("InChIKey", "A short, fixed-length fingerprint of the InChI, for searching the web "
     "and databases."),
    ("label (prefix)", "The start of an InChI. <code>InChI=1S/</code> is standard, "
     "<code>InChI=1BB/</code> marks a structure the new metal handling processed, "
     "<code>InChI=1N/</code> one computed with the old handling switched back on."),
    ("Molecular Inorganics", "The new treatment of metals: bonds to metals are kept as "
     "drawn, instead of being cut before the InChI is computed."),
    ("metal disconnection", "What the old version does to many bonds between a metal and "
     "the rest of a molecule: it cuts them and describes the pieces separately."),
    ("salt disconnection", "The same for metal salts: the old version splits them into "
     "separate ions."),
    ("RecMet", "An option of the old version that reconnects cut metal bonds and adds the "
     "result as an extra layer (<code>/r</code>). It cannot undo salt disconnection."),
    ("<code>-LegacyMetalHandling</code>", "An option of the new version that switches the "
     "old metal treatment back on."),
    ("placeholder atom", "An atom drawn as <code>*</code>, standing for an unspecified "
     "group. The old version rejects it unless run with <code>-NPZz</code>."),
    ("too slow (timed out)", "A structure that took longer than the time limit of 60 "
     "seconds. It is recorded, not compared."),
    ("aborted file", "An input file whose processing stopped part-way. Its remaining "
     "structures were never compared."),
    ("PubChem", "A public database of chemical structures run by the US National Library "
     "of Medicine; the structures in this report come from it."),
)


def _render_glossary() -> str:
    terms = "".join(
        f"<div><dt>{term}</dt><dd>{meaning}</dd></div>" for term, meaning in _GLOSSARY
    )

    return f"""
<section class="part part-g" id="glossary">
  <div class="part-head"><h2>Words used in this report</h2></div>
  <dl class="glossary">{terms}</dl>
</section>"""


def _all_gates(data: dict) -> list[bool | None]:
    return (
        [data["reference"]["no_aborted"]]
        + list(data["a_vs_c"]["gates"].values())
        + list(data["a_vs_b"]["gates"].values())
    )


def _in_short(data: dict) -> str:
    """The findings in a few sentences, for a reader who stops here."""
    control = data["a_vs_c"]
    option = data["a_vs_b"]
    k = option["categories"]
    gates = _all_gates(data)
    failed = gates.count(False)
    unchecked = gates.count(None)
    tone = "flag" if failed else ("na" if unchecked == len(gates) else "good")

    points = []
    if option["mismatched"] is not None:
        expected = k["reconnected_as_recmet"] + k["salt_kept_bonded"]
        points.append(
            f"With the new metal handling, <b>{_fmt(option['mismatched'])} of "
            f"{_fmt(option['total_structures'])} structures ({_pct(option['mismatch_rate'])}) "
            f"get a different InChI</b>{_one_in(option['mismatched'], option['total_structures'])}."
        )
        points.append(
            f"<b>{_pct(_share(expected, option['mismatched']), 1)} of these changes are "
            f"expected</b>: the bond to the metal or within the salt is kept, as intended."
        )
    review = sum(k[category] for category in _NEEDS_REVIEW)
    if review:
        points.append(f"<b>{_fmt(review)} changed structures need a chemist&rsquo;s review</b>; "
                      "there is no known explanation for them yet.")
    without_metal = option["changed_without_metal"]
    if without_metal == 0:
        points.append("No structure without a metal changed, as it should be.")
    elif without_metal:
        points.append(f"<b>{_fmt(without_metal)} structures without a metal changed.</b> "
                      "That should not happen and needs investigating.")
    unexplained = (control["categories"] or {}).get("unexplained")
    if unexplained == 0:
        points.append("With the new handling switched off, the new version gives the old "
                      "results: no unexplained differences.")
    elif unexplained:
        points.append(f"With the new handling switched off, <b>{_fmt(unexplained)} "
                      "differences from the old version remain unexplained.</b>")
    points.append(
        f"{len(gates) - failed - unchecked} of {len(gates)} automatic checks passed"
        + (f", <b>{failed} failed</b>" if failed else "")
        + (f", {unchecked} could not be checked" if unchecked else "")
        + "."
    )
    items = "".join(f"<li>{point}</li>" for point in points)

    return f"""
<section class="box {tone}">
  <h2>In short</h2>
  <ul>{items}</ul>
</section>"""


def render_html(data: dict) -> str:
    control = data["a_vs_c"]
    option = data["a_vs_b"]
    c_tone, c_caption = _verdict_c(control)
    baseline = escape(data["baseline_label"])
    test = escape(data["test_label"])
    dataset = escape(data["dataset"])

    return f"""<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<meta name="color-scheme" content="light">
<title>MolecularInorganics Comparison</title>
<link rel="stylesheet" href="https://fonts.googleapis.com/css2?family=IBM+Plex+Mono:wght@400;500&family=IBM+Plex+Sans:wght@400;600&family=IBM+Plex+Serif:wght@600&display=swap">
<style>
{_CSS}
</style>
</head>
<body>
<div class="wrap">
<header>
  <div class="eyebrow">IUPAC InChI &middot; comparison report</div>
  <h1>How the new metal handling changes InChI</h1>
  <p class="lede">The new InChI version keeps bonds to metals that older versions cut.
  This report compares it with the released {baseline} on the PubChem
  <b>{dataset}</b> structures: what changes, and whether anything changes that
  should not.</p>
  <div class="meta">
    <span>old version <b>{baseline}</b></span>
    <span>new version <b>{test}</b></span>
    <span>data <b>PubChem {dataset}</b></span>
    <span>generated {escape(data["generated"])}</span>
  </div>
</header>
{_in_short(data)}
<section>
  <h2>What was tested</h2>
  <p>An <b>InChI</b> is a text code that identifies a chemical structure; databases use it
  to tell whether two records describe the same compound. Up to {baseline}, InChI cut many
  bonds between a metal and the rest of the molecule before computing the code, so a metal
  complex was described as separate pieces. The new version keeps those bonds
  (<b>Molecular Inorganics</b>), so the code describes the structure as drawn. That should
  change the InChI of many structures with a metal, and nothing else.</p>
  <p>Every structure was run three times:</p>
  <div class="runs">
    <div class="run run-a"><span class="badge">A</span><b>Old version</b>
      <span>{baseline}, as released. The reference.</span></div>
    <div class="run run-b"><span class="badge">B</span><b>New version</b>
      <span>{test}, metal bonds kept (the default). Shows what changes.</span></div>
    <div class="run run-c"><span class="badge">C</span><b>New version, old handling</b>
      <span>{test} with <code>-LegacyMetalHandling</code>. Should match A.</span></div>
  </div>
</section>

<section class="summary">
  <a class="card part-c" href="#a-vs-c">
    <span class="badge">A vs C &middot; control</span>
    <div class="n {c_tone}">{_fmt((control["categories"] or {}).get("unexplained"))}</div>
    <div class="d">unexplained differences. {c_caption}</div>
  </a>
  <a class="card part-b" href="#a-vs-b">
    <span class="badge">A vs B &middot; Molecular Inorganics</span>
    <div class="n">{_pct(option["mismatch_rate"])}</div>
    <div class="d">of structures change their InChI
    ({_fmt(option["mismatched"])}); {_pct(option["reconnected_share"], 1)} of those match what
    the old version&rsquo;s RecMet setting reconnects (<span class="code">reconnected_as_recmet</span>).</div>
  </a>
</section>
{_render_reference(data)}
{_render_control(data)}
{_render_option(data)}
{_render_glossary()}

<footer>
  <p>Generated by <code>INCHI-1-TEST/comparisons/molecular_inorganics/report.py</code> from
  <code>classifications.csv</code>, <code>summary.json</code> and the three run logs.</p>
  <p>Context: issue #259 proposes making Molecular Inorganics the standard path with opt-out
  parameters for the legacy metal handling.</p>
</footer>
</div>
</body>
</html>
"""


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Render an HTML report from the comparison output."
    )
    parser.add_argument(
        "--comparison-dir",
        required=True,
        type=Path,
        help="classify.py's output: summary.json, a_vs_b/ and a_vs_c/.",
    )
    parser.add_argument("--baseline-label", default="v1.07.5")
    parser.add_argument("--test-label", default="dev")
    parser.add_argument("--run-a-log", type=Path)
    parser.add_argument("--run-b-log", type=Path)
    parser.add_argument("--run-c-log", type=Path)
    parser.add_argument(
        "--expected-structures",
        type=int,
        help=(
            "How many structures the run should have covered, counted independently "
            "of the comparison -- run_comparison.sh passes the reference row count. "
            "Without it the completeness gates read 'not checked' rather than "
            "checking a total against itself."
        ),
    )
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()

    data = build_report_data(
        comparison_dir=args.comparison_dir,
        baseline_label=args.baseline_label,
        test_label=args.test_label,
        run_a_log=args.run_a_log,
        run_b_log=args.run_b_log,
        run_c_log=args.run_c_log,
        expected_structures=args.expected_structures,
    )

    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(render_html(data), encoding="utf-8")
    print(f"Wrote {args.output} ({args.output.stat().st_size:,} bytes).")


if __name__ == "__main__":
    main()
