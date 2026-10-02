"""Render the comparison report from the output of `classify.py`.

Turns `classifications.csv`, `summary.json` and the three run logs into a
self-contained HTML page, so every run of the comparison produces the same report
without hand-assembly.

The page keeps the two comparisons strictly apart. Run A, the reference, has a
block of its own; A vs C (the control, byte-for-byte) and A vs B (the option,
prefix-insensitive) each get a part with their own figures, gates, blind spots
and cost, built only from that pass's data.

The one piece of interpretation baked in here is the pathway split, and it is
derived from data rather than assumed. The old code breaks bonds to metals by two
different routes and `-RecMet` only reverses one of them:

  * metal disconnection  -- `-RecMet` reverses it and emits an `/r` layer, so a
    structure `-MolecularInorganics` left alone matches that layer exactly.
  * salt disconnection   -- `-RecMet` has no mechanism to undo it, so there is no
    `/r` layer and nothing for the MI output to be compared against.

`-MolecularInorganics` does not create bonds; it declines to break the ones the
molfile already has. The presence of an `/r` layer in the `-RecMet` output is
therefore a proxy for which route the old code took, and on the pilot corpus it
separated the two perfectly (3875/3875 metal, 671/671 salt, 164/164 metal).
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
    ROUTE_CHECKS,
    Mismatch,
    has_reconnected_layer,
    parse_comparison_summary,
    parse_regression_log,
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


def novel_split(rows: list[dict]) -> dict[str, int]:
    """Split the `novel` rows by which route the old code used to break the bond."""
    novel = [row for row in rows if row["category"] == "novel"]

    metal = sum(has_reconnected_layer(row["recmet_inchi"]) for row in novel)

    return {
        "total": len(novel),
        "metal_pathway": metal,
        "salt_pathway": len(novel) - metal,
    }


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


_CSS = """
:root{
  --bg:#F6F7F9; --surface:#FFFFFF; --surface-2:#EFF2F6;
  --ink:#171B22; --muted:#616B7C; --line:#DDE2E9; --line-soft:#E9EDF2;
  --base:#2F5D8C; --mi:#9A5B2B; --mi-soft:#F4E9DE;
  --good:#2E7D5B; --ctl:#2E6F73; --flag:#8A3F3F;
  --serif:"IBM Plex Serif",Georgia,serif;
  --sans:"IBM Plex Sans",-apple-system,BlinkMacSystemFont,"Segoe UI",sans-serif;
  --mono:"IBM Plex Mono",ui-monospace,Menlo,monospace;
}
@media (prefers-color-scheme: dark){
  :root:not([data-theme="light"]){
    --bg:#101318; --surface:#171B22; --surface-2:#1E232C;
    --ink:#E7EAF0; --muted:#98A2B3; --line:#2A313B; --line-soft:#222832;
    --base:#87AEDA; --mi:#D9955C; --mi-soft:#2C2218;
    --good:#66BE93; --ctl:#6CC0C4; --flag:#D98C8C;
  }
}
:root[data-theme="dark"]{
  --bg:#101318; --surface:#171B22; --surface-2:#1E232C;
  --ink:#E7EAF0; --muted:#98A2B3; --line:#2A313B; --line-soft:#222832;
  --base:#87AEDA; --mi:#D9955C; --mi-soft:#2C2218;
  --good:#66BE93; --ctl:#6CC0C4; --flag:#D98C8C;
}
*{box-sizing:border-box}
body{background:var(--bg);color:var(--ink);font-family:var(--sans);font-size:16px;
  line-height:1.65;padding-inline:20px;padding-block:0}
.wrap{max-width:820px;margin:0 auto;padding-block:56px 80px;display:flex;
  flex-direction:column;gap:42px}
h1,h2{font-family:var(--serif);text-wrap:balance;margin:0;line-height:1.25}
h1{font-size:clamp(30px,5vw,40px);font-weight:600;letter-spacing:-.01em}
h2{font-size:21px;font-weight:600;display:flex;align-items:baseline;gap:12px}
h2 .num{font-family:var(--mono);font-size:12px;color:var(--muted);font-weight:500;letter-spacing:.08em}
p{margin:0} section{display:flex;flex-direction:column;gap:14px}
code,.mono{font-family:var(--mono);font-size:.88em}
code{background:var(--surface-2);padding:1px 5px;border-radius:3px}
.eyebrow{font-family:var(--mono);font-size:11px;letter-spacing:.14em;
  text-transform:uppercase;color:var(--muted)}
header{display:flex;flex-direction:column;gap:16px;border-bottom:2px solid var(--ink);
  padding-bottom:24px}
.lede{font-size:18px;color:var(--muted);max-width:62ch}
.meta{display:flex;flex-wrap:wrap;gap:6px 26px;font-family:var(--mono);font-size:12.5px;color:var(--muted)}
.meta b{color:var(--ink);font-weight:500}
.figs{display:grid;grid-template-columns:repeat(auto-fit,minmax(200px,1fr));gap:1px;
  background:var(--line);border:1px solid var(--line)}
