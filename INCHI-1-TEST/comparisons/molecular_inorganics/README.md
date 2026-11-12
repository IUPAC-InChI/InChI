# MolecularInorganics comparison

What does Molecular Inorganics change relative to a released InChI version? This
directory answers that question over a whole PubChem dataset. It is a one-off
comparison, not part of the test suite: nothing here runs in CI, and the shared
harness under `INCHI-1-TEST/tests/test_library` knows nothing about it.

It runs three passes over the same SDFs, the three columns of the test strategy
in issue #259, and compares B and C with A separately:

| pass | #259 column | library | options | compared with A |
| --- | --- | --- | --- | --- |
| A | 1: version ≤ 1.07.5 | baseline tag | none | — (the reference) |
| B | 2: Molecular Inorganics on | working tree | none (the default) | InChI body and failure state |
| C | 3: Molecular Inorganics off | working tree | `-LegacyMetalHandling` | every field except the prefix and the key flags |

## Run it

On the host the script needs Python 3.11 or newer with the `venv` module (on
Debian and Ubuntu the `python3.X-venv` package), git with the baseline tag
fetched, CMake and a C/C++ compiler, `wget`, and network access for the
download and for googletest, which the root `CMakeLists.txt` fetches on first
configure. The container below has all of it.

From the repository root:

```Shell
./INCHI-1-TEST/comparisons/molecular_inorganics/run_comparison.sh <compound|compound3d|substance>
```

The script creates a virtual environment, builds both libraries in Release mode
(`build_with_cmake.sh` sets no `CMAKE_BUILD_TYPE`, and `libinchi` gates `-O1` on
Release, so an unconfigured build is several times slower), adds a git worktree
for the baseline tag, downloads and md5-verifies the dataset, runs all three
passes, classifies the differences and writes everything to
`INCHI-1-TEST/tests/test_library/data/pubchem/comparisons/molecular_inorganics/<dataset>/`:

| path | contents |
| --- | --- |
| `report.html` | the rendered report |
| `summary.json` | counts of both comparisons, one section each (`a_vs_b`, `a_vs_c`) |
| `a_vs_b/a_vs_b_classifications.csv`, `a_vs_c/a_vs_c_classifications.csv` | one row per mismatch |
| `a_vs_b/ids/`, `a_vs_c/ids/` | mismatching IDs, one file per category |
| `logs/` | the three pass logs |

Re-running is safe: the environment, the worktree and the references are reused,
so a reference pass that stopped computes exactly the references that are
missing. Pass `--skip-download` to re-run against data already on disk.
`BASELINE_TAG` (default `v1.07.5`) and `RUN_TAG` are environment overrides.

The script stops with an error on the conditions that would otherwise go
unnoticed: an SDF whose md5 does not match (`validate.py` only prints these and
always exits 0), an SDF whose processing stopped during a pass (`run.py` logs
`Aborted …` and continues, so it never reaches the exit code), and a leftover
`.partial` reference.

### In a container

`docker-compose.yml` here defines an `inchi-comparison` service for long runs:

```Shell
docker compose -f INCHI-1-TEST/comparisons/molecular_inorganics/docker-compose.yml build
docker compose -f INCHI-1-TEST/comparisons/molecular_inorganics/docker-compose.yml \
    run --rm inchi-comparison substance --skip-download
```

Arguments after the service name go to `run_comparison.sh`; `BASELINE_TAG=v1.06
docker compose ...` selects another baseline.

The service **bind-mounts the repository**, so it always runs the current working
tree and needs no rebuild after a code change. What it builds goes on named
volumes rather than into the bind mount:

| volume | mounted at | why |
| --- | --- | --- |
| `comparison_build` | `/inchi/CMake_build` | container-built binaries must not overwrite the host's |
| `comparison_worktree` | `/comparison/worktree` | the baseline worktree would otherwise be outside the mount and be lost on exit |
| `comparison_venv` | `/opt/comparison/venv` | a virtualenv's interpreter paths are only valid inside the container |

