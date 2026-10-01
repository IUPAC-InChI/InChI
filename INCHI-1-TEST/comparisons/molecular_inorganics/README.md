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

The passes therefore store the raw result (full InChI, key, exit code) and decide
at comparison time what counts as a match. Pass B uses
`--compare=prefix-insensitive`, i.e. `inchi_tests.comparators.PrefixInsensitiveComparator`:
it compares the InChI *body* plus a failure flag, where failure means `exit >= 2`
or an empty InChI. Exit code 1 is a warning, and a metal disconnection always
warns, so treating it as failure would misclassify most of the structures of
interest. The differences it ignores are counted and logged as `prefix_only`,
`key_only`, `warning_only`, `both_failed` and `failure_kind_only` — the last being
two passes that both failed on the same body with different error codes.

Pass C reads the same tagged reference as pass B but keeps `--compare=exact`. A
prefix-insensitive pass C could not see a changed prefix, InChIKey or warning
level, which is exactly the version drift it exists to catch.

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
- `error_under_mi` / `error_in_reference` / `recmet_failed` / `recmet_missing` —
  one side produced no InChI.

`classifications.csv` carries one row per mismatch with the InChI, InChIKey and
exit code of all three sides — baseline, the option, and the `-RecMet` re-check —
so it can be queried without going back to the logs. An empty `recmet_*` cell
means no re-computation was made for that structure.

### Mismatch IDs per cause

Every run writes the molfile ID of each mismatch to one file per cause, so a set
of structures can be fed straight into another tool:

```
ids/
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

Anything the report cannot establish from its inputs renders as *not checked* or an
em dash rather than as a passing number: without `--run-c-log` the control is not
measured, without `--run-a/b/c-log` the aborted-shard gate has nothing to read, and
without `--expected-structures` the completeness gate has no independent count to
check the comparison against. `run_comparison.sh` passes all four; point a rebuild
at the copies in `logs/`.

## Layout

| file | role |
| --- | --- |
| `run_comparison.sh` | the whole comparison, end to end |
| `run.py` | one pass: raw results, tagged references and logs, optional prefix-insensitive comparison |
| `consumers.py` | `raw_regression_consumer`: InChI, key and exit code, unnormalised |
| `classify.py` | parses pass B's log, re-computes mismatches with `-RecMet`, classifies them |
| `report.py` | renders `report.html` from `classify.py`'s output |
| `tests/` | `pytest INCHI-1-TEST/comparisons/molecular_inorganics/tests` |

The package reuses `sdf_pipeline` and `inchi_tests` (the ctypes binding, the
comparator, `inchi_body`/`is_failed`) and adds nothing to either.