.fig{background:var(--surface);padding:20px;display:flex;flex-direction:column;gap:5px}
.fig .n{font-family:var(--serif);font-size:36px;font-weight:600;line-height:1;
  font-variant-numeric:tabular-nums;letter-spacing:-.02em}
.fig .k{font-family:var(--mono);font-size:11px;letter-spacing:.1em;
  text-transform:uppercase;color:var(--muted)}
.fig .d{font-size:13.5px;color:var(--muted);line-height:1.5}
.fig.is-good .n{color:var(--good)} .fig.is-mi .n{color:var(--mi)} .fig.is-base .n{color:var(--base)}
.fig.is-flag .n{color:var(--flag)} .fig.is-muted .n{color:var(--muted)}
.cascade{display:flex;flex-direction:column;gap:9px;background:var(--surface);
  border:1px solid var(--line);padding:20px}
.bar-row{display:grid;grid-template-columns:minmax(120px,176px) 1fr auto;gap:14px;align-items:center}
.bar-row .lab{font-family:var(--mono);font-size:12px;color:var(--muted);text-align:right}
.bar-track{background:var(--surface-2);height:16px;overflow:hidden}
.bar-fill{height:100%}
.bar-row .val{font-family:var(--mono);font-size:12.5px;font-variant-numeric:tabular-nums;
  min-width:84px;text-align:right}
.scroll{overflow-x:auto;border:1px solid var(--line);background:var(--surface)}
table{border-collapse:collapse;width:100%;font-size:14px}
th,td{padding:9px 14px;text-align:left;border-bottom:1px solid var(--line-soft);white-space:nowrap}
thead th{background:var(--surface-2);font-family:var(--mono);font-size:11px;
  letter-spacing:.07em;text-transform:uppercase;color:var(--muted);font-weight:500}
tbody tr:last-child td{border-bottom:none}
td.n,th.n{text-align:right;font-family:var(--mono);font-variant-numeric:tabular-nums}
.specimen{background:var(--surface);border:1px solid var(--line);border-left:3px solid var(--mi)}
.specimen .cid{font-family:var(--mono);font-size:11.5px;letter-spacing:.06em;color:var(--muted);
  padding:11px 16px 9px;border-bottom:1px solid var(--line-soft)}
.specimen dl{margin:0;display:flex;flex-direction:column}
.specimen .row{display:grid;grid-template-columns:88px 1fr;gap:12px;padding:8px 16px;align-items:baseline}
.specimen .row+.row{border-top:1px solid var(--line-soft)}
.specimen dt{font-family:var(--mono);font-size:11px;letter-spacing:.05em;
  text-transform:uppercase;color:var(--muted)}
.specimen dd{margin:0;font-family:var(--mono);font-size:12.5px;overflow-x:auto;white-space:nowrap}
.absent{color:var(--muted);font-style:italic;font-family:var(--sans);font-size:13px}
.gates{display:flex;flex-direction:column;gap:1px;background:var(--line);border:1px solid var(--line)}
.gate{background:var(--surface);padding:11px 15px;display:flex;flex-wrap:wrap;gap:4px 12px;
  align-items:baseline;font-family:var(--mono);font-size:12.5px}
.tick{color:var(--good);font-weight:600} .cross{color:var(--flag);font-weight:600}
.na{color:var(--muted)} .gate .what{color:var(--muted)}
.census{display:flex;flex-wrap:wrap;gap:6px}
.el{display:inline-flex;align-items:baseline;gap:5px;border:1px solid var(--line);
  background:var(--surface);padding:4px 9px;font-family:var(--mono);font-size:12.5px}
.el b{font-weight:500} .el span{color:var(--muted);font-size:11.5px}
.callout{background:var(--surface);border:1px solid var(--line);border-left:3px solid var(--base);
  padding:16px 18px;display:flex;flex-direction:column;gap:8px;font-size:14.5px}
.callout.clean{border-left-color:var(--good)}
.callout.flagged{border-left-color:var(--flag)}
footer{border-top:1px solid var(--line);padding-top:20px;font-size:13px;color:var(--muted);
  display:flex;flex-direction:column;gap:6px}
