"""A molfile that never comes back must cost that molfile, not the whole SDF.

PubChem substance 141382403 -- 238 atoms, 204 of them phosphorus -- ran for over
ten minutes without finishing. `core.run` waited 60s for any result, got none,
and reported "A process terminated unexpectedly", which was a guess: the
consumer was alive and still computing. The whole 500000-record shard was lost,
and `run_campaign.sh` stopped the campaign on it.
"""

import gzip
import time
import pytest
from functools import partial
from pathlib import Path
from sdf_pipeline import core, drivers


SLOW_MOLFILE_ID = "999"


def _record(molfile_id: str) -> str:
    return (
        f"{molfile_id}\n  test\n\n"
        "  0  0  0  0  0  0  0  0  0  0999 V2000\nM  END\n$$$$\n"
    )


@pytest.fixture
def sdf(tmp_path):
    def _write(ids):
        path = tmp_path / "shard.sdf.gz"
        with gzip.open(path, "wt", encoding="utf-8") as handle:
            handle.write("".join(_record(molfile_id) for molfile_id in ids))
        return path

    return _write


def _get_molfile_id(molfile: str) -> str:
    return molfile.split()[0].strip()


def hanging_consumer(molfile: str, get_molfile_id) -> drivers.ConsumerResult:
    """Sleeps forever on one molfile, the way libinchi grinds on 141382403."""
    molfile_id = get_molfile_id(molfile)
    if molfile_id == SLOW_MOLFILE_ID:
        time.sleep(3600)

    return drivers.ConsumerResult(
        molfile_id=molfile_id,
        info=drivers.ConsumerInfo(consumer="test"),
        result={"ok": True},
    )


def test_the_shard_survives_a_molfile_that_never_returns(sdf):
    path = sdf(["1", "2", SLOW_MOLFILE_ID, "4", "5"])

    results = list(
        core.run(
            sdf_path=path,
            consumer_function=partial(
                hanging_consumer, get_molfile_id=_get_molfile_id
            ),
            number_of_consumer_processes=2,
            timeout_seconds_per_molfile=5,
            get_molfile_id=_get_molfile_id,
        )
    )

    timed_out = [r for r in results if isinstance(r, core.TimedOut)]
    completed = [r for r in results if not isinstance(r, core.TimedOut)]

    # The offender is named rather than guessed at.
    assert len(timed_out) == 1
    assert timed_out[0].molfile_id == SLOW_MOLFILE_ID
    assert timed_out[0].seconds >= 5
    # And every other record still came through.
    assert sorted(r.molfile_id for r in completed) == ["1", "2", "4", "5"]


def test_a_clean_shard_is_unaffected_by_the_watchdog(sdf):
    path = sdf(["1", "2", "3"])

    results = list(
        core.run(
            sdf_path=path,
            consumer_function=partial(
                hanging_consumer, get_molfile_id=_get_molfile_id
            ),
            number_of_consumer_processes=2,
            timeout_seconds_per_molfile=5,
            get_molfile_id=_get_molfile_id,
        )
    )

    assert not [r for r in results if isinstance(r, core.TimedOut)]
    assert sorted(r.molfile_id for r in results) == ["1", "2", "3"]


def test_reference_records_the_timeout_instead_of_dropping_the_molfile(sdf, tmp_path):
    """A reference that omits a molfile makes every later run fail obscurely.

    `drivers.regression` asserts on "Reference contains molfile IDs that haven't
    been processed", which names the shard but not the reason."""
    path = sdf(["1", SLOW_MOLFILE_ID, "3"])
    reference = tmp_path / "reference.sqlite"

    drivers.regression_reference(
        sdf_path=path,
        reference_path=reference,
        consumer_function=hanging_consumer,
        get_molfile_id=_get_molfile_id,
        number_of_consumer_processes=2,
        timeout_seconds_per_molfile=5,
    )

    import sqlite3

    with sqlite3.connect(reference) as db:
        rows = dict(db.execute("SELECT molfile_id, result FROM results").fetchall())

    assert sorted(rows) == ["1", "3", SLOW_MOLFILE_ID]
    assert "timeout_seconds" in rows[SLOW_MOLFILE_ID]


def test_regression_fails_on_a_timeout_unless_it_is_expected(sdf, tmp_path):
    path = sdf(["1", SLOW_MOLFILE_ID])
    reference = tmp_path / "reference.sqlite"
    drivers.regression_reference(
        sdf_path=path,
        reference_path=reference,
        consumer_function=hanging_consumer,
        get_molfile_id=_get_molfile_id,
        number_of_consumer_processes=2,
        timeout_seconds_per_molfile=5,
    )

    def run(expected_failures):
        return drivers.regression(
            sdf_path=path,
            reference_path=reference,
            consumer_function=hanging_consumer,
            get_molfile_id=_get_molfile_id,
            number_of_consumer_processes=2,
            timeout_seconds_per_molfile=5,
            expected_failures=expected_failures,
        )

    assert run(set()) == 1
    assert run({SLOW_MOLFILE_ID}) == 0