All three persist between runs. `WORKTREE` and `VENV` are what redirect the
script onto them; both default to in-repo paths when unset, so a host run is
unaffected.

The container runs as `1001:1001` so that references, logs and the report stay
owned by the host user instead of root. If your UID differs, set `COMPARISON_UID`
and `COMPARISON_GID` at **both** build and run time — they are a build argument as
well, because the volume mount points must be created with that ownership:

```Shell
COMPARISON_UID=$(id -u) COMPARISON_GID=$(id -g) \
    docker compose -f INCHI-1-TEST/comparisons/molecular_inorganics/docker-compose.yml build
```

`git worktree` bookkeeping is written into the bind-mounted `.git`, where a
registration can outlive the volume holding the worktree, which is why the script
removes registrations without a directory before adding one.

## How the comparison works

Since #280, the version prefix is a calculation-class designator and, where
Molecular Inorganics processed the structure, a subversion designator: a
structure Molecular Inorganics processes gets `InChI=1BB/` (InChIKey flags `BB`),
every other structure keeps `InChI=1S/` (`SA`), and `-LegacyMetalHandling`
produces non-standard `InChI=1N/` (`NA`). Comparing raw strings would therefore
mark every structure of pass C, and every metal structure of pass B, as changed.

The passes store the raw result (full InChI, key, exit code and the warning
`message`) and decide at comparison time what counts as a match. `aux` and `log`
are not stored; they would dominate reference size at PubChem scale. A reference
storing any other set of fields, e.g. one written before `message` was added, is
refused with an `Aborted` line naming the file: delete it and the reference pass
recomputes it. References are namespaced by `--run-tag`
(`<sdf>.<tag>.regression_reference.sqlite`) and logs by `--log-tag`, so the
comparison never reads or overwrites the committed references the regression
tests use.

`run.py` compares each result with the reference itself, in a loop over
`sdf_pipeline.core.run`, with a rule from `comparators.py`, and logs the rule's
counts as a `Comparison summary` line. A molfile that exceeds the time limit of
`core.run` (60 s) is logged as `timed out`.

### A vs B: what Molecular Inorganics changes

Pass B uses `--compare=prefix-insensitive` (`OptionComparator`): the InChI *body*
and a failure flag must match, where failure means `exit >= 2` or no InChI. Exit
code 1 is a warning, and a metal disconnection always warns, so treating it as a
failure would misclassify most of the structures of interest. The differences it
accepts are counted: `prefix_only`, `key_only`, `warning_only`, `message_only`,
`unaffected` (the prefix changed and nothing else did), `both_failed`,
`failure_kind_only` (both failed with different error codes), and, on structures
without a metal, `without_metal_prefix_changed`, `without_metal_warning_only` and
`without_metal_message_only`, each of which fails a gate.

Every mismatch is re-computed with the baseline and `-RecMet` and gets one
category, named for what happened to the structure. The old code breaks bonds to
metals by two routes and `-RecMet` only reverses one of them; the route is the
one 1.07.5 states in its own Run A warning, "Metal was disconnected" or "Salt was
disconnected". Checked in this order:

| category | what happened |
| --- | --- |
| `error_in_reference` / `error_under_mi` | one side produced no InChI |
| `changed_without_metal` | neither side's formula contains a metal (InChI's own list, the `METAL`/`METAL2` rows of `eldata.c`), yet the InChI changed. Molecular Inorganics only changes how bonds to metals are treated, so any count here fails a gate |
| `recmet_missing` / `recmet_failed` | no usable `-RecMet` result |
| `reconnected_as_recmet` | the InChI equals 1.07.5's `-RecMet` reconnected (`/r`) layer: Molecular Inorganics reproduces what `-RecMet` reconnected |
| `reconnected_differently` | 1.07.5 disconnected the metal (alone or along with a salt) and `-RecMet` reconnects it, but differently. Needs chemical review |
| `salt_kept_bonded` | 1.07.5 disconnected a salt, which `-RecMet` cannot undo; Molecular Inorganics keeps the bond |
| `changed_without_disconnection` | 1.07.5 broke no bond to the metal, yet the InChI changed |

