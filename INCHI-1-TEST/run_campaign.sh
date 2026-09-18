#!/bin/bash
#
# Run the MolecularInorganics regression campaign end to end.
#
#   ./INCHI-1-TEST/run_campaign.sh <dataset> [--skip-download]
#
# <dataset> is "compound", "compound3d" or "substance". SDFs and per-shard
# references live in .../data/pubchem/<dataset>/; every output of the run --
# report, classifications, per-cause ID lists and copies of the run logs --
# goes to .../data/pubchem/campaign/<dataset>/.
#
# The campaign compares three passes over the same SDFs:
#
#   A  baseline (a released tag), no options   -- the reference
#   B  working tree, -MolecularInorganics      -- what the option changes,
#                                                 compared ignoring the prefix
#   C  working tree, no options                -- the control, compared
#                                                 byte-for-byte against A
#
# and re-checks every A-vs-B mismatch against the baseline's -RecMet output.
#
# Safe to re-run: the venv, the worktree and the per-shard references are all
# reused rather than rebuilt. Re-running after an aborted reference shard picks
# up exactly the shards that are missing.
#
# Environment overrides:
#   BASELINE_TAG   git tag to compare against          (default v1.07.5)
#   RUN_TAG        namespace for reference files       (default ref_<tag>)

set -euo pipefail
shopt -s nullglob

DATASET="${1:-}"
SKIP_DOWNLOAD="${2:-}"

if [[ ! "$DATASET" =~ ^(compound|compound3d|substance)$ ]]; then
    echo "usage: $0 <compound|compound3d|substance> [--skip-download]" >&2
    exit 2
fi

BASELINE_TAG="${BASELINE_TAG:-v1.07.5}"
RUN_TAG="${RUN_TAG:-ref_${BASELINE_TAG//[.-]/_}}"
REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
WORKTREE="$REPO/../inchi-${BASELINE_TAG}"
DATA="$REPO/INCHI-1-TEST/tests/test_library/data/pubchem/$DATASET"
CONFIG="$REPO/INCHI-1-TEST/tests/test_library/config/config_pubchem_${DATASET}.py"
OUT="$REPO/INCHI-1-TEST/tests/test_library/data/pubchem/campaign/$DATASET"
LIB_REL="CMake_build/full_build/INCHI-1-SRC/INCHI_API/libinchi/src/lib/libinchi.so"
PY="$REPO/.venv/bin/python"
TESTS="$REPO/INCHI-1-TEST/tests/test_library/inchi_tests"

cd "$REPO"

step() { printf '\n\033[1m=== %s ===\033[0m\n' "$1"; }
die()  { printf '\n\033[31mFAILED: %s\033[0m\n' "$1" >&2; exit 1; }

