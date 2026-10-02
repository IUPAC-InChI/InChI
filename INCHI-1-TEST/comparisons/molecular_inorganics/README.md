# MolecularInorganics comparison

What does `-MolecularInorganics` change relative to a released InChI version?
This directory answers that question over a whole PubChem dataset. It is a
one-off comparison, not part of the test suite: nothing here runs in CI, and the
shared harness under `INCHI-1-TEST/tests/test_library` knows nothing about it.

It runs three passes over the same SDFs and re-checks every difference:

| pass | library | options | compared | purpose |
| --- | --- | --- | --- | --- |
| A | baseline tag | none | — | the reference |
| B | working tree | `-MolecularInorganics` | InChI body only | what the option changes |
| C | working tree | none | byte-for-byte | the control: must reproduce A exactly |

## Run it

From the repository root:

```Shell
./INCHI-1-TEST/comparisons/molecular_inorganics/run_comparison.sh <compound|compound3d|substance>
```

The script creates a virtual environment, builds both libraries in Release mode
(`build_with_cmake.sh` sets no `CMAKE_BUILD_TYPE`, and `libinchi` gates `-O1` on
Release, so an unconfigured build is several times slower), adds a git worktree
for the baseline tag, mirrors and md5-verifies the dataset, runs all three
passes, classifies the differences and writes everything to
`INCHI-1-TEST/tests/test_library/data/pubchem/comparisons/molecular_inorganics/<dataset>/`:

| path | contents |
| --- | --- |
| `report.html` | the rendered report |
| `classifications.csv` | one row per mismatch |
| `ids/` | mismatching IDs, one file per cause |
| `summary.json` | counts and tallies |
| `logs/` | the three pass logs |

Re-running is safe: the environment, the worktree and the per-shard references
are reused, so an interrupted reference pass resumes at the shards it is missing.
Pass `--skip-download` to re-run against data already on disk. `BASELINE_TAG`
(default `v1.07.5`) and `RUN_TAG` are environment overrides.

The script fails loudly on the conditions that otherwise pass silently: a shard
whose md5 does not match (`validate.py` only prints these and always exits 0), a
shard that aborted mid-pass (`run.py` logs `Aborted …` and continues, so it never
reaches the exit code), and a leftover `.partial` reference. A non-zero pass C is
reported prominently, because it means pass B's differences cannot be attributed
to the option.

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
| `comparison_worktree` | `/comparison/worktree` | the baseline worktree would otherwise sit outside the mount and be lost on exit |
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
prunes stale registrations before adding one.

## How the comparison works

Comparing raw InChI strings across option sets does not work. `-MolecularInorganics`
sets `is_beta` from the option alone (`ichiprt1.c:1678`), so *every* structure
changes prefix (`InChI=1S/` to `InChI=1B/`) and InChIKey flag — metal-free
organics included — and a byte comparison would mark an entire dataset as changed.

The passes therefore store the raw result (full InChI, key, exit code and the
warning `message`) and decide at comparison time what counts as a match. `aux` and
`log` are not stored; they would dominate reference size at PubChem scale. A
reference storing any other set of fields, e.g. one written before `message` was
added, is refused with an `Aborted` line naming the file: delete it and the
reference pass recomputes it.

Pass B uses `--compare=prefix-insensitive`, i.e. `run.MessageTallyingComparator`,
which is `inchi_tests.comparators.PrefixInsensitiveComparator` plus one tally:
it compares the InChI *body* plus a failure flag, where failure means `exit >= 2`
or an empty InChI. Exit code 1 is a warning, and a metal disconnection always
warns, so treating it as failure would misclassify most of the structures of
interest. The differences it ignores are counted and logged as `prefix_only`,
`key_only`, `warning_only`, `message_only`, `metal_free_warning_only`,
`metal_free_message_only`, `both_failed` and `failure_kind_only` —
the last being two passes that both failed on the same body with different error
codes.

Pass C reads the same tagged reference as pass B but keeps `--compare=exact`,
byte-for-byte on all four stored fields. A prefix-insensitive pass C could not see
a changed prefix, InChIKey, warning level or warning text, which is exactly the
version drift it exists to catch.

References are namespaced by `--run-tag` (`<shard>.<tag>.regression_reference.sqlite`)
and logs by `--log-tag`, so the comparison never reads or overwrites the
committed references the regression tests use.