@media (max-width:560px){
  .bar-row{grid-template-columns:1fr auto;gap:6px 10px}
  .bar-row .lab{text-align:left} .bar-track{grid-column:1/-1}
  .specimen .row{grid-template-columns:1fr;gap:3px}
}
"""


def _pick_examples(rows: list[dict]) -> dict[str, dict | None]:
    """One representative row per mechanism, chosen deterministically."""
    examples: dict[str, dict | None] = {
        "equivalent": None,
        "salt_pathway": None,
        "metal_pathway": None,
    }

    for row in rows:
        if row["category"] == "recmet_equivalent" and not examples["equivalent"]:
            examples["equivalent"] = row
        elif row["category"] == "novel":
            key = (
                "metal_pathway"
                if has_reconnected_layer(row["recmet_inchi"])
                else "salt_pathway"
            )
            if not examples[key]:
                examples[key] = row
        if all(examples.values()):
            break

    return examples


def _fmt(n) -> str:
    return f"{n:,}" if isinstance(n, int) else ("—" if n is None else f"{n:.2f}")


def _pct(value: float | None, places: int = 2) -> str:
    """A percentage, or an em dash where there is no measurement to render."""
    return "—" if value is None else f"{value:.{places}f}%"


def _bar(value: float | None) -> float:
    """A bar width. An unmeasured quantity draws nothing rather than guessing."""
    return 0.0 if value is None else value


def _duration(seconds: float | None) -> str:
    if seconds is None:
        return "—"
    minutes, secs = divmod(int(seconds), 60)

    return f"{minutes} min {secs:02d} s" if minutes else f"{secs} s"


def _tick(value) -> str:
    if value is None:
        return '<span class="na">not checked</span>'

    return (
        '<span class="tick">&#10003;</span>'
        if value
        else '<span class="cross">&#10007;</span>'
    )


def _specimen(title: str, row: dict | None, recmet_note: str = "") -> str:
    if not row:
        return ""
    recmet = (
        escape(row["recmet_inchi"][:200])
        if row["recmet_inchi"]
        else '<span class="absent">not computed</span>'
    )
    if recmet_note and not has_reconnected_layer(row["recmet_inchi"]):
        recmet = f'<span class="absent">{escape(recmet_note)}</span>'

    return f"""
  <div class="specimen">
    <div class="cid">{escape(title)} &middot; ID {escape(row["molfile_id"])}</div>
    <dl>
      <div class="row"><dt>baseline</dt><dd>{escape(row["reference_inchi"][:200])}</dd></div>
      <div class="row"><dt>MI</dt><dd>{escape(row["dev_mi_inchi"][:200])}</dd></div>
      <div class="row"><dt>RecMet</dt><dd>{recmet}</dd></div>
    </dl>
  </div>"""


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
    """Whether no shard aborted in this one pass, or `None` if its log is unreadable."""
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

    Against the reference row count, which is arrived at by counting rows in the
    reference databases -- a different route to the number than the comparator's
    tallies. Checking `matched + mismatched` against a total *defined* as their
    sum, as this once did, is a tautology that renders green on exactly the
    short-total runs it is meant to catch. A molfile that timed out is never handed
    to the comparator, so without the timeout count from the pass's own log the
    gate cannot be checked."""
    if None in (matched, mismatched, timeouts, expected_structures):
        return None

    return matched + mismatched + timeouts == expected_structures


def differing_fields(mismatch: Mismatch) -> list[str]:
    """The result fields that differ between a pass and the reference."""
    fields = set(mismatch.current) | set(mismatch.reference)

    return sorted(
        field
        for field in fields
        if mismatch.current.get(field) != mismatch.reference.get(field)
    )


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
    run_c_log: Path | None, expected_structures: int | None
) -> dict:
    """A vs C: the test build without options, byte-for-byte against A.

    Everything here comes from Run C's own log. Without it the control was not
    measured for this report, and every figure and gate says so instead of
    rendering a zero nobody counted."""
    if not _readable(run_c_log):
        return {
            "measured": False,
            "matched": None,
            "mismatched": None,
            "unexpected": None,
            "expected": None,
            "timeouts": None,
            "field_counts": {},
            "mismatches": [],
            "gates": {"clean": None, "completeness": None, "no_aborted": None},
            "duration": None,
        }

    summary = parse_comparison_summary(run_c_log)
    measured = "matched" in summary and "mismatched" in summary
    matched = summary["matched"] if measured else None
    mismatched = summary["mismatched"] if measured else None
    mismatches = parse_regression_log(run_c_log)
    unexpected = sum(not mismatch.expected for mismatch in mismatches)
    timeouts = _timeouts(run_c_log)
    rows = [
        {
            "molfile_id": mismatch.molfile_id,
            "sdf": mismatch.sdf,
            "expected": mismatch.expected,
            "fields": differing_fields(mismatch),
            "reference": mismatch.reference,
            "current": mismatch.current,
        }
        for mismatch in mismatches
    ]

    return {
        "measured": measured,
        "matched": matched,
        "mismatched": mismatched,
        "unexpected": unexpected,
        "expected": len(mismatches) - unexpected,
        "timeouts": timeouts,
        "field_counts": dict(
            Counter(field for row in rows for field in row["fields"]).most_common()
        ),
        "mismatches": rows,
        "gates": {
            # The gate run_comparison.sh applies: no unexpected mismatch.
            "clean": unexpected == 0,
            "completeness": _completeness(
                matched, mismatched, timeouts, expected_structures
            ),
            "no_aborted": _no_aborted(run_c_log),
        },
        "duration": run_duration(run_c_log),
    }


