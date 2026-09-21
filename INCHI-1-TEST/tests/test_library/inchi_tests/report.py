"""Render a campaign report from the output of `campaign.py`.

Turns `classifications.csv` plus `summary.json` into a self-contained HTML page,
so every run of the campaign produces the same report without hand-assembly.

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

from inchi_tests.campaign import has_reconnected_layer

_TIMESTAMP = re.compile(r"(\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2})")
_ELEMENT = re.compile(r"[A-Z][a-z]?")
_ORGANIC = {"C", "H"}


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


def _is_zero(count: int | None) -> bool | None:
    """A gate over a count that may not have been obtainable.

    `None` propagates as `None` -- not checked -- rather than collapsing to a pass.
    A log that could not be opened is not evidence that nothing went wrong in it."""
    return None if count is None else count == 0


def _share(part: int | None, whole: int | None) -> float | None:
    return part / whole * 100 if part is not None and whole else None


def _no_aborted(*log_paths: Path | None) -> bool | None:
    """Whether no shard aborted in any run whose log could actually be read."""
    counts = [
        count_log_lines(log_path, "Aborted regression")
        for log_path in log_paths
        if log_path
    ]
    if not counts or any(count is None for count in counts):
        return None

    return all(count == 0 for count in counts)


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
    rows = load_classifications(classifications_path)
    summary = json.loads(Path(summary_path).read_text(encoding="utf-8"))
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

    novel = novel_split(rows)
    equivalent = counts.get("recmet_equivalent", 0)

    return {
        "generated": datetime.now().isoformat(timespec="seconds"),
        "baseline_label": baseline_label,
        "test_label": test_label,
        "total_structures": total,
        "matched": matched,
        "mismatched": mismatched,
        "mismatch_rate": _share(mismatched, total),
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
        "gates": {
            # Every structure that produced an InChI flips prefix under MI; the rest
            # are structures both libraries rejected.
            "run_b_prefix_gate": (
                comparison.get("prefix_only", 0) + comparison.get("both_failed", 0)
                == matched
            )
            if measured
            else None,
            # Against the reference row count, which is arrived at by counting rows
            # in the reference databases -- a different route to the number than the
            # comparator's tallies. Checking `matched + mismatched` against a total
            # *defined* as their sum, as this once did, is a tautology that renders
            # green on exactly the short-total runs it is meant to catch.
            "completeness": (matched + mismatched == expected_structures)
            if measured and expected_structures is not None
            else None,
            "run_c_clean": _is_zero(
                count_log_lines(run_c_log, "regression test failed:")
            )
            if run_c_log
            else None,
            "no_aborted": _no_aborted(run_a_log, run_b_log, run_c_log),
        },
        "durations": {
            "run_a": run_duration(run_a_log) if run_a_log else None,
            "run_b": run_duration(run_b_log) if run_b_log else None,
            "run_c": run_duration(run_c_log) if run_c_log else None,
        },
    }


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


def render_html(data: dict) -> str:
    c = data["comparison"]
    n = data["novel"]
    census = "".join(
        f'<span class="el"><b>{escape(el)}</b><span>{count}</span></span>'
        for el, count in data["census"].items()
    )
    counts_rows = "".join(
        f'<tr><td>{escape(k)}</td><td class="n">{_fmt(v)}</td>'
        f'<td class="n">{_pct(_share(v, data["mismatched"]), 1)}</td></tr>'
        for k, v in data["counts"].items()
    )
    mismatch_pct = data["mismatch_rate"]
    eq_pct = _share(data["equivalent"], data["total_structures"])
    novel_pct = _share(n["total"], data["total_structures"])
    matched_pct = _share(data["matched"], data["total_structures"])

    # An absent Run C count means the control was not measured for this report --
    # report.py was run without --run-c-log. Rendering that as a green zero would
    # assert a check nobody performed.
    run_c_mismatches = c.get("mismatched_run_c")
    if run_c_mismatches is None:
        run_c_class, run_c_note = (
            "is-muted",
            "Not measured: this report was rendered without a Run C log.",
        )
    elif run_c_mismatches == 0:
        run_c_class, run_c_note = (
            "is-good",
            "With no options the test build reproduces the baseline exactly.",
        )
    else:
        run_c_class, run_c_note = (
            "is-flag",
            "The test build does not reproduce the baseline without options. "
            "Run B&rsquo;s differences cannot be attributed to the option.",
        )

    return f"""<title>MolecularInorganics Campaign</title>