# Newest log for a given tag, or empty. Avoids `ls | head`, whose exit status
# under pipefail depends on whether the glob matched.
newest_log() {
    local logs=("$DATA"/*"$1".log)
    [[ ${#logs[@]} -eq 0 ]] && return 0
    local newest="" candidate
    for candidate in "${logs[@]}"; do
        [[ -z "$newest" || "$candidate" -nt "$newest" ]] && newest="$candidate"
    done
    printf '%s' "$newest"
}

# run_tests.py exits 1 whenever it logs a mismatch, which is not a script error.
run_pass() {
    local test=$1 lib=$2 log_tag=$3 compare=$4 options=${5:-}
    local args=(--test="$test" --lib-path="$lib" --data-config="$CONFIG"
                --run-tag="$RUN_TAG" --log-tag="$log_tag" --compare="$compare")
    [[ -n "$options" ]] && args+=(--inchi-api-parameters="$options")

    # run_tests.py opens its log only after argparse and the data-config import
    # have succeeded, so a pass that dies before that leaves no log at all --
    # and newest_log would then hand back an earlier campaign's log for this tag,
    # which greps clean and gets classified as if it were this run's. Comparing
    # against the logs that existed beforehand catches that without depending on
    # filesystem timestamp granularity.
    local before=("$DATA"/*"$log_tag".log)
    "$PY" "$TESTS/run_tests.py" "${args[@]}" || true

    local log candidate
    log=$(newest_log "$log_tag")
    [[ -n "$log" ]] || die "$log_tag produced no log"
    for candidate in "${before[@]}"; do
        [[ "$candidate" == "$log" ]] && die \
            "$log_tag produced no log this run; newest is from an earlier run ($log)"
    done
    local aborted
    aborted=$(grep -c "Aborted " "$log" || true)
    # An aborted shard is logged and the run continues, so it never shows up in
    # the exit code. A regression pass that aborted has partial results that
    # still look well-formed, which is worse than none.
    [[ "$aborted" -eq 0 ]] || die "$log_tag: $aborted shard(s) aborted, see $log"
    echo "$log"
}

step "1/8  Python environment"
if [[ ! -x "$PY" ]]; then
    python3 -m venv "$REPO/.venv"
    "$REPO/.venv/bin/pip" install --quiet --upgrade pip
fi
"$REPO/.venv/bin/pip" install --quiet -e INCHI-1-TEST
"$PY" -c "import pydantic, sdf_pipeline, inchi_tests" || die "environment incomplete"
echo "ok: $($PY --version)"

step "2/8  Build the working tree (Release)"
# build_with_cmake.sh sets no CMAKE_BUILD_TYPE, and libinchi gates -O1 on
# Release, so configure explicitly or the runs take several times longer.
cmake -B CMake_build/full_build -S . -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=ON > /dev/null
cmake --build CMake_build/full_build > /dev/null
[[ -f "$REPO/$LIB_REL" ]] || die "working-tree libinchi.so not built"
echo "ok: $(git rev-parse --short HEAD)"

step "3/8  Build the $BASELINE_TAG baseline (Release)"
if [[ ! -d "$WORKTREE" ]]; then
    git worktree add "$WORKTREE" "$BASELINE_TAG"
fi
( cd "$WORKTREE"
  cmake -B CMake_build/full_build -S . -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON > /dev/null
  cmake --build CMake_build/full_build > /dev/null )
[[ -f "$WORKTREE/$LIB_REL" ]] || die "baseline libinchi.so not built"
BASELINE_LIB="$WORKTREE/$LIB_REL"
DEV_LIB="$REPO/$LIB_REL"
echo "ok: $BASELINE_TAG at $(git -C "$WORKTREE" rev-parse --short HEAD)"

step "4/8  Mirror and verify $DATASET"
if [[ "$SKIP_DOWNLOAD" == "--skip-download" ]]; then
    echo "skipping download on request"
else
    # Must complete before any run starts: wget writes in place, and a
    # half-written .sdf.gz aborts that shard.
    "$PY" -m INCHI-1-TEST.tests.test_library.data.pubchem.download "$DATASET"
fi
sdfs=("$DATA"/*.sdf.gz)
shards=${#sdfs[@]}
[[ "$shards" -gt 0 ]] || die "no SDFs in $DATA"
if [[ "$DATASET" == "compound3d" ]]; then
    echo "ok: $shards shards (PubChem publishes no hashes for compound3d)"
else
    # validate.py prints mismatches and always exits 0, so the grep is the gate.
    bad=$("$PY" -m INCHI-1-TEST.tests.test_library.data.pubchem.validate "$DATASET" \
          | grep -c "does not match server hash" || true)
    [[ "$bad" -eq 0 ]] || die "$bad shard(s) failed md5 — re-download before running"
    echo "ok: $shards shards, all md5-verified"
fi

step "5/8  Run A — $BASELINE_TAG reference"
run_a_log=$(run_pass regression-reference "$BASELINE_LIB" run_a_ref exact)
partials=("$DATA"/*.partial)
[[ ${#partials[@]} -eq 0 ]] || die "${#partials[@]} incomplete reference(s) left behind"
rows=$("$PY" - "$DATA" "$RUN_TAG" <<'PYEOF'
import glob, sqlite3, sys
total = 0
for path in glob.glob(f"{sys.argv[1]}/*.{sys.argv[2]}.regression_reference.sqlite"):
    with sqlite3.connect(path) as db:
        total += db.execute("SELECT COUNT(*) FROM results").fetchone()[0]
print(total)
PYEOF
)
echo "ok: $rows reference rows"

step "6/8  Run B — working tree, -MolecularInorganics"
run_b_log=$(run_pass regression "$DEV_LIB" run_b_dev_mi prefix-insensitive -MolecularInorganics)
echo "ok: $(grep -c 'regression test failed:' "$run_b_log" || true) mismatches"

step "7/8  Run C — working tree, no options"
run_c_log=$(run_pass regression "$DEV_LIB" run_c_dev_plain exact)
run_c_mismatches=$(grep -c 'regression test failed:' "$run_c_log" || true)
if [[ "$run_c_mismatches" -ne 0 ]]; then
    printf '\n\033[31m*** %s Run C mismatches ***\033[0m\n' "$run_c_mismatches"
    echo "The working tree does not reproduce $BASELINE_TAG without options."
    echo "Run B's differences are NOT attributable to -MolecularInorganics until"
    echo "this is explained. See $run_c_log"
else
    echo "ok: 0 mismatches — the working tree reproduces $BASELINE_TAG exactly"
fi

step "8/8  Classify and report"
"$PY" "$TESTS/campaign.py" \
    --regression-log="$run_b_log" \
    --recmet-lib-path="$BASELINE_LIB" \
    --mi-lib-path="$DEV_LIB" \
    --data-config="$CONFIG" \
    --output="$OUT"
"$PY" "$TESTS/report.py" \
    --classifications="$OUT/classifications.csv" \
    --summary="$OUT/summary.json" \
    --baseline-label="$BASELINE_TAG · $(git -C "$WORKTREE" rev-parse --short HEAD)" \
    --test-label="$(git rev-parse --abbrev-ref HEAD) · $(git rev-parse --short HEAD)" \
    --run-a-log="$run_a_log" --run-b-log="$run_b_log" --run-c-log="$run_c_log" \
    --output="$OUT/report.html"

# Copy the run logs in so the output folder stands alone when archived or moved.
mkdir -p "$OUT/logs"
cp "$run_a_log" "$run_b_log" "$run_c_log" "$OUT/logs/"

printf '\n\033[1mCampaign complete.\033[0m\n'
echo "  everything in  $OUT"
echo "    report.html          the rendered report"
echo "    classifications.csv  one row per mismatch"
echo "    ids/                 mismatching IDs, one file per cause"
echo "    summary.json         counts and tallies"
echo "    logs/                the three run logs"
