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


def regression(
    sdf_path: str,
    reference_path: str,
    consumer_function: Callable,
    get_molfile_id: Callable,
    number_of_consumer_processes: int = 8,
) -> int:
    with sqlite3.connect(reference_path) as reference_db:
        exit_code = 0
        processed_molfile_ids = set()

        for consumer_result in core.run(
            sdf_path=sdf_path,
            consumer_function=partial(consumer_function, get_molfile_id=get_molfile_id),
            number_of_consumer_processes=number_of_consumer_processes,
        ):
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
            if current_result != reference_result:
                exit_code = 1
                log_entry = json.dumps(
                    {
                        "time": consumer_result.time,
                        "molfile_id": molfile_id,
                        "sdf": Path(sdf_path).name,
                        "info": dict(consumer_result.info),
                        "diff": {
                            "current": current_result,
                            "reference": reference_result,
                        },
                    }
                )
                logger.info(f"regression test failed:{log_entry}")

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
    sdf_path: str,
    reference_path: str,
    consumer_function: Callable,
    get_molfile_id: Callable,
    number_of_consumer_processes: int = 8,
) -> int:
    with sqlite3.connect(reference_path) as reference_db:
        reference_db.execute(
            "CREATE TABLE IF NOT EXISTS results (molfile_id UNIQUE, time, info, result)"
        )

        for consumer_result in core.run(
            sdf_path=sdf_path,
            consumer_function=partial(consumer_function, get_molfile_id=get_molfile_id),
            number_of_consumer_processes=number_of_consumer_processes,
        ):
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
    sdf_path: str,
    consumer_function: Callable,
    get_molfile_id: Callable,
    number_of_consumer_processes: int = 8,
) -> int:
    exit_code = 0

    for consumer_result in core.run(
        sdf_path=sdf_path,
        consumer_function=partial(consumer_function, get_molfile_id=get_molfile_id),
        number_of_consumer_processes=number_of_consumer_processes,
    ):
        n_variants = len(consumer_result.result["variants"])
        if n_variants == 1:
            continue
        exit_code = 1
        if n_variants == 0:
            logger.info(
                f"invariance test didn't run: molfile ID {consumer_result.molfile_id} from {Path(sdf_path).name} could not be read."
            )
        else:
            log_entry = json.dumps(
                {
                    "time": consumer_result.time,
                    "molfile_id": consumer_result.molfile_id,
                    "sdf": Path(sdf_path).name,
                    "info": dict(consumer_result.info),
                    "variants": consumer_result.result["variants"],
                }
            )
            logger.info(f"invariance test failed:{log_entry}")

    return exit_code