def build_option_data(
    rows: list[dict],
    summary: dict,
    run_b_log: Path | None,
    expected_structures: int | None,
) -> dict:
    """A vs B: the test build with -MolecularInorganics, prefix-insensitive."""
    comparison = summary.get("comparison", {})
    counts = summary.get("counts", {})

    # No summary line in the run log means the comparison was never measured: the
    # log was truncated, or the pass compared byte-for-byte and wrote none. Falling
    # back to 0 matched and len(rows) mismatched would make the total the mismatch
    # count, put the headline at exactly 100.00% changed, and pass the prefix gate
    # on 0 + 0 == 0 -- a measured-looking report of a measurement that never
    # happened. Carry the absence through as None instead and render it as such.
    measured = "matched" in comparison and "mismatched" in comparison
    matched = comparison["matched"] if measured else None
    mismatched = comparison["mismatched"] if measured else None
    total = matched + mismatched if measured else None
    timeouts = _timeouts(run_b_log)

    novel = novel_split(rows)
    equivalent = counts.get("recmet_equivalent", 0)
    # The metal-free category and the route check exist only in classifications
    # written since they were added; an older summary has neither, and must not
    # read as zero metal-free mismatches or a passed check.
    checked = "route_check" in summary
    route = {key: summary["route_check"].get(key, 0) for key in ROUTE_CHECKS} if checked else None
    metal_free = counts.get("metal_free", 0) if checked else None
    # Matched metal-free structures whose warning level or text changed; only
    # logged by passes run since the tally was added.
    metal_free_warnings = (
        comparison["metal_free_warning_only"] + comparison["metal_free_message_only"]
        if "metal_free_warning_only" in comparison
        and "metal_free_message_only" in comparison
        else None
    )

    return {
        "total_structures": total,
        "matched": matched,
        "mismatched": mismatched,
        "mismatch_rate": _share(mismatched, total),
        "timeouts": timeouts,
        "comparison": comparison,
        "counts": counts,
        "equivalent": equivalent,
        "equivalent_share": _share(equivalent, mismatched),
        "novel": novel,
        "novel_share": _share(novel["total"], mismatched),
        "census": element_census(
            [row for row in rows if row["category"] == "novel"]
        ),
        "examples": _pick_examples(rows),
        "metal_free": metal_free,
        "metal_free_warnings": metal_free_warnings,
        "route": route,
        "gates": {
            # MI only changes how bonds to metals are treated.
            "metal_free": _is_zero(metal_free),
            "metal_free_warnings": _is_zero(metal_free_warnings),
            # The `/r` layer names the route the baseline's own warning names.
            "route": _is_zero(route["disagrees"]) if route else None,
            # Every structure that produced an InChI flips prefix under MI; the rest
            # are structures both libraries rejected.
            "prefix": (
                comparison.get("prefix_only", 0) + comparison.get("both_failed", 0)
                == matched
            )
            if measured
            else None,
            "completeness": _completeness(
                matched, mismatched, timeouts, expected_structures
            ),
            "no_aborted": _no_aborted(run_b_log),
        },
        "duration": run_duration(run_b_log) if run_b_log else None,
    }


def build_report_data(
    classifications_path: Path,
    summary_path: Path,
    baseline_label: str,
    test_label: str,
    run_a_log: Path | None = None,
    run_b_log: Path | None = None,
    run_c_log: Path | None = None,
    expected_structures: int | None = None,
) -> dict:
    """The report's data, kept apart by comparison.

    `reference` is Run A on its own, `a_vs_c` the control and `a_vs_b` the option
    pass. Nothing is shared between the two comparisons except the reference row
    count they are both checked against."""
    rows = load_classifications(classifications_path)
    summary = json.loads(Path(summary_path).read_text(encoding="utf-8"))

    return {
        "generated": datetime.now().isoformat(timespec="seconds"),
        "baseline_label": baseline_label,
        "test_label": test_label,
        "reference": build_reference_data(run_a_log, expected_structures),
        "a_vs_c": build_control_data(run_c_log, expected_structures),
        "a_vs_b": build_option_data(rows, summary, run_b_log, expected_structures),
    }


# Each comparison owns a colour: the reference (A) `--base`, the control (A vs C)
# `--ctl`, the option pass (A vs B) `--mi`. A part's colour runs through its
# band, badge and every sub-heading tag, so a reader always knows which
# comparison a figure belongs to.
_CSS_PARTS = """
.summary{display:grid;grid-template-columns:repeat(auto-fit,minmax(260px,1fr));gap:14px}
.card{background:var(--surface);border:1px solid var(--line);border-top:4px solid var(--part);
  padding:18px 20px;display:flex;flex-direction:column;gap:8px;text-decoration:none;color:inherit}
.card .n{font-family:var(--serif);font-size:34px;font-weight:600;line-height:1;
  font-variant-numeric:tabular-nums;color:var(--part)}
.card .n.is-good{color:var(--good)} .card .n.is-flag{color:var(--flag)} .card .n.is-muted{color:var(--muted)}
.card .d{font-size:14px;color:var(--muted);line-height:1.5}
.badge{align-self:flex-start;font-family:var(--mono);font-weight:500;font-size:12.5px;
  letter-spacing:.06em;color:var(--surface);background:var(--part);padding:4px 9px;white-space:nowrap}
.part{display:flex;flex-direction:column;gap:30px;border-top:4px solid var(--part);padding-top:22px}
.part-a{--part:var(--base)} .part-c{--part:var(--ctl)} .part-b{--part:var(--mi)}
.part-head{display:flex;flex-direction:column;gap:10px}
.part-head h2{font-size:25px}
.rule{font-size:14.5px;color:var(--muted);max-width:66ch}
.part h3{font-family:var(--serif);font-size:18px;font-weight:600;margin:0;display:flex;
  flex-wrap:wrap;align-items:baseline;gap:4px 10px;text-wrap:balance}
.tag{font-family:var(--mono);font-size:11px;font-weight:500;letter-spacing:.08em;color:var(--part);
  border:1px solid var(--part);padding:1px 6px;white-space:nowrap}
.passes td:first-child{font-family:var(--mono);font-weight:500;color:var(--part)}
.passes tr.pa{--part:var(--base)} .passes tr.pb{--part:var(--mi)} .passes tr.pc{--part:var(--ctl)}
.passes td{white-space:normal}
.diff td{white-space:normal;vertical-align:top}
.diff .v{font-family:var(--mono);font-size:12px;word-break:break-all;display:block}
.diff .v b{font-weight:500;color:var(--muted)}
.fields-list{display:flex;flex-wrap:wrap;gap:6px}
"""