The `/r` layer is the structural evidence for the stated route, and it is checked
per structure: `route_check` records whether "Metal was disconnected" comes with
an `/r` layer and "Salt was disconnected" alone without one (`agrees` /
`disagrees`), whether the baseline disconnected nothing although the structure
has a metal (`no_disconnection`), whether the message was cut off at the
library's 256-character limit before naming a route (`message_truncated`, e.g.
after a long `Charge not recognized` list), whether there is no metal and so no route
(`no_metal`), or whether there was no usable `-RecMet` result (`not_checked`).
A prefix may only change on a structure with a metal; the report's *Prefix
changes* table shows every outcome by whether the prefix changed.

`reconnected_differently` is split further by how the bonds to metals in Molecular
Inorganics' `/c` layer differ from those in the `/r` layer, counted per (metal,
partner element) pair since the two are numbered differently (`difference`
column, `identifiers.metal_bonds`). Both sides always have the same heavy atoms:

| difference | Molecular Inorganics, compared with the `/r` layer |
| --- | --- |
| `metal_hydrogen_bonded` | only has extra metal-H bonds: it writes a hydrogen on a metal as a bonded atom in `/c`, where 1.07.5 counts it in `/h` |
| `partly_disconnected` | only lacks bonds, at least one of them to a metal outside groups 1 and 2, e.g. bonds drawn between oppositely charged atoms |
| `group_1_2_disconnected` | only lacks bonds to group 1/2 metals |
| `more_bonds_kept` | only has extra bonds, not all to H, e.g. ones 1.07.5 broke as a salt |
| `mixed` | has both extra and missing bonds |
| `same_metal_bonds` | has the same bonds to metals; H, charges or the rest differ |
| `not_parsed` | the `/c` layer could not be read |

### A vs C: does the legacy handling reproduce the baseline

Pass C uses `--compare=except-prefix` (`LegacyComparator`): every stored field
must match except the prefix and the two InChIKey characters that mirror it.
`-LegacyMetalHandling` also implies `NPZz`, which pass A does not use, so every
mismatch is re-computed with the baseline and `-NPZz`:

| category | what happened |
| --- | --- |
| `explained_by_npzz` | the baseline with `-NPZz` gives Run C's result (compared with Run C's rule), e.g. a structure with star atoms that 1.07.5 rejects without `NPZz` |
| `mn7_warning_dropped` | same result except that the `Mn(7)` unusual-valence warning is gone; valence 7 was added to neutral Mn |
| `accepted_above_old_maxval` | only pass C gives an InChI, and its warning names an atom with more than 20 bonds; GHI #77 raised `MAXVAL` from 20 to 50 |
| `message_text_changed` | same result except for the wording of a V3000 message |
| `no_hydrogen_on_metal` | the baseline's InChI is Run C's plus bare `H` components: 1.07.5 gives a metal without bonds or charge implicit H and disconnects them, Run C adds no H to metals; may lack "Metal was disconnected" |
| `metal_not_disconnected` | the InChI differs, and the baseline says "Metal was disconnected" where pass C does not; fails a gate |
| `changed_otherwise` | the InChI differs for another reason; fails a gate |
| `accepted_otherwise` | only pass C gives an InChI, without an atom above the old `MAXVAL`; fails a gate |
| `unexplained` | anything else, including no `-NPZz` result; fails a gate |

The first five have a known cause: `NPZz`, or a change since 1.07.5 that the
CHANGELOG lists. The other four do not, and fail the control's gate.

As #259 expects, every A vs C difference must also occur in A vs B; a difference
only in A vs C fails a second gate. A difference in the warning alone (exit code
or message, same InChI body) is left out of that gate, since pass B does not
compare warnings.

### Output