<link rel="stylesheet" href="https://fonts.googleapis.com/css2?family=IBM+Plex+Mono:wght@400;500&family=IBM+Plex+Sans:wght@400;500;600&family=IBM+Plex+Serif:wght@500;600&display=swap">
<style>
:root{{
  --bg:#F6F7F9; --surface:#FFFFFF; --surface-2:#EFF2F6;
  --ink:#171B22; --muted:#616B7C; --line:#DDE2E9; --line-soft:#E9EDF2;
  --base:#2F5D8C; --mi:#9A5B2B; --mi-soft:#F4E9DE;
  --good:#2E7D5B; --flag:#8A3F3F;
  --serif:"IBM Plex Serif",Georgia,serif;
  --sans:"IBM Plex Sans",-apple-system,BlinkMacSystemFont,"Segoe UI",sans-serif;
  --mono:"IBM Plex Mono",ui-monospace,Menlo,monospace;
}}
@media (prefers-color-scheme: dark){{
  :root:not([data-theme="light"]){{
    --bg:#101318; --surface:#171B22; --surface-2:#1E232C;
    --ink:#E7EAF0; --muted:#98A2B3; --line:#2A313B; --line-soft:#222832;
    --base:#87AEDA; --mi:#D9955C; --mi-soft:#2C2218;
    --good:#66BE93; --flag:#D98C8C;
  }}
}}
:root[data-theme="dark"]{{
  --bg:#101318; --surface:#171B22; --surface-2:#1E232C;
  --ink:#E7EAF0; --muted:#98A2B3; --line:#2A313B; --line-soft:#222832;
  --base:#87AEDA; --mi:#D9955C; --mi-soft:#2C2218;
  --good:#66BE93; --flag:#D98C8C;
}}
*{{box-sizing:border-box}}
body{{background:var(--bg);color:var(--ink);font-family:var(--sans);font-size:16px;
  line-height:1.65;padding-inline:20px;padding-block:0}}
.wrap{{max-width:820px;margin:0 auto;padding-block:56px 80px;display:flex;
  flex-direction:column;gap:42px}}
h1,h2{{font-family:var(--serif);text-wrap:balance;margin:0;line-height:1.25}}
h1{{font-size:clamp(30px,5vw,40px);font-weight:600;letter-spacing:-.01em}}
h2{{font-size:21px;font-weight:600;display:flex;align-items:baseline;gap:12px}}
h2 .num{{font-family:var(--mono);font-size:12px;color:var(--muted);font-weight:500;letter-spacing:.08em}}
p{{margin:0}} section{{display:flex;flex-direction:column;gap:14px}}
code,.mono{{font-family:var(--mono);font-size:.88em}}
code{{background:var(--surface-2);padding:1px 5px;border-radius:3px}}
.eyebrow{{font-family:var(--mono);font-size:11px;letter-spacing:.14em;
  text-transform:uppercase;color:var(--muted)}}
header{{display:flex;flex-direction:column;gap:16px;border-bottom:2px solid var(--ink);
  padding-bottom:24px}}
.lede{{font-size:18px;color:var(--muted);max-width:62ch}}
.meta{{display:flex;flex-wrap:wrap;gap:6px 26px;font-family:var(--mono);font-size:12.5px;color:var(--muted)}}
.meta b{{color:var(--ink);font-weight:500}}
.figs{{display:grid;grid-template-columns:repeat(auto-fit,minmax(200px,1fr));gap:1px;
  background:var(--line);border:1px solid var(--line)}}
.fig{{background:var(--surface);padding:20px;display:flex;flex-direction:column;gap:5px}}
.fig .n{{font-family:var(--serif);font-size:36px;font-weight:600;line-height:1;
  font-variant-numeric:tabular-nums;letter-spacing:-.02em}}
.fig .k{{font-family:var(--mono);font-size:11px;letter-spacing:.1em;
  text-transform:uppercase;color:var(--muted)}}
