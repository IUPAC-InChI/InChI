import ctypes
import multiprocessing
import time
from dataclasses import dataclass
from queue import Empty
from typing import Callable, TYPE_CHECKING
from collections.abc import Generator
from pathlib import Path
from sdf_pipeline import logger, utils

# Room for a molfile ID in shared memory; PubChem's are far shorter.
_MOLFILE_ID_BYTES = 256

if TYPE_CHECKING:
    # https://adamj.eu/tech/2021/05/13/python-type-hints-how-to-fix-circular-imports/
    from sdf_pipeline.drivers import ConsumerResult


def _produce_molfiles(
    molfile_queue: multiprocessing.Queue, sdf_path: Path, n_poison_pills: int
) -> None:
    for molfile in utils.read_records_from_gzipped_sdf(sdf_path):
        molfile_queue.put(molfile)

    for _ in range(n_poison_pills):
        # Poison pill: tell consumer processes we're done.
        molfile_queue.put("DONE")


@dataclass(frozen=True)
class TimedOut:
    """One molfile the consumer never came back from.

    Yielded in place of a result so the caller can record the molfile and carry
    on with the rest of the SDF. A consumer's work is opaque C in the usual case,
    which no in-process timer can interrupt, so the only way to bound it is to
    kill the worker -- hence this rather than an exception from the consumer."""

    molfile_id: str
    seconds: float


def _consume_molfiles(
    molfile_queue: multiprocessing.Queue,
    result_queue: multiprocessing.Queue,
    process_id: int,
    consumer_function: Callable,
    get_molfile_id: Callable | None = None,
    current_molfile_id: "multiprocessing.Array | None" = None,  # type: ignore[valid-type]
    started_at: "multiprocessing.Value | None" = None,  # type: ignore[valid-type]
) -> None:
    for molfile in iter(molfile_queue.get, "DONE"):
        if get_molfile_id is not None:
            # Publish what we are about to work on, so a parent that stops
            # hearing from us can say which molfile we are stuck on. Shared
            # memory, so this costs no round trip per record.
            current_molfile_id.value = get_molfile_id(molfile).encode("utf-8")[
                : _MOLFILE_ID_BYTES - 1
            ]
            started_at.value = time.monotonic()
        result_queue.put(consumer_function(molfile))

    result_queue.put(process_id)


def run(
    sdf_path: Path,
    consumer_function: Callable,
    number_of_consumer_processes: int,
    timeout_seconds_per_molfile: int = 60,
    get_molfile_id: Callable | None = None,
) -> Generator["ConsumerResult", None, None]:
    """Fan the SDF's records across consumer processes and yield their results.

    `timeout_seconds_per_molfile` bounds how long a single record may occupy a
    consumer. Pass `get_molfile_id` to enforce it: a consumer that overruns is
    killed and its record yielded as a `TimedOut`, leaving the rest of the SDF to
    the surviving consumers. Without it there is no way to say which record was
    to blame, and an overrun aborts the whole SDF as it always did."""
    molfile_queue: multiprocessing.Queue = multiprocessing.Queue()  # TODO: limit size?
    result_queue: multiprocessing.Queue = multiprocessing.Queue()

    producer_process = multiprocessing.Process(
        target=_produce_molfiles,
        args=(molfile_queue, sdf_path, number_of_consumer_processes),
    )

    process_ids = list(range(number_of_consumer_processes))
    # One slot per consumer, written only by that consumer, read only by us.
    current_molfile_ids = [
        multiprocessing.Array(ctypes.c_char, _MOLFILE_ID_BYTES)
        for _ in process_ids
    ]
    started_at = [multiprocessing.Value(ctypes.c_double, 0.0) for _ in process_ids]
    consumer_processes = [
        multiprocessing.Process(
            target=_consume_molfiles,
            args=(
                molfile_queue,
                result_queue,
                process_id,
                consumer_function,
                get_molfile_id,
                current_molfile_ids[process_id],
                started_at[process_id],
            ),
        )
        for process_id in process_ids
    ]

    def _overrunning() -> list[int]:
        """Consumers still alive and on the same record for too long."""
        now = time.monotonic()
        overrunning = []
        for process_id in list(process_ids):
            if not consumer_processes[process_id].is_alive():
                continue
            begun = started_at[process_id].value
            if begun and now - begun > timeout_seconds_per_molfile:
                overrunning.append(process_id)

        return overrunning

    try:
        producer_process.start()

        for consumer_process in consumer_processes:
            consumer_process.start()

        while process_ids:
            try:
                result = result_queue.get(
                    timeout=timeout_seconds_per_molfile
                )  # Blocks until result is available or timeout is reached.
            except Empty:
                # Silence means every consumer is finished or busy; by itself it
                # does not mean one died. A consumer sitting on one record for
                # longer than the timeout is the reason, and killing it costs
                # this record rather than the whole SDF.
                overrunning = _overrunning()
                if not overrunning:
                    raise  # Handled below: a consumer really is gone.

                now = time.monotonic()
                for overrunning_id in overrunning:
                    molfile_id = current_molfile_ids[overrunning_id].value.decode(
                        "utf-8", "backslashreplace"
                    )
                    seconds = now - started_at[overrunning_id].value
                    logger.error(
                        f"timed out after {seconds:.0f}s on molfile ID {molfile_id} "
                        f"from {sdf_path.name}; killing consumer {overrunning_id}."
                    )
                    consumer_processes[overrunning_id].kill()
                    consumer_processes[overrunning_id].join()
                    # A killed consumer never posts its sentinel, so retire it here.
                    process_ids.remove(overrunning_id)
                    yield TimedOut(molfile_id=molfile_id, seconds=seconds)

                continue

            if result in process_ids:
                process_ids.remove(result)
                continue
            yield result

        # Processes won't join before all queues their interacting with are empty.
        producer_process.join()
        for consumer_process in consumer_processes:
            consumer_process.join()

    except Empty:
        # Nothing overrunning and nothing talking: a consumer really has gone,
        # and we cannot catch its exception from here.
        message = f"could not process {sdf_path}: A process terminated unexpectedly."
        logger.error(message)

        raise RuntimeError(message)

    except Exception as exception:
        # Exceptions during setup and teardown.
        logger.error(
            f"could not process {sdf_path}: {type(exception).__name__}; {exception}"
        )

        raise exception

    finally:
        producer_process.terminate()
        for consumer_process in consumer_processes:
            consumer_process.terminate()
