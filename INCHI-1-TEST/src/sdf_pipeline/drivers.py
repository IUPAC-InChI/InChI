"""
From https://docs.python.org/3/library/sqlite3.html#how-to-use-the-connection-context-manager:
'If the body of the with statement finishes without exceptions, the transaction is committed.
If this commit fails, or if the body of the with statement raises an uncaught exception, the transaction is rolled back.'

Contrary to the conventional behavior of context managers, the connection is not closed upon leaving the `with` block:
'The context manager neither implicitly opens a new transaction nor closes the connection.'
See also https://blog.rtwilson.com/a-python-sqlite3-context-manager-gotcha/.

"""

import sqlite3
import json
from typing import Callable, Any
from functools import partial
from pathlib import Path
from datetime import datetime
from pydantic import BaseModel, Field
from sdf_pipeline import core, logger


class ConsumerInfo(BaseModel):
    consumer: str
    parameters: str = Field(default="")


class ConsumerResult(BaseModel):
    molfile_id: str
    info: ConsumerInfo
    time: str = Field(
        default_factory=lambda: datetime.now().isoformat(timespec="seconds")
    )
    result: dict[str, Any]


def _log_timeout(
    timed_out: core.TimedOut,
    sdf_path: Path,
    expected_failures: set[str],
    exit_code: int,
) -> int:
    """Log an overrunning molfile and fail the run unless it is expected."""
    expected_failure = timed_out.molfile_id in expected_failures
    log_entry = json.dumps(
        {
            "molfile_id": timed_out.molfile_id,
            "sdf": sdf_path.name,
            "seconds": round(timed_out.seconds, 1),
        }
    )
    logger.info(
        f"timed out{' expectedly' if expected_failure else ''}:{log_entry}"
    )

    # Never reset the exit code, matching the other failure paths.
    return exit_code if expected_failure else 1


def regression(
    sdf_path: Path,
    reference_path: Path,
    consumer_function: Callable,
    get_molfile_id: Callable,
    number_of_consumer_processes: int = 8,
    timeout_seconds_per_molfile: int = 60,
    expected_failures: set[str] = set(),
    compare: Callable[[dict, dict], bool] | None = None,
) -> int:
    with sqlite3.connect(reference_path) as reference_db:
        exit_code = 0
        processed_molfile_ids = set()

        for consumer_result in core.run(
            sdf_path=sdf_path,
            consumer_function=partial(consumer_function, get_molfile_id=get_molfile_id),
            number_of_consumer_processes=number_of_consumer_processes,
            timeout_seconds_per_molfile=timeout_seconds_per_molfile,
            get_molfile_id=get_molfile_id,
        ):
            if isinstance(consumer_result, core.TimedOut):
                # No result to compare, so this is a failure of the run rather
                # than a difference from the reference.
                exit_code = _log_timeout(
                    consumer_result, sdf_path, expected_failures, exit_code
                )
                processed_molfile_ids.add(consumer_result.molfile_id)
                continue

            molfile_id = consumer_result.molfile_id
            assert (
                molfile_id not in processed_molfile_ids
            ), f"Molfile ID {molfile_id} has been processed multiple times."
            processed_molfile_ids.add(molfile_id)

            reference_query = reference_db.execute(
                "SELECT result FROM results WHERE molfile_id = ?",
                (molfile_id,),
            ).fetchone()
            assert (
                reference_query
            ), f"Couldn't find molfile ID {molfile_id} in reference."
            reference_result = reference_query[0]

            current_result = json.dumps(consumer_result.result)
            if compare is None:
                # Default: byte-for-byte, as the CI regression tests rely on.
                is_match = current_result == reference_result
            else:
                # The caller decides what counts as a match. `compare` receives the
                # result dictionaries and knows nothing about this module.
                is_match = compare(consumer_result.result, json.loads(reference_result))

            if not is_match:
                expected_failure = molfile_id in expected_failures
                # Never reset the exit code: once an unexpected failure has
                # occurred the run must fail, regardless of any later expected
                # failures.
                if not expected_failure:
                    exit_code = 1
                log_entry = json.dumps(
                    {
                        "time": consumer_result.time,
                        "molfile_id": molfile_id,
                        "sdf": sdf_path.name,
                        "info": dict(consumer_result.info),
                        "diff": {
                            "current": current_result,
                            "reference": reference_result,
                        },
                    }
                )
                logger.info(
                    f"regression test failed{' expectedly' if expected_failure else ''}:{log_entry}"
                )

        unprocessed_molfile_ids = (
            set(
                molfile_id[0]
                for molfile_id in reference_db.execute(
                    "SELECT molfile_id FROM results"
                ).fetchall()
            )
            - processed_molfile_ids
        )

        assert (
            not unprocessed_molfile_ids
        ), f"Reference contains molfile IDs that haven't been processed: {unprocessed_molfile_ids}."

    reference_db.close()

    return exit_code