.fig .d{{font-size:13.5px;color:var(--muted);line-height:1.5}}
.fig.is-good .n{{color:var(--good)}} .fig.is-mi .n{{color:var(--mi)}} .fig.is-base .n{{color:var(--base)}}
.fig.is-flag .n{{color:var(--flag)}} .fig.is-muted .n{{color:var(--muted)}}
.cascade{{display:flex;flex-direction:column;gap:9px;background:var(--surface);
  border:1px solid var(--line);padding:20px}}
.bar-row{{display:grid;grid-template-columns:minmax(120px,176px) 1fr auto;gap:14px;align-items:center}}
.bar-row .lab{{font-family:var(--mono);font-size:12px;color:var(--muted);text-align:right}}
.bar-track{{background:var(--surface-2);height:16px;overflow:hidden}}
.bar-fill{{height:100%}}
.bar-row .val{{font-family:var(--mono);font-size:12.5px;font-variant-numeric:tabular-nums;
  min-width:84px;text-align:right}}
.scroll{{overflow-x:auto;border:1px solid var(--line);background:var(--surface)}}
table{{border-collapse:collapse;width:100%;font-size:14px}}
th,td{{padding:9px 14px;text-align:left;border-bottom:1px solid var(--line-soft);white-space:nowrap}}
thead th{{background:var(--surface-2);font-family:var(--mono);font-size:11px;
  letter-spacing:.07em;text-transform:uppercase;color:var(--muted);font-weight:500}}
tbody tr:last-child td{{border-bottom:none}}
td.n,th.n{{text-align:right;font-family:var(--mono);font-variant-numeric:tabular-nums}}
.specimen{{background:var(--surface);border:1px solid var(--line);border-left:3px solid var(--mi)}}
.specimen .cid{{font-family:var(--mono);font-size:11.5px;letter-spacing:.06em;color:var(--muted);
  padding:11px 16px 9px;border-bottom:1px solid var(--line-soft)}}
.specimen dl{{margin:0;display:flex;flex-direction:column}}
.specimen .row{{display:grid;grid-template-columns:88px 1fr;gap:12px;padding:8px 16px;align-items:baseline}}
.specimen .row+.row{{border-top:1px solid var(--line-soft)}}
.specimen dt{{font-family:var(--mono);font-size:11px;letter-spacing:.05em;
  text-transform:uppercase;color:var(--muted)}}
.specimen dd{{margin:0;font-family:var(--mono);font-size:12.5px;overflow-x:auto;white-space:nowrap}}
.absent{{color:var(--muted);font-style:italic;font-family:var(--sans);font-size:13px}}
.gates{{display:flex;flex-direction:column;gap:1px;background:var(--line);border:1px solid var(--line)}}
.gate{{background:var(--surface);padding:11px 15px;display:flex;flex-wrap:wrap;gap:4px 12px;
  align-items:baseline;font-family:var(--mono);font-size:12.5px}}
.tick{{color:var(--good);font-weight:600}} .cross{{color:var(--flag);font-weight:600}}
.na{{color:var(--muted)}} .gate .what{{color:var(--muted)}}
.census{{display:flex;flex-wrap:wrap;gap:6px}}
.el{{display:inline-flex;align-items:baseline;gap:5px;border:1px solid var(--line);
  background:var(--surface);padding:4px 9px;font-family:var(--mono);font-size:12.5px}}
.el b{{font-weight:500}} .el span{{color:var(--muted);font-size:11.5px}}
.callout{{background:var(--surface);border:1px solid var(--line);border-left:3px solid var(--base);
  padding:16px 18px;display:flex;flex-direction:column;gap:8px;font-size:14.5px}}
.callout.clean{{border-left-color:var(--good)}}
.callout.flagged{{border-left-color:var(--flag)}}
footer{{border-top:1px solid var(--line);padding-top:20px;font-size:13px;color:var(--muted);
  display:flex;flex-direction:column;gap:6px}}
@media (max-width:560px){{
  .bar-row{{grid-template-columns:1fr auto;gap:6px 10px}}
  .bar-row .lab{{text-align:left}} .bar-track{{grid-column:1/-1}}
  .specimen .row{{grid-template-columns:1fr;gap:3px}}
}}
</style>