def _gates(rows: list[tuple]) -> str:
    return '<div class="gates">' + "".join(
        f'<div class="gate">{_tick(value)}<span>{check}</span>'
        f'<span class="what">{what}</span></div>'
        for value, check, what in rows
    ) + "</div>"


def _fig(value: str, label: str, note: str, tone: str = "") -> str:
    return (
        f'<div class="fig {tone}"><div class="n">{value}</div>'
        f'<div class="k">{label}</div><div class="d">{note}</div></div>'
    )


def _verdict_c(control: dict) -> tuple[str, str]:
    """Tone and caption for the control. Absent means not measured, never zero."""
    unexpected = control["unexpected"]
    if unexpected is None:
        return "is-muted", "Not measured: this report was rendered without a Run C log."
    if unexpected == 0:
        return (
            "is-good",
            "With no options the test build reproduces the baseline exactly.",
        )

    return (
        "is-flag",
        "The test build does not reproduce the baseline without options, so "
        "differences under the option cannot be attributed to it until this is "
        "explained.",
    )


_MAX_LISTED = 25
_EXPECTED_TAG = ' <span class="na">(expected)</span>'


def _diff_cell(result: dict, fields: list[str]) -> str:
    return "".join(
        f'<span class="v"><b>{escape(field)}</b> '
        f'{escape(str(result.get(field, "—"))[:200])}</span>'
        for field in fields
    )


def _control_mismatches(control: dict) -> str:
    rows = control["mismatches"]
    if not rows:
        return ""
    listed = rows[:_MAX_LISTED]
    more = len(rows) - len(listed)
    field_chips = "".join(
        f'<span class="el"><b>{escape(field)}</b><span>{_fmt(count)}</span></span>'
        for field, count in control["field_counts"].items()
    )
    body = "".join(
        f'<tr><td class="mono">{escape(row["molfile_id"])}'
        f'{_EXPECTED_TAG if row["expected"] else ""}'
        f'<br><span class="na">{escape(row["sdf"])}</span></td>'
        f'<td>{_diff_cell(row["reference"], row["fields"])}</td>'
        f'<td>{_diff_cell(row["current"], row["fields"])}</td></tr>'
        for row in listed
    )
    more_note = (
        f'<p class="rule">{_fmt(more)} more in the Run C log.</p>' if more else ""
    )

    return f"""
  <section>
    <h3><span class="tag">A vs C</span> Mismatches</h3>
    <p>Fields that differ, counted over all {_fmt(len(rows))} mismatching structures:</p>
    <div class="census">{field_chips}</div>
    <div class="scroll"><table class="diff">
      <thead><tr><th>molfile</th><th>A &middot; baseline</th><th>C &middot; test, no options</th></tr></thead>
      <tbody>{body}</tbody></table></div>
    {more_note}
  </section>"""


_ROUTE_LABELS = {
    "agrees": "proxy and message name the same route",
    "disagrees": "proxy and message name different routes",
    "no_disconnection": "the baseline disconnected nothing",
    "no_metal": "no metal, so no route (<code>metal_free</code>)",
    "not_checked": "no usable <code>-RecMet</code> result",
}


def _route_table(route: dict | None) -> str:
    if route is None:
        return (
            '<p class="rule">Route check: not checked. These classifications predate it.</p>'
        )
    body = "".join(
        f'<tr><td>{escape(key)}</td><td>{_ROUTE_LABELS[key]}</td>'
        f'<td class="n">{_fmt(route[key])}</td></tr>'
        for key in ROUTE_CHECKS
    )

    return f"""<div class="scroll"><table>
      <thead><tr><th>route check</th><th>meaning</th><th class="n">count</th></tr></thead>
      <tbody>{body}</tbody></table></div>"""


def _metal_free_callout(metal_free: int | None, warnings: int | None) -> str:
    """Flag any change to a structure without a metal, which MI cannot explain."""
    parts = []
    if metal_free:
        parts.append(
            f"<b>{_fmt(metal_free)} structures without a metal changed their InChI.</b> "
            "Their IDs are in <code>ids/metal_free.txt</code>."
        )
    if warnings:
        parts.append(
            f"<b>{_fmt(warnings)} warning changes on structures without a metal</b> "
            "whose InChI is unchanged (<span class=\"mono\">metal_free_warning_only</span> "
            "plus <span class=\"mono\">metal_free_message_only</span>)."
        )
    if not parts:
        return ""

    return f"""<div class="callout flagged">
      <div class="eyebrow">unexpected</div>
      <p>{" ".join(parts)} MolecularInorganics only changes how bonds to metals are
      treated, so it has no mechanism to change these.</p>
    </div>"""


