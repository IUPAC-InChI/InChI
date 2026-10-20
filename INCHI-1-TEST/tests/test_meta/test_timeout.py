"""A molfile that exceeds the time limit must not stop the rest of the SDF.

PubChem substance record SID 141382403 (238 atoms, 204 of them phosphorus) did
not finish within ten minutes in v1.07.5. `core.run` waited 60 s for any result,
received none, and reported "A process terminated unexpectedly" although the
consumer was still running. None of the 500,000 records in that SDF were
processed.
"""

import gzip
import sqlite3
import time
import pytest
from functools import partial
from sdf_pipeline import core, drivers


SLOW_MOLFILE_ID = "999"
# Long enough that only the time limit ends it within the test.
SLOW_SECONDS = 3600


def _record(molfile_id: str) -> str:
    return (
        f"{molfile_id}\n  test\n\n"
        "  0  0  0  0  0  0  0  0  0  0999 V2000\nM  END\n$$$$\n"
    )


@pytest.fixture
def sdf(tmp_path):
    def _write(ids):
        path = tmp_path / "test.sdf.gz"
        with gzip.open(path, "wt", encoding="utf-8") as handle:
            handle.write("".join(_record(molfile_id) for molfile_id in ids))
        return path

    return _write


def _get_molfile_id(molfile: str) -> str:
    return molfile.split()[0].strip()


def slow_consumer(
    molfile: str, get_molfile_id, seconds_per_molfile: float = 0.0
) -> drivers.ConsumerResult:
    """Runs for `SLOW_SECONDS` on `SLOW_MOLFILE_ID`, as libinchi does on SID 141382403."""
    molfile_id = get_molfile_id(molfile)
    time.sleep(SLOW_SECONDS if molfile_id == SLOW_MOLFILE_ID else seconds_per_molfile)

    return drivers.ConsumerResult(
        molfile_id=molfile_id,
        info=drivers.ConsumerInfo(consumer="test"),
        result={"ok": True},
    )


def _run(path, number_of_consumer_processes=2, timeout=5, **consumer_arguments):
    return list(
        core.run(
            sdf_path=path,
            consumer_function=partial(
                slow_consumer, get_molfile_id=_get_molfile_id, **consumer_arguments
            ),
            number_of_consumer_processes=number_of_consumer_processes,
            timeout_seconds_per_molfile=timeout,
            get_molfile_id=_get_molfile_id,
        )
    )


def test_the_sdf_is_processed_despite_a_molfile_over_the_time_limit(sdf):
    results = _run(sdf(["1", "2", SLOW_MOLFILE_ID, "4", "5"]))

    timed_out = [r for r in results if isinstance(r, core.TimedOut)]
    completed = [r for r in results if not isinstance(r, core.TimedOut)]

    assert len(timed_out) == 1
    assert timed_out[0].molfile_id == SLOW_MOLFILE_ID
    assert timed_out[0].seconds >= 5
    assert sorted(r.molfile_id for r in completed) == ["1", "2", "4", "5"]


def test_an_sdf_without_slow_molfiles_has_no_timeouts(sdf):
    results = _run(sdf(["1", "2", "3"]))

    assert not [r for r in results if isinstance(r, core.TimedOut)]
    assert sorted(r.molfile_id for r in results) == ["1", "2", "3"]


def test_the_time_limit_applies_while_other_consumers_send_results(sdf):
    """The limit applies to each molfile, not to a silent queue.

    The other consumer sends a result every 0.25 s for about 5 s. A limit checked
    only when no result arrives would stop the slow molfile after those 5 s."""
    ids = [SLOW_MOLFILE_ID] + [str(i) for i in range(1, 21)]
    results = _run(sdf(ids), timeout=2, seconds_per_molfile=0.25)

    timed_out = [r for r in results if isinstance(r, core.TimedOut)]
    assert [r.molfile_id for r in timed_out] == [SLOW_MOLFILE_ID]
    assert 2 <= timed_out[0].seconds < 4
    assert len(results) == len(ids)


def test_a_new_consumer_continues_after_the_only_consumer_is_stopped(sdf):
    results = _run(sdf([SLOW_MOLFILE_ID, "2", "3"]), number_of_consumer_processes=1)

    assert [r.molfile_id for r in results if isinstance(r, core.TimedOut)] == [
        SLOW_MOLFILE_ID
    ]
    assert sorted(
        r.molfile_id for r in results if not isinstance(r, core.TimedOut)
    ) == ["2", "3"]


@pytest.fixture
def run_with_timeout(monkeypatch):
    """Replace `core.run` with results for "1" and "3" and a timeout for the slow ID.

    The drivers' handling of `TimedOut` is tested without processes; the tests
    above cover how `core.run` produces it."""

    def _fake_run(sdf_path, consumer_function, number_of_consumer_processes, **_):
        for molfile_id in ("1", "3"):
            yield drivers.ConsumerResult(
                molfile_id=molfile_id,
                info=drivers.ConsumerInfo(consumer="test"),
                result={"ok": True},
            )
        yield core.TimedOut(molfile_id=SLOW_MOLFILE_ID, seconds=60.0)

    monkeypatch.setattr(core, "run", _fake_run)


def test_the_reference_records_a_timed_out_molfile(sdf, tmp_path, run_with_timeout):
    """Without the row, every later regression run fails on "Reference contains
    molfile IDs that haven't been processed", which does not say why."""
    reference = tmp_path / "reference.sqlite"

    drivers.regression_reference(
        sdf_path=sdf(["1", SLOW_MOLFILE_ID, "3"]),
        reference_path=reference,
        consumer_function=slow_consumer,
        get_molfile_id=_get_molfile_id,
    )

    with sqlite3.connect(reference) as db:
        rows = dict(db.execute("SELECT molfile_id, result FROM results").fetchall())

    assert sorted(rows) == ["1", "3", SLOW_MOLFILE_ID]
    assert "timeout_seconds" in rows[SLOW_MOLFILE_ID]


def test_regression_fails_on_a_timeout_unless_it_is_expected(
    sdf, tmp_path, run_with_timeout
):
    path = sdf(["1", SLOW_MOLFILE_ID, "3"])
    reference = tmp_path / "reference.sqlite"
    drivers.regression_reference(
        sdf_path=path,
        reference_path=reference,
        consumer_function=slow_consumer,
        get_molfile_id=_get_molfile_id,
    )

    def run(expected_failures):
        return drivers.regression(
            sdf_path=path,
            reference_path=reference,
            consumer_function=slow_consumer,
            get_molfile_id=_get_molfile_id,
            expected_failures=expected_failures,
        )

    assert run(set()) == 1
    assert run({SLOW_MOLFILE_ID}) == 0