`a_vs_b/a_vs_b_classifications.csv` carries one row per A vs B mismatch with the InChI,
InChIKey, exit code and message of the baseline, pass B and the `-RecMet`
re-run, plus the route check; `a_vs_c/a_vs_c_classifications.csv` the same for the
baseline, pass C and the `-NPZz` re-run, plus whether the molfile also mismatches
in A vs B. `a_vs_b/error_under_mi_messages.json` lists the message of every
structure that failed only in pass B. An empty `recmet_*` or `npzz_*` cell means
no result was computed for that structure.

Both comparisons write the molfile ID of each mismatch to one file per category,
so a set of structures can be fed straight into another tool:

```
a_vs_b/ids/
    changed_without_metal.txt     reconnected_as_recmet.txt
    reconnected_differently.txt   salt_kept_bonded.txt
    changed_without_disconnection.txt
    error_under_mi.txt            error_in_reference.txt
    recmet_failed.txt             recmet_missing.txt
a_vs_b/ids/reconnected_differently/
    metal_hydrogen_bonded.txt     partly_disconnected.txt
    group_1_2_disconnected.txt    more_bonds_kept.txt
    mixed.txt                     same_metal_bonds.txt
    not_parsed.txt
a_vs_c/ids/
    explained_by_npzz.txt         mn7_warning_dropped.txt
    accepted_above_old_maxval.txt message_text_changed.txt
    no_hydrogen_on_metal.txt
    metal_not_disconnected.txt    changed_otherwise.txt
    accepted_otherwise.txt        unexplained.txt
```

One ID per line, numerically sorted where the IDs are numeric. Every file is
written even when empty, so a consumer can rely on the filenames.

### Rebuilding the report

```Shell
OUT=INCHI-1-TEST/tests/test_library/data/pubchem/comparisons/molecular_inorganics/<dataset>
PYTHONPATH=INCHI-1-TEST/comparisons python -m molecular_inorganics.report \
    --comparison-dir $OUT \
    --output $OUT/report.html
```

The report is written for readers who do not work on InChI. It opens with the
findings in a few plain sentences, explains what was tested, and states every
check as a question with its result. Each kind of change gets a plain name,
an explanation and an assessment (expected, needs review, should not happen),
and a glossary at the end explains the terms. The category names, tallies,
exact rules and per-structure listings are all still there, in the
*technical details* of each part. The page has print rules, so a PDF is a
browser's *Print → Save as PDF* away, each comparison starting on a new page.

The report keeps the two comparisons strictly apart: a block for Run A, the
reference, then one part for **A vs C** (the control) and one for **A vs B**
(Molecular Inorganics). Each part has its own figures, gates, blind spots and
cost, built only from that comparison's data, and every heading in it is tagged
with its comparison.

Anything the report cannot establish from its inputs renders as *not checked* or an
em dash rather than as a passing number. Each pass's log drives its own part:
without `--run-a-log`, `--run-b-log` or `--run-c-log` that pass's aborted-SDF gate,
timeout count and cost have nothing to read. A completeness gate needs both
`--expected-structures`, the independent count it checks against, and the pass's
log, because timed-out molfiles are reference rows that never reach the
comparison. `run_comparison.sh` passes all of them; point a rebuild at the copies
in `logs/`.

## Layout

| file | role |
| --- | --- |
| `run_comparison.sh` | the whole comparison, end to end |
| `run.py` | one pass: raw results, tagged references and logs, the comparison loop |
| `comparators.py` | the comparison rules of passes B and C |
| `identifiers.py` | parts of an InChI and an InChIKey, and the metal list |
| `consumers.py` | `raw_regression_consumer`: InChI, key, exit code and message, unnormalised |
| `classify.py` | re-computes and classifies the mismatches of both comparisons |
| `report.py` | renders `report.html` from `classify.py`'s output |
| `tests/` | `pytest INCHI-1-TEST/comparisons/molecular_inorganics/tests` |

The package uses `sdf_pipeline` (`core.run`, the reference driver) and
`inchi_tests` (the ctypes binding) and adds nothing to either.