def _render_reference(data: dict) -> str:
    ref = data["reference"]

    return f"""
<section class="part part-a" id="reference">
  <div class="part-head">
    <span class="badge">Run A</span>
    <h2>The reference</h2>
    <p class="rule">{escape(data["baseline_label"])}, no options. Both comparisons below
    are measured against these rows and nothing else.</p>
  </div>
  <div class="figs">
    {_fig(_fmt(ref["rows"]), "reference rows", "Counted in the reference databases, independently of either comparison.")}
    {_fig(_fmt(ref["timeouts"]), "timed out", "Stored as a timeout row, not an InChI.")}
    {_fig(_duration(ref["duration"]), "wall clock", "Run A, reference pass.")}
  </div>
  {_gates([(ref["no_aborted"], "no aborted shards", "Run A")])}
</section>"""


def _render_control(data: dict) -> str:
    control = data["a_vs_c"]
    tone, caption = _verdict_c(control)
    gates = control["gates"]

    return f"""
<section class="part part-c" id="a-vs-c">
  <div class="part-head">
    <span class="badge">A vs C &middot; control</span>
    <h2>Does the test build reproduce the baseline without options?</h2>
    <p class="rule">Run C: {escape(data["test_label"])}, no options. Compared with Run A
    byte-for-byte on the four stored fields: <code>inchi</code> including its prefix,
    <code>key</code> including its flag characters, <code>exit</code> and the warning
    <code>message</code>.</p>
  </div>

  <section>
    <h3><span class="tag">A vs C</span> Result</h3>
    <div class="figs">
      {_fig(_fmt(control["unexpected"]), "unexpected mismatches", caption, tone)}
      {_fig(_fmt(control["matched"]), "identical", "Byte-for-byte equal to Run A.")}
      {_fig(_fmt(control["timeouts"]), "timed out", "Never compared. Expected ones are listed in the data config.")}
    </div>
  </section>

  <section>
    <h3><span class="tag">A vs C</span> Gates</h3>
    {_gates([
        (gates["clean"], "unexpected mismatches = 0", "no version drift"),
        (gates["completeness"], "identical + mismatched + timed out = reference rows", "completeness"),
        (gates["no_aborted"], "no aborted shards", "Run C"),
    ])}
  </section>
{_control_mismatches(control)}
  <section>
    <h3><span class="tag">A vs C</span> What this cannot see</h3>
    <p>AuxInfo and the full log are not stored, so a change in either is invisible
    here. A clean control proves the same InChI, InChIKey, return code and warning
    message as the baseline on every structure, not identical output.</p>
  </section>

  <section>
    <h3><span class="tag">A vs C</span> Cost</h3>
    <p>Run C took {_duration(control["duration"])}.</p>
  </section>
</section>"""


