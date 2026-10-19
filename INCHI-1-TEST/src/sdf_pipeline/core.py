import multiprocessing
import time
from dataclasses import dataclass
from queue import Empty
from typing import Any, Callable, TYPE_CHECKING
from collections.abc import Generator
from pathlib import Path
from sdf_pipeline import logger, utils

if TYPE_CHECKING:
    # https://adamj.eu/tech/2021/05/13/python-type-hints-how-to-fix-circular-imports/
    from sdf_pipeline.drivers import ConsumerResult


@dataclass(frozen=True)
class TimedOut:
    """A molfile whose consumer was stopped after exceeding the time limit.

    Yielded in place of a result, so the caller can record the molfile and the
    rest of the SDF is still processed. The consumer usually spends its time in
    C code, which a timer inside the consumer process cannot interrupt, so the
    parent stops the process instead."""

    molfile_id: str
    seconds: float


@dataclass(frozen=True)
class _Started:
    """Sent by a consumer before it processes a molfile."""

    molfile_id: str


class _ConsumerDied(Exception):
    pass


def _produce_molfiles(
    molfile_queue: multiprocessing.Queue, sdf_path: Path, n_poison_pills: int
) -> None:
    for molfile in utils.read_records_from_gzipped_sdf(sdf_path):
        molfile_queue.put(molfile)

    for _ in range(n_poison_pills):
        # Poison pill: tell consumer processes we're done.
        molfile_queue.put("DONE")


def _consume_molfiles(
    molfile_queue: multiprocessing.Queue,
    result_queue: multiprocessing.Queue,
    process_id: int,
    consumer_function: Callable,
    get_molfile_id: Callable | None,
) -> None:
    # Every message is `(process_id, payload)`: `_Started` before a molfile, the
    # consumer's result after it, and `None` once the queue is exhausted.
    for molfile in iter(molfile_queue.get, "DONE"):
        molfile_id = get_molfile_id(molfile) if get_molfile_id is not None else ""
        result_queue.put((process_id, _Started(molfile_id)))
        result_queue.put((process_id, consumer_function(molfile)))

    result_queue.put((process_id, None))


def run(
    sdf_path: Path,
    consumer_function: Callable,
    number_of_consumer_processes: int,
    timeout_seconds_per_molfile: int = 60,
    get_molfile_id: Callable | None = None,
) -> Generator["ConsumerResult | TimedOut", None, None]:
    """Distribute the SDF's records across consumer processes and yield results.

    A consumer that spends more than `timeout_seconds_per_molfile` on one molfile
    is stopped, the molfile is yielded as a `TimedOut`, and a new consumer takes
    its place for the rest of the SDF. `get_molfile_id` names the molfile in the
    `TimedOut`; without it the ID is empty.

    The time is measured from when the parent receives the consumer's `_Started`
    message, so a consumer is stopped no earlier than the limit after it started,
    and `TimedOut.seconds` can be slightly lower than the time it actually ran."""
    molfile_queue: multiprocessing.Queue = multiprocessing.Queue()  # TODO: limit size?
    result_queue: multiprocessing.Queue = multiprocessing.Queue()

    producer_process = multiprocessing.Process(
        target=_produce_molfiles,
        args=(molfile_queue, sdf_path, number_of_consumer_processes),
    )

    def _new_consumer(process_id: int) -> multiprocessing.Process:
        return multiprocessing.Process(
            target=_consume_molfiles,
            args=(
                molfile_queue,
                result_queue,
                process_id,
                consumer_function,
                get_molfile_id,
            ),
        )

    process_ids = list(range(number_of_consumer_processes))
    consumer_processes = [_new_consumer(process_id) for process_id in process_ids]
    # Consumers that have not yet sent `None`.
    running = set(process_ids)
    # The molfile each consumer is working on, and when the parent learned of it.
    started: dict[int, tuple[str, float]] = {}

    def _handle(message: tuple[int, Any]) -> Generator[Any, None, None]:
        process_id, payload = message
        if payload is None:
            running.discard(process_id)
            started.pop(process_id, None)
        elif isinstance(payload, _Started):
            started[process_id] = (payload.molfile_id, time.monotonic())
        else:
            started.pop(process_id, None)
            yield payload

    def _over_time() -> list[int]:
        now = time.monotonic()

        return [
            process_id
            for process_id, (_, began) in started.items()
            if now - began >= timeout_seconds_per_molfile
        ]

    def _seconds_to_next_limit() -> float:
        if not started:
            return timeout_seconds_per_molfile
        earliest = min(began for _, began in started.values())

        return max(0.0, earliest + timeout_seconds_per_molfile - time.monotonic())

    try:
        producer_process.start()

        for consumer_process in consumer_processes:
            consumer_process.start()

        while running:
            if _over_time():
                # A result for one of these molfiles may already be queued behind
                # others, so read everything that has arrived before deciding.
                while True:
                    try:
                        message = result_queue.get_nowait()
                    except Empty:
                        break
                    yield from _handle(message)

                for process_id in _over_time():
                    if not consumer_processes[process_id].is_alive():
                        raise _ConsumerDied()
                    molfile_id, began = started.pop(process_id)
                    seconds = time.monotonic() - began
                    logger.error(
                        f"timed out after {seconds:.0f}s on molfile ID {molfile_id} "
                        f"from {sdf_path.name}; stopping consumer {process_id} and "
                        "starting a new one."
                    )
                    # A stopped consumer never takes its poison pill, so its
                    # replacement takes it instead, and the producer can finish.
                    consumer_processes[process_id].kill()
                    consumer_processes[process_id].join()
                    consumer_processes[process_id] = _new_consumer(process_id)
                    consumer_processes[process_id].start()
                    yield TimedOut(molfile_id=molfile_id, seconds=seconds)

                continue

            try:
                message = result_queue.get(timeout=_seconds_to_next_limit())
            except Empty:
                # No message before the next time limit. A consumer that exits
                # normally sends `None` first, so one that is no longer alive
                # here has crashed. So has a producer that exited with an error.
                if any(
                    not consumer_processes[process_id].is_alive()
                    for process_id in running
                ) or producer_process.exitcode not in (None, 0):
                    raise _ConsumerDied()
                continue

            yield from _handle(message)

        # Processes won't join before all queues their interacting with are empty.
        producer_process.join()
        for consumer_process in consumer_processes:
            consumer_process.join()

    except _ConsumerDied:
        # We cannot directly catch exceptions from individual processes, e.g.,
        # segfaults. We only see that the process is gone.
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