Differences are then classified against the baseline's `-RecMet` output. The old
code breaks bonds to metals by two routes and `-RecMet` only reverses one of
them, so:

- `recmet_equivalent` — the option's InChI equals the `-RecMet` reconnected
  (`/r`) layer: the old code disconnected the metal and `-RecMet` put it back.
- `novel` with an `/r` layer — both reconnect, and disagree.
- `novel` without one — the old code used salt disconnection, which `-RecMet`
  cannot undo.
- `metal_free` — neither side's formula contains a metal (InChI's own list, the
  `METAL`/`METAL2` rows of `eldata.c`). The option only changes how bonds to
  metals are treated, so it has no mechanism to change these; any count here is
  unexpected and fails a gate in the report. Checked after the error categories
  and before the `-RecMet` comparison. A metal-free structure whose InChI matched
  but whose warning changed is counted by pass B as `metal_free_warning_only` /
  `metal_free_message_only` and fails a second gate.
- `error_under_mi` / `error_in_reference` / `recmet_failed` / `recmet_missing` —
  one side produced no InChI.

The `/r` layer is only a proxy for the route the old code took, so it is checked
per structure: the `-RecMet` re-run stores the baseline's warning message, and
`route_check` records whether "Metal was disconnected" comes with an `/r` layer
and "Salt was disconnected" alone without one (`agrees` / `disagrees`), whether
the baseline disconnected nothing although the structure has a metal
(`no_disconnection`), whether there is no metal and so no route (`no_metal`, the
`metal_free` rows), or whether there was no usable `-RecMet` result
(`not_checked`).

`classifications.csv` carries one row per mismatch with the InChI, InChIKey, exit
code and message of all three sides — baseline, the option, and the `-RecMet`
re-check — plus the route check, so it can be queried without going back to the
logs. `error_under_mi_messages.json` lists the message of every structure that
failed only under the option. An empty `recmet_*` cell means no re-computation was made
for that structure.

### Mismatch IDs per cause

Every run writes the molfile ID of each mismatch to one file per cause, so a set
of structures can be fed straight into another tool:

```
ids/
    metal_free.txt
    recmet_equivalent.txt     novel_metal_pathway.txt   novel_salt_pathway.txt
    error_under_mi.txt        error_in_reference.txt
    recmet_failed.txt         recmet_missing.txt
```

One ID per line, numerically sorted where the IDs are numeric. `novel` is split by
disconnection pathway, which is the distinction that matters when following one
up. Every file is written even when empty, so a consumer can rely on the
filenames.

### Rebuilding the report

```Shell
OUT=INCHI-1-TEST/tests/test_library/data/pubchem/comparisons/molecular_inorganics/<dataset>
PYTHONPATH=INCHI-1-TEST/comparisons python -m molecular_inorganics.report \
    --classifications $OUT/classifications.csv \
    --summary $OUT/summary.json \
    --output $OUT/report.html
```

The report keeps the two comparisons strictly apart: a block for Run A, the
reference, then one part for **A vs C** (the control, byte-for-byte) and one for
**A vs B** (the option, prefix-insensitive). Each part has its own figures, gates,
blind spots and cost, built only from that pass's data, and every heading in it
is tagged with its comparison.

Anything the report cannot establish from its inputs renders as *not checked* or an
em dash rather than as a passing number. Each pass's log drives its own part:
without `--run-c-log` the control is not measured at all, and without
`--run-a-log` or `--run-b-log` that pass's aborted-shard gate and timeout count
have nothing to read. A completeness gate needs both `--expected-structures`, the
independent count it checks against, and the pass's log, because timed-out
molfiles are reference rows that never reach the comparator. `run_comparison.sh`
passes all of them; point a rebuild at the copies in `logs/`.

## Layout

| file | role |
| --- | --- |
| `run_comparison.sh` | the whole comparison, end to end |
| `run.py` | one pass: raw results, tagged references and logs, optional prefix-insensitive comparison |
| `consumers.py` | `raw_regression_consumer`: InChI, key, exit code and message, unnormalised |
| `classify.py` | parses pass B's log, re-computes mismatches with `-RecMet`, classifies them |
| `report.py` | renders `report.html` from `classify.py`'s output |
| `tests/` | `pytest INCHI-1-TEST/comparisons/molecular_inorganics/tests` |

The package reuses `sdf_pipeline` and `inchi_tests` (the ctypes binding, the
comparator, `inchi_body`/`is_failed`) and adds nothing to either.