def regression_reference(
    sdf_path: Path,
    reference_path: Path,
    consumer_function: Callable,
    get_molfile_id: Callable,
    number_of_consumer_processes: int = 8,
    timeout_seconds_per_molfile: int = 60,
) -> int:
    with sqlite3.connect(reference_path) as reference_db:
        reference_db.execute(
            "CREATE TABLE IF NOT EXISTS results (molfile_id UNIQUE, time, info, result)"
        )

        for consumer_result in core.run(
            sdf_path=sdf_path,
            consumer_function=partial(consumer_function, get_molfile_id=get_molfile_id),
            number_of_consumer_processes=number_of_consumer_processes,
            timeout_seconds_per_molfile=timeout_seconds_per_molfile,
            get_molfile_id=get_molfile_id,
        ):
            if isinstance(consumer_result, core.TimedOut):
                # Recorded rather than dropped: a reference that silently omits
                # a molfile makes every later run fail on "molfile IDs that
                # haven't been processed", naming the shard but not the reason.
                logger.info(
                    f"reference timed out: molfile ID {consumer_result.molfile_id} "
                    f"from {sdf_path.name} after {consumer_result.seconds:.0f}s."
                )
                reference_db.execute(
                    "INSERT INTO results VALUES (:molfile_id, :time, :info, :result)",
                    {
                        "molfile_id": consumer_result.molfile_id,
                        "time": datetime.now().isoformat(timespec="seconds"),
                        "info": ConsumerInfo(consumer="timeout").model_dump_json(),
                        "result": json.dumps({"timeout_seconds": consumer_result.seconds}),
                    },
                )
                continue

            reference_db.execute(
                "INSERT INTO results VALUES (:molfile_id, :time, :info, :result)",
                {
                    "molfile_id": consumer_result.molfile_id,
                    "time": consumer_result.time,
                    "info": consumer_result.info.model_dump_json(),
                    "result": json.dumps(consumer_result.result),
                },
            )

        reference_db.execute(
            "CREATE INDEX IF NOT EXISTS molfile_id_index ON results (molfile_id)"
        )  # crucial, reduces look-up speed by orders of magnitude

    reference_db.close()

    return 0


def invariance(
    sdf_path: Path,
    consumer_function: Callable,
    get_molfile_id: Callable,
    number_of_consumer_processes: int = 8,
    timeout_seconds_per_molfile: int = 60,
    expected_failures: set[str] = set(),
) -> int:
    exit_code = 0

    for consumer_result in core.run(
        sdf_path=sdf_path,
        consumer_function=partial(consumer_function, get_molfile_id=get_molfile_id),
        number_of_consumer_processes=number_of_consumer_processes,
        timeout_seconds_per_molfile=timeout_seconds_per_molfile,
        get_molfile_id=get_molfile_id,
    ):
        if isinstance(consumer_result, core.TimedOut):
            exit_code = _log_timeout(
                consumer_result, sdf_path, expected_failures, exit_code
            )
            continue

        n_variants = len(consumer_result.result["variants"])
        if n_variants == 1:
            continue
        expected_failure = consumer_result.molfile_id in expected_failures
        # Never reset the exit code: once an unexpected failure has occurred the
        # run must fail, regardless of any later expected failures.
        if not expected_failure:
            exit_code = 1
        if n_variants == 0:
            logger.info(
                f"invariance test didn't run: molfile ID {consumer_result.molfile_id} from {sdf_path.name} could not be read."
            )
        else:
            log_entry = json.dumps(
                {
                    "time": consumer_result.time,
                    "molfile_id": consumer_result.molfile_id,
                    "sdf": sdf_path.name,
                    "info": dict(consumer_result.info),
                    "variants": consumer_result.result["variants"],
                }
            )
            logger.info(
                f"invariance test failed{' expectedly' if expected_failure else ''}:{log_entry}"
            )

    return exit_code