def _render_option(data: dict) -> str:
    option = data["a_vs_b"]
    c = option["comparison"]
    n = option["novel"]
    gates = option["gates"]
    census = "".join(
        f'<span class="el"><b>{escape(el)}</b><span>{count}</span></span>'
        for el, count in option["census"].items()
    )
    counts_rows = "".join(
        f'<tr><td>{escape(k)}</td><td class="n">{_fmt(v)}</td>'
        f'<td class="n">{_pct(_share(v, option["mismatched"]), 1)}</td></tr>'
        for k, v in option["counts"].items()
    )
    total = option["total_structures"]
    mismatch_pct = option["mismatch_rate"]
    eq_pct = _share(option["equivalent"], total)
    novel_pct = _share(n["total"], total)
    matched_pct = _share(option["matched"], total)

    return f"""
<section class="part part-b" id="a-vs-b">
  <div class="part-head">
    <span class="badge">A vs B &middot; MolecularInorganics</span>
    <h2>What does <code>-MolecularInorganics</code> change?</h2>
    <p class="rule">Run B: {escape(data["test_label"])} with <code>-MolecularInorganics</code>.
    Compared with Run A on the InChI body and the failure state only. The prefix,
    InChIKey and warning level are counted, not compared.</p>
  </div>

  <section>
    <h3><span class="tag">A vs B</span> Result</h3>
    <div class="figs">
      {_fig(_pct(mismatch_pct), "body changed", f"{_fmt(option['mismatched'])} structures differ chemically.", "is-mi")}
      {_fig(_pct(option["equivalent_share"], 1), "of those reproduce RecMet", f"{_fmt(option['equivalent'])} equal the baseline&rsquo;s reconnected <span class=\"mono\">/r</span> layer.", "is-base")}
      {_fig(_fmt(option["timeouts"]), "timed out", "Never compared. Expected ones are listed in the data config.")}
    </div>
  </section>

  <section>
    <h3><span class="tag">A vs B</span> Where the structures go</h3>
    <div class="cascade">
      <div class="bar-row"><div class="lab">compared</div>
        <div class="bar-track"><div class="bar-fill" style="width:100%;background:var(--base)"></div></div>
        <div class="val">{_fmt(total)}</div></div>
      <div class="bar-row"><div class="lab">body unchanged</div>
        <div class="bar-track"><div class="bar-fill" style="width:{_bar(matched_pct):.3f}%;background:var(--good)"></div></div>
        <div class="val">{_fmt(option["matched"])}</div></div>
      <div class="bar-row"><div class="lab">mismatched</div>
        <div class="bar-track"><div class="bar-fill" style="width:{_bar(mismatch_pct):.3f}%;min-width:3px;background:var(--mi)"></div></div>
        <div class="val">{_fmt(option["mismatched"])}</div></div>
      <div class="bar-row"><div class="lab">&rarr; recmet_equivalent</div>
        <div class="bar-track"><div class="bar-fill" style="width:{_bar(eq_pct):.3f}%;min-width:3px;background:var(--base)"></div></div>
        <div class="val">{_fmt(option["equivalent"])}</div></div>
      <div class="bar-row"><div class="lab">&rarr; novel</div>
        <div class="bar-track"><div class="bar-fill" style="width:{_bar(novel_pct):.3f}%;min-width:2px;background:var(--flag)"></div></div>
        <div class="val">{_fmt(n["total"])}</div></div>
    </div>
  </section>

  <section>
    <h3><span class="tag">A vs B</span> Tallies and gates</h3>
    <p>Comparison runs on the InChI body, not the raw string: <code>ichiprt1.c:1678</code> sets
    <span class="mono">is_beta</span> from the option alone, so every structure changes prefix
    (<span class="mono">InChI=1S/</span> &rarr; <span class="mono">InChI=1B/</span>) and InChIKey flag
    under MI, metal-free organics included. Those differences are counted, not compared.</p>
    <div class="scroll"><table>
      <thead><tr><th>tally</th><th class="n">count</th></tr></thead>
      <tbody>
        <tr><td>matched</td><td class="n">{_fmt(c.get("matched"))}</td></tr>
        <tr><td>mismatched</td><td class="n">{_fmt(c.get("mismatched"))}</td></tr>
        <tr><td>prefix_only</td><td class="n">{_fmt(c.get("prefix_only"))}</td></tr>
        <tr><td>key_only</td><td class="n">{_fmt(c.get("key_only"))}</td></tr>
        <tr><td>warning_only</td><td class="n">{_fmt(c.get("warning_only"))}</td></tr>
        <tr><td>message_only</td><td class="n">{_fmt(c.get("message_only"))}</td></tr>
        <tr><td>metal_free_warning_only</td><td class="n">{_fmt(c.get("metal_free_warning_only"))}</td></tr>
        <tr><td>metal_free_message_only</td><td class="n">{_fmt(c.get("metal_free_message_only"))}</td></tr>
        <tr><td>both_failed</td><td class="n">{_fmt(c.get("both_failed"))}</td></tr>
        <tr><td>failure_kind_only</td><td class="n">{_fmt(c.get("failure_kind_only"))}</td></tr>
      </tbody></table></div>
    {_gates([
        (gates["metal_free"], "metal-free mismatches = 0", "MI touches metals only"),
        (gates["metal_free_warnings"], "metal-free warning changes = 0", "MI touches metals only"),
        (gates["route"], "<span class=\"mono\">/r</span> proxy disagrees with the baseline message = 0", "route proxy"),
        (gates["prefix"], "prefix_only + both_failed = matched", "prefix flip"),
        (gates["completeness"], "matched + mismatched + timed out = reference rows", "completeness"),
        (gates["no_aborted"], "no aborted shards", "Run B"),
    ])}
  </section>

  <section>
    <h3><span class="tag">A vs B</span> What the two categories mean</h3>
    <p><b>MolecularInorganics does not create bonds. It declines to break the ones the molfile
    already has.</b> The old code breaks them by two different routes, and <code>-RecMet</code>
    only reverses one:</p>
    <div class="scroll"><table>
      <thead><tr><th>route taken by the old code</th><th class="n">count</th><th>RecMet</th></tr></thead>
      <tbody>
        <tr><td>metal disconnection &mdash; MI output equals the restored bonds</td>
          <td class="n">{_fmt(option["equivalent"])}</td><td>reverses it, emits <span class="mono">/r</span></td></tr>
        <tr><td>metal disconnection &mdash; but the two disagree</td>
          <td class="n">{_fmt(n["metal_pathway"])}</td><td>reverses it, differently</td></tr>
        <tr><td>salt disconnection</td>
          <td class="n">{_fmt(n["salt_pathway"])}</td><td>cannot undo it, no <span class="mono">/r</span></td></tr>
      </tbody></table></div>
    <p>The presence of an <span class="mono">/r</span> layer is the proxy for which route was taken.
    It is checked per structure against the baseline&rsquo;s own warning from the
    <code>-RecMet</code> re-run: &ldquo;Metal was disconnected&rdquo; should come with an
    <span class="mono">/r</span> layer, &ldquo;Salt was disconnected&rdquo; alone without one.</p>
{_route_table(option["route"])}
{_specimen("MI reproduces the restored bonds", option["examples"]["equivalent"])}
{_specimen("Salt route — RecMet cannot restore it", option["examples"]["salt_pathway"], "no /r layer — salt disconnection is not reversible by RecMet")}
{_specimen("Metal route — the two disagree", option["examples"]["metal_pathway"])}
  </section>

  <section>
    <h3><span class="tag">A vs B</span> Categories</h3>
    <div class="scroll"><table>
      <thead><tr><th>category</th><th class="n">count</th><th class="n">share of mismatches</th></tr></thead>
      <tbody>{counts_rows}</tbody></table></div>
    <p>Elements other than C and H across the <span class="mono">novel</span> formulae:</p>
    <div class="census">{census}</div>
{_metal_free_callout(option["metal_free"], option["metal_free_warnings"])}
    <div class="callout flagged">
      <div class="eyebrow">needs chemical review</div>
      <p><b>{_fmt(n["metal_pathway"])} structures.</b> Both the old code and MI act on the metal,
      and they still disagree. This is the only part of the report not explained by a stated
      mechanism.</p>
    </div>
  </section>

  <section>
    <h3><span class="tag">A vs B</span> What this cannot see</h3>
    <p>The comparison normalises away the prefix and the InChIKey flag, so it is blind by
    construction to the change affecting the most structures: all {_fmt(c.get("prefix_only"))}
    matched structures changed both. A mismatch count means &ldquo;the chemical body
    differs&rdquo;, never &ldquo;the output is unchanged&rdquo;. MI failures on structures the
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

  <section>
    <h3><span class="tag">A vs B</span> Cost</h3>
    <p>Run B took {_duration(option["duration"])}.</p>
  </section>
</section>"""


