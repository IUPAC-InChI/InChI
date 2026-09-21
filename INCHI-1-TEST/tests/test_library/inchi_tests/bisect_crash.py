"""Find the molfile that kills a consumer process.

`sdf_pipeline` fans records across worker processes, so a structure that
segfaults or hangs libinchi surfaces only as

    RuntimeError: could not process <shard>: A process terminated unexpectedly.

with no indication of which record did it -- the worker takes its state to the
grave and the records it had in flight are unknowable.

This walks one shard in a single process instead, writing the current molfile ID
to a checkpoint file and flushing it to disk *before* handing the record to the
library. A hard crash therefore leaves the culprit's ID on disk. On a clean exit
the shard is exonerated.

    python inchi_tests/bisect_crash.py \\
        --sdf-path=.../Substance_141000001_141500000.sdf.gz \\
        --lib-path=.../libinchi.so

The run dies where the shard dies, so to find a second offender, resume past the
first:

    ... --start-after=141261563

`--timeout` catches a hang rather than a crash: the record is processed in a
child process that is killed if it overruns, and the walk continues. Without it
an infinite loop in the library looks like no progress at all.
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
    """Exactly what the campaign consumer asks of the library, nothing more."""
    inchi_lib = ctypes.CDLL(inchi_lib_path)
    make_inchi_from_molfile_text(inchi_lib, molfile, inchi_api_parameters)


def _checkpoint(path: Path, molfile_id: str, index: int) -> None:
    # Written and fsynced before the call, so it survives a SIGSEGV that gives
    # the interpreter no chance to run anything.
    with open(path, "w", encoding="utf-8") as checkpoint_file:
        checkpoint_file.write(f"{molfile_id}\t{index}\n")
        checkpoint_file.flush()
        os.fsync(checkpoint_file.fileno())


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Walk one SDF in-process to find the record that kills the library."
    )
    parser.add_argument("--sdf-path", required=True, type=Path)
    parser.add_argument("--lib-path", required=True, type=str)
    parser.add_argument("--inchi-api-parameters", default="", type=str)
    parser.add_argument(
        "--start-after",
        default="",
        type=str,
        help="Skip records up to and including this molfile ID, to resume past a known offender.",
    )
    parser.add_argument(
        "--timeout",
        default=0.0,
        type=float,
        help="Seconds to allow per record; 0 runs in-process with no timeout. "
        "A non-zero value isolates each record in a child process, which is far "
        "slower but catches a hang as well as a crash.",
    )
    parser.add_argument(
        "--checkpoint",
        default=Path("bisect_crash.checkpoint"),
        type=Path,
        help="Where the current molfile ID is recorded before each call.",
    )
    parser.add_argument(
        "--report-every",
        default=10000,
        type=int,
        help="Print progress every N records.",
    )
    args = parser.parse_args()

    skipping = bool(args.start_after)
    processed = 0
    hangs = []

    print(f"Walking {args.sdf_path.name} with '{args.lib_path}'.", flush=True)
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
                hangs.append((molfile_id, index))
                print(
                    f"HANG  record {index}, molfile ID {molfile_id}: "
                    f"exceeded {args.timeout}s",
                    flush=True,
                )
            elif worker.exitcode != 0:
                # The child died where this process would have. Report and stop:
                # the ID is the answer the campaign log could not give.
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

    print(f"\nCompleted {processed} records without dying.", flush=True)
    if hangs:
        print(f"{len(hangs)} record(s) exceeded the timeout:", flush=True)
        for molfile_id, index in hangs:
            print(f"  record {index}, molfile ID {molfile_id}", flush=True)
        raise SystemExit(1)

    # Nothing to blame, so leave no stale ID behind to mislead the next reader.
    args.checkpoint.unlink(missing_ok=True)


if __name__ == "__main__":
    multiprocessing.set_start_method("spawn")
    main()