<div class="wrap">
<header>
  <div class="eyebrow">IUPAC InChI &middot; regression campaign</div>
  <h1>MolecularInorganics on PubChem</h1>
  <p class="lede">What <code>-MolecularInorganics</code> changes against the
  {escape(data["baseline_label"])} baseline, measured over
  {_fmt(data["total_structures"])} structures.</p>
  <div class="meta">
    <span>baseline <b>{escape(data["baseline_label"])}</b></span>
    <span>test <b>{escape(data["test_label"])}</b></span>
    <span>generated {escape(data["generated"])}</span>
  </div>
</header>

<section>
  <div class="figs">
    <div class="fig {run_c_class}"><div class="n">{_fmt(run_c_mismatches)}</div>
      <div class="k">Run C mismatches</div>
      <div class="d">{run_c_note}</div></div>
    <div class="fig is-mi"><div class="n">{_pct(mismatch_pct)}</div>
      <div class="k">changed by MI</div>
      <div class="d">{_fmt(data["mismatched"])} structures differ chemically.</div></div>
    <div class="fig is-base"><div class="n">{_pct(data["equivalent_share"], 1)}</div>
      <div class="k">reproduce RecMet</div>
      <div class="d">{_fmt(data["equivalent"])} equal the reconnected <span class="mono">/r</span> layer.</div></div>
  </div>
</section>

<section>
  <h2><span class="num">01</span> Where the structures go</h2>
  <div class="cascade">
    <div class="bar-row"><div class="lab">processed</div>
      <div class="bar-track"><div class="bar-fill" style="width:100%;background:var(--base)"></div></div>
      <div class="val">{_fmt(data["total_structures"])}</div></div>
    <div class="bar-row"><div class="lab">body unchanged</div>
      <div class="bar-track"><div class="bar-fill" style="width:{_bar(matched_pct):.3f}%;background:var(--good)"></div></div>
      <div class="val">{_fmt(data["matched"])}</div></div>
    <div class="bar-row"><div class="lab">mismatched</div>
      <div class="bar-track"><div class="bar-fill" style="width:{_bar(mismatch_pct):.3f}%;min-width:3px;background:var(--mi)"></div></div>
      <div class="val">{_fmt(data["mismatched"])}</div></div>
    <div class="bar-row"><div class="lab">&rarr; recmet_equivalent</div>
      <div class="bar-track"><div class="bar-fill" style="width:{_bar(eq_pct):.3f}%;min-width:3px;background:var(--base)"></div></div>
      <div class="val">{_fmt(data["equivalent"])}</div></div>
    <div class="bar-row"><div class="lab">&rarr; novel</div>
      <div class="bar-track"><div class="bar-fill" style="width:{_bar(novel_pct):.3f}%;min-width:2px;background:var(--flag)"></div></div>
      <div class="val">{_fmt(n["total"])}</div></div>
  </div>
</section>

<section>
  <h2><span class="num">02</span> Tallies and gates</h2>
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
      <tr><td>both_failed</td><td class="n">{_fmt(c.get("both_failed"))}</td></tr>
      <tr><td>failure_kind_only</td><td class="n">{_fmt(c.get("failure_kind_only"))}</td></tr>
    </tbody></table></div>
  <div class="gates">
    <div class="gate">{_tick(data["gates"]["run_b_prefix_gate"])}
      <span>prefix_only + both_failed = matched</span><span class="what">Run B</span></div>
    <div class="gate">{_tick(data["gates"]["completeness"])}
      <span>matched + mismatched = reference rows</span><span class="what">completeness</span></div>
    <div class="gate">{_tick(data["gates"]["run_c_clean"])}
      <span>Run C mismatches = 0</span><span class="what">no version drift</span></div>
    <div class="gate">{_tick(data["gates"]["no_aborted"])}
      <span>no aborted shards</span><span class="what">all runs</span></div>
  </div>
</section>