def render_html(data: dict) -> str:
    control = data["a_vs_c"]
    option = data["a_vs_b"]
    c_tone, c_caption = _verdict_c(control)

    return f"""<title>MolecularInorganics Comparison</title>
<link rel="stylesheet" href="https://fonts.googleapis.com/css2?family=IBM+Plex+Mono:wght@400;500&family=IBM+Plex+Sans:wght@400;500;600&family=IBM+Plex+Serif:wght@500;600&display=swap">
<style>
{_CSS}
{_CSS_PARTS}
</style>

<div class="wrap">
<header>
  <div class="eyebrow">IUPAC InChI &middot; MolecularInorganics comparison</div>
  <h1>MolecularInorganics on PubChem</h1>
  <p class="lede">Two separate comparisons against one reference. <b>A vs C</b> checks that the
  test build reproduces the {escape(data["baseline_label"])} baseline without options.
  <b>A vs B</b> measures what <code>-MolecularInorganics</code> changes.</p>
  <div class="meta">
    <span>baseline <b>{escape(data["baseline_label"])}</b></span>
    <span>test <b>{escape(data["test_label"])}</b></span>
    <span>generated {escape(data["generated"])}</span>
  </div>
</header>

<section>
  <div class="scroll"><table class="passes">
    <thead><tr><th>run</th><th>library</th><th>options</th><th>role</th></tr></thead>
    <tbody>
      <tr class="pa"><td>A</td><td>{escape(data["baseline_label"])}</td><td>none</td><td>the reference</td></tr>
      <tr class="pc"><td>C</td><td>{escape(data["test_label"])}</td><td>none</td><td>control: compared with A byte-for-byte</td></tr>
      <tr class="pb"><td>B</td><td>{escape(data["test_label"])}</td><td><code>-MolecularInorganics</code></td><td>compared with A ignoring the prefix</td></tr>
    </tbody></table></div>
</section>

<section class="summary">
  <a class="card part-c" href="#a-vs-c">
    <span class="badge">A vs C &middot; control</span>
    <div class="n {c_tone}">{_fmt(control["unexpected"])}</div>
    <div class="d">unexpected mismatches. {c_caption}</div>
  </a>
  <a class="card part-b" href="#a-vs-b">
    <span class="badge">A vs B &middot; MolecularInorganics</span>
    <div class="n">{_pct(option["mismatch_rate"])}</div>
    <div class="d">of structures change their InChI body
    ({_fmt(option["mismatched"])}); {_pct(option["equivalent_share"], 1)} of those reproduce RecMet.</div>
  </a>
</section>
{_render_reference(data)}
{_render_control(data)}
{_render_option(data)}

<footer>
  <p>Generated by <code>INCHI-1-TEST/comparisons/molecular_inorganics/report.py</code> from
  <code>classifications.csv</code>, <code>summary.json</code> and the three run logs.</p>
  <p>Context: issue #259 proposes making MolecularInorganics the standard path with opt-out
  parameters for the legacy metal handling.</p>
</footer>
</div>
"""


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Render an HTML report from the comparison output."
    )
    parser.add_argument("--classifications", required=True, type=Path)
    parser.add_argument("--summary", required=True, type=Path)
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
        classifications_path=args.classifications,
        summary_path=args.summary,
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
