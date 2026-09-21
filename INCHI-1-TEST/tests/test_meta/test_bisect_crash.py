"""`bisect_crash.py` names the record that kills the library.

Driven against a stub shared library that segfaults on a marked molfile, so the
crash path is exercised for real rather than simulated.
"""

import gzip
import shutil
import subprocess
import sys
from pathlib import Path

import pytest

REPO = Path(__file__).resolve().parents[3]
SCRIPT = REPO / "INCHI-1-TEST/tests/test_library/inchi_tests/bisect_crash.py"
ENVIRONMENT_PATHS = [
    str(REPO / "INCHI-1-TEST/src"),
    str(REPO / "INCHI-1-TEST/tests/test_library"),
]

STUB_SOURCE = r"""
#include <string.h>

typedef struct {
    char *szInChI;
    char *szAuxInfo;
    char *szMessage;
    char *szLog;
} InChIOutput;

int MakeINCHIFromMolfileText(const char *molfile, const char *options,
                             InChIOutput *output)
{
    (void) options;
    (void) output;
    if (strstr(molfile, "CRASHME") != NULL) {
        volatile int *null_pointer = 0;
        *null_pointer = 1;  /* the segfault under test */
    }
    return 0;
}

void FreeINCHI(InChIOutput *output)
{
    (void) output;
}
"""


@pytest.fixture
def stub_library(tmp_path):
    if not shutil.which("gcc"):
        pytest.skip("gcc not available to build the stub library")
    source = tmp_path / "stub.c"
    source.write_text(STUB_SOURCE, encoding="utf-8")
    library = tmp_path / "libstub.so"
    subprocess.run(
        ["gcc", "-shared", "-fPIC", "-o", str(library), str(source)], check=True
    )

    return library


@pytest.fixture
def sdf(tmp_path):
    def _write(crashing_id: str | None, ids: list[str]):
        records = []
        for molfile_id in ids:
            marker = "CRASHME" if molfile_id == crashing_id else "ordinary"
            records.append(
                f"{molfile_id}\n  test\n  {marker}\n"
                "  0  0  0  0  0  0  0  0  0  0999 V2000\nM  END\n$$$$\n"
            )
        path = tmp_path / "shard.sdf.gz"
        with gzip.open(path, "wt", encoding="utf-8") as handle:
            handle.write("".join(records))
        return path

    return _write


def _run(script_args, tmp_path):
    return subprocess.run(
        [sys.executable, str(SCRIPT), *script_args],
        capture_output=True,
        text=True,
        cwd=tmp_path,
        env={"PYTHONPATH": ":".join(ENVIRONMENT_PATHS), "PATH": "/usr/bin:/bin"},
    )


def test_checkpoint_names_the_crashing_record(stub_library, sdf, tmp_path):
    """In-process: the parent dies, and the ID must survive on disk."""
    path = sdf("300", ["100", "200", "300", "400"])
    checkpoint = tmp_path / "cp.txt"

    result = _run(
        [
            f"--sdf-path={path}",
            f"--lib-path={stub_library}",
            f"--checkpoint={checkpoint}",
        ],
        tmp_path,
    )

    # Killed by SIGSEGV, exactly as a consumer process would be.
    assert result.returncode < 0, result.stdout
    assert checkpoint.read_text().split("\t")[0] == "300"


def test_timeout_mode_reports_the_crashing_record(stub_library, sdf, tmp_path):
    """Isolated per record: the walk reports the ID instead of dying with it."""
    path = sdf("200", ["100", "200", "300"])
    checkpoint = tmp_path / "cp.txt"

    result = _run(
        [
            f"--sdf-path={path}",
            f"--lib-path={stub_library}",
            f"--checkpoint={checkpoint}",
            "--timeout=30",
        ],
        tmp_path,
    )

    assert result.returncode == 1
    assert "CRASH" in result.stdout
    assert "molfile ID 200" in result.stdout
    assert checkpoint.read_text().split("\t")[0] == "200"


def test_a_clean_shard_completes_and_clears_the_checkpoint(stub_library, sdf, tmp_path):
    path = sdf(None, ["100", "200", "300"])
    checkpoint = tmp_path / "cp.txt"

    result = _run(
        [
            f"--sdf-path={path}",
            f"--lib-path={stub_library}",
            f"--checkpoint={checkpoint}",
        ],
        tmp_path,
    )

    assert result.returncode == 0, result.stderr
    assert "Completed 3 records" in result.stdout
    # No stale ID left behind to incriminate an innocent record.
    assert not checkpoint.exists()


def test_start_after_resumes_past_a_known_offender(stub_library, sdf, tmp_path):
    """Two crashing records: skipping the first must reach the second."""
    path = sdf(None, ["100", "200", "300"])
    # Rewrite so that both 100 and 300 crash.
    records = []
    for molfile_id in ["100", "200", "300"]:
        marker = "CRASHME" if molfile_id in ("100", "300") else "ordinary"
        records.append(
            f"{molfile_id}\n  test\n  {marker}\n"
            "  0  0  0  0  0  0  0  0  0  0999 V2000\nM  END\n$$$$\n"
        )
    with gzip.open(path, "wt", encoding="utf-8") as handle:
        handle.write("".join(records))

    checkpoint = tmp_path / "cp.txt"
    result = _run(
        [
            f"--sdf-path={path}",
            f"--lib-path={stub_library}",
            f"--checkpoint={checkpoint}",
            "--start-after=100",
            "--timeout=30",
        ],
        tmp_path,
    )

    assert result.returncode == 1
    assert "molfile ID 300" in result.stdout
