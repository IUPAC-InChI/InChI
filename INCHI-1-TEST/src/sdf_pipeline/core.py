import multiprocessing
from queue import Empty
import gzip
from typing import Callable, TYPE_CHECKING
from collections.abc import Generator
from sdf_pipeline import logger

if TYPE_CHECKING:
    # https://adamj.eu/tech/2021/05/13/python-type-hints-how-to-fix-circular-imports/
    from sdf_pipeline.drivers import ConsumerResult


def read_records_from_gzipped_sdf(sdf_path: str) -> Generator[str, None, None]:
    # https://en.wikipedia.org/wiki/Chemical_table_file#SDF"
    current_record = ""
    # TODO: guard file opening.
    with gzip.open(sdf_path, "rb") as gzipped_sdf:
        # Decompress SDF line-by-line to avoid loading entire SDF into memory.
        for decompressed_line in gzipped_sdf:
            decoded_line = decompressed_line.decode("utf-8", "backslashreplace")
            current_record += decoded_line
            if decoded_line.strip() == "$$$$":
                # TODO: harden SDF parsing according to
                # http://www.dalkescientific.com/writings/diary/archive/2020/09/18/handling_the_sdf_record_delimiter.html.
                yield current_record
                current_record = ""

    return None


def _produce_molfiles(
    molfile_queue: multiprocessing.Queue, sdf_path: str, n_poison_pills: int
) -> None:
    for molfile in read_records_from_gzipped_sdf(sdf_path):
        molfile_queue.put(molfile)

    for _ in range(n_poison_pills):
        # Poison pill: tell consumer processes we're done.
        molfile_queue.put("DONE")


def _consume_molfiles(
    molfile_queue: multiprocessing.Queue,
    result_queue: multiprocessing.Queue,
    process_id: int,
    consumer_function: Callable,
) -> None:
    for molfile in iter(molfile_queue.get, "DONE"):
        result_queue.put(consumer_function(molfile))

    result_queue.put(process_id)


def run(
    sdf_path: str,
    consumer_function: Callable,
    number_of_consumer_processes: int,
    timeout_seconds_per_molfile: int = 60,
) -> Generator["ConsumerResult", None, None]:
    molfile_queue: multiprocessing.Queue = multiprocessing.Queue()  # TODO: limit size?
    result_queue: multiprocessing.Queue = multiprocessing.Queue()

    producer_process = multiprocessing.Process(
        target=_produce_molfiles,
        args=(molfile_queue, sdf_path, number_of_consumer_processes),
    )

    process_ids = list(range(number_of_consumer_processes))
    consumer_processes = [
        multiprocessing.Process(
            target=_consume_molfiles,
            args=(
                molfile_queue,
                result_queue,
                process_id,
                consumer_function,
            ),
        )
        for process_id in process_ids
    ]

    try:
        producer_process.start()

        for consumer_process in consumer_processes:
            consumer_process.start()

        while process_ids:
            result = result_queue.get(
                timeout=timeout_seconds_per_molfile
            )  # Blocks until result is available or timeout is reached.
            if result in process_ids:
                process_ids.remove(result)
                continue
            yield result

        # Processes won't join before all queues their interacting with are empty.
        producer_process.join()
        for consumer_process in consumer_processes:
            consumer_process.join()

    except Empty:
        # We cannot directly catch exceptions from individual processes.
        # The best we can do is assume that the process terminated unexpectedly
        # (e.g., segfaults) if `result_queue.get()` times out.
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