"""Find the molfile in an SDF that crashes the library or exceeds a time limit.

`sdf_pipeline` processes an SDF's records in several consumer processes. When the
library crashes one of them, `core.run` can only report

    RuntimeError: could not process <SDF>: A process terminated unexpectedly.

because the molfile the crashed process was working on is lost with it.

This script processes one SDF's records one at a time in a single process. Before
it passes a molfile to the library, it writes the molfile ID to a checkpoint file
and flushes it to disk. After a crash, the checkpoint file holds the ID of the
molfile that caused it. After a run without a crash, the checkpoint file is
deleted.

    python inchi_tests/find_failing_molfile.py \\
        --sdf-path=.../Substance_141000001_141500000.sdf.gz \\
        --lib-path=.../libinchi.so

The SDF path is a PubChem Substance SDF; the IDs are PubChem substance record IDs
(SIDs). A crash ends the run, so to find a second crashing molfile, skip past the
first:

    ... --start-after=141261563

With `--timeout`, each molfile is processed in a child process, which is stopped
if it exceeds the time limit; the run then continues with the next molfile. This
is much slower, but it also finds molfiles that exceed the time limit and does
not end the run at the first crash.
"""

import argparse
import ctypes
import multiprocessing
import os
import sys
from pathlib import Path

from sdf_pipeline.utils import read_records_from_gzipped_sdf
from inchi_tests.inchi_api import make_inchi_from_molfile_text
from inchi_tests.utils import get_molfile_id_pubchem


def _compute(molfile: str, inchi_lib_path: str, inchi_api_parameters: str) -> None:
    """Make the library call that a consumer makes for each molfile."""
    inchi_lib = ctypes.CDLL(inchi_lib_path)
    make_inchi_from_molfile_text(inchi_lib, molfile, inchi_api_parameters)


def _checkpoint(path: Path, molfile_id: str, index: int) -> None:
    # Written and flushed to disk before the library call, because a crash
    # (e.g., SIGSEGV) ends the process without running any further Python code.
    with open(path, "w", encoding="utf-8") as checkpoint_file:
        checkpoint_file.write(f"{molfile_id}\t{index}\n")
        checkpoint_file.flush()
        os.fsync(checkpoint_file.fileno())


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Process one SDF one molfile at a time to find the molfile that crashes the library or exceeds a time limit."
    )
    parser.add_argument("--sdf-path", required=True, type=Path)
    parser.add_argument("--lib-path", required=True, type=str)
    parser.add_argument("--inchi-api-parameters", default="", type=str)
    parser.add_argument(
        "--start-after",
        default="",
        type=str,
        help="Skip molfiles up to and including this molfile ID, e.g., one that is known to crash.",
    )
    parser.add_argument(
        "--timeout",
        default=0.0,
        type=float,
        help="Seconds allowed per molfile; 0 (default) processes every molfile in this "
        "process without a time limit. A non-zero value processes each molfile in a "
        "child process, which is much slower but also finds molfiles that exceed "
        "the time limit.",
    )
    parser.add_argument(
        "--checkpoint",
        default=Path("bisect_crash.checkpoint"),
        type=Path,
        help="File the current molfile ID is written to before each library call.",
    )
    parser.add_argument(
        "--report-every",
        default=10000,
        type=int,
        help="Print progress every N molfiles.",
    )
    args = parser.parse_args()

    skipping = bool(args.start_after)
    processed = 0
    over_time_limit = []

    print(f"Processing {args.sdf_path.name} with '{args.lib_path}'.", flush=True)
    if skipping:
        print(f"Skipping until past molfile ID {args.start_after}.", flush=True)

    for index, molfile in enumerate(read_records_from_gzipped_sdf(args.sdf_path)):
        molfile_id = get_molfile_id_pubchem(molfile)

        if skipping:
            if molfile_id == args.start_after:
                skipping = False
            continue

        _checkpoint(args.checkpoint, molfile_id, index)

        if args.timeout:
            worker = multiprocessing.Process(
                target=_compute,
                args=(molfile, args.lib_path, args.inchi_api_parameters),
            )
            worker.start()
            worker.join(args.timeout)
            if worker.is_alive():
                worker.kill()
                worker.join()
                over_time_limit.append((molfile_id, index))
                print(
                    f"TIME LIMIT  record {index}, molfile ID {molfile_id}: "
                    f"exceeded {args.timeout}s",
                    flush=True,
                )
            elif worker.exitcode != 0:
                # The library crashed the child process. Report the molfile and
                # stop, as this process would have crashed without `--timeout`.
                print(
                    f"\nCRASH record {index}, molfile ID {molfile_id}: "
                    f"child exited with {worker.exitcode}",
                    flush=True,
                )
                raise SystemExit(1)
        else:
            _compute(molfile, args.lib_path, args.inchi_api_parameters)

        processed += 1
        if processed % args.report_every == 0:
            print(f"  {processed} records, last ID {molfile_id}", flush=True)

    print(f"\nCompleted {processed} records without a crash.", flush=True)
    if over_time_limit:
        print(f"{len(over_time_limit)} record(s) exceeded the time limit:", flush=True)
        for molfile_id, index in over_time_limit:
            print(f"  record {index}, molfile ID {molfile_id}", flush=True)
        raise SystemExit(1)

    # No crash, so remove the checkpoint file: its ID would suggest one.
    args.checkpoint.unlink(missing_ok=True)


if __name__ == "__main__":
    multiprocessing.set_start_method("spawn")
    main()