<section>
  <h2><span class="num">03</span> What the two categories mean</h2>
  <p><b>MolecularInorganics does not create bonds. It declines to break the ones the molfile
  already has.</b> The old code breaks them by two different routes, and <code>-RecMet</code>
  only reverses one:</p>
  <div class="scroll"><table>
    <thead><tr><th>route taken by the old code</th><th class="n">count</th><th>RecMet</th></tr></thead>
    <tbody>
      <tr><td>metal disconnection &mdash; MI output equals the restored bonds</td>
        <td class="n">{_fmt(data["equivalent"])}</td><td>reverses it, emits <span class="mono">/r</span></td></tr>
      <tr><td>metal disconnection &mdash; but the two disagree</td>
        <td class="n">{_fmt(n["metal_pathway"])}</td><td>reverses it, differently</td></tr>
      <tr><td>salt disconnection</td>
        <td class="n">{_fmt(n["salt_pathway"])}</td><td>cannot undo it, no <span class="mono">/r</span></td></tr>
    </tbody></table></div>
  <p>The presence of an <span class="mono">/r</span> layer is the proxy for which route was taken.
  On the pilot corpus it separated the two perfectly, with the baseline emitting
  &ldquo;Metal was disconnected&rdquo; for every <span class="mono">/r</span> case and
  &ldquo;Salt was disconnected&rdquo; for every case without one.</p>
{_specimen("MI reproduces the restored bonds", data["examples"]["equivalent"])}
{_specimen("Salt route — RecMet cannot restore it", data["examples"]["salt_pathway"], "no /r layer — salt disconnection is not reversible by RecMet")}
{_specimen("Metal route — the two disagree", data["examples"]["metal_pathway"])}
</section>

<section>
  <h2><span class="num">04</span> Categories</h2>
  <div class="scroll"><table>
    <thead><tr><th>category</th><th class="n">count</th><th class="n">share of mismatches</th></tr></thead>
    <tbody>{counts_rows}</tbody></table></div>
  <p>Elements other than C and H across the <span class="mono">novel</span> formulae:</p>
  <div class="census">{census}</div>
  <div class="callout flagged">
    <div class="eyebrow">needs chemical review</div>
    <p><b>{_fmt(n["metal_pathway"])} structures.</b> Both the old code and MI act on the metal,
    and they still disagree. This is the only part of the report not explained by a stated
    mechanism.</p>
  </div>
</section>

<section>
  <h2><span class="num">05</span> Cost</h2>
  <div class="scroll"><table>
    <thead><tr><th>stage</th><th class="n">wall clock</th></tr></thead>
    <tbody>
      <tr><td>Run A &mdash; reference</td><td class="n">{_duration(data["durations"]["run_a"])}</td></tr>
      <tr><td>Run B &mdash; MolecularInorganics</td><td class="n">{_duration(data["durations"]["run_b"])}</td></tr>
      <tr><td>Run C &mdash; no options</td><td class="n">{_duration(data["durations"]["run_c"])}</td></tr>
    </tbody></table></div>
</section>

<section>
  <h2><span class="num">06</span> What this cannot see</h2>
  <p>The comparison normalises away the prefix and the InChIKey flag, so it is blind by
  construction to the change affecting the most structures: all {_fmt(c.get("prefix_only"))}
  matched structures changed both. A mismatch count means &ldquo;the chemical body
  differs&rdquo;, never &ldquo;the output is unchanged&rdquo;. MI failures on structures the
  baseline also rejected are invisible (<span class="mono">both_failed</span>:
  {_fmt(c.get("both_failed"))}), and <span class="mono">aux</span>,
  <span class="mono">log</span> and <span class="mono">message</span> are not stored, so
  changed warnings appear only as <span class="mono">warning_only</span>:
  {_fmt(c.get("warning_only"))}. Where both runs failed, a differing error code is a
  failure <em>kind</em> rather than a warning flip and is counted separately
  (<span class="mono">failure_kind_only</span>: {_fmt(c.get("failure_kind_only"))}).</p>
</section>

<footer>
  <p>Generated by <code>inchi_tests/report.py</code> from
  <code>classifications.csv</code> and <code>summary.json</code>.</p>
  <p>Context: issue #259 proposes making MolecularInorganics the standard path with opt-out
  parameters for the legacy metal handling.</p>
</footer>
</div>
"""


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Render an HTML report from campaign output."
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
            "of the comparison -- run_campaign.sh passes the reference row count. "
            "Without it the completeness gate reads 'not checked' rather than "
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
    # None when no Run C log was given: the control was not measured for this
    # report, which must not render as a measured zero under a caption asserting
    # that the build reproduces the baseline.
    data["comparison"]["mismatched_run_c"] = (
        count_log_lines(args.run_c_log, "regression test failed:")
        if args.run_c_log is not None
        else None
    )

    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(render_html(data), encoding="utf-8")
    print(f"Wrote {args.output} ({args.output.stat().st_size:,} bytes).")


if __name__ == "__main__":
    main()
