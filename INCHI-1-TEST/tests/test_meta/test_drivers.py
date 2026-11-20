import pytest
import re
import sqlite3
import logging
import json
from operator import add
from functools import reduce
from pathlib import Path
from functools import partial
from types import SimpleNamespace
from unittest.mock import Mock
from inchi_tests import parse_log
from sdf_pipeline import drivers, core
from consumers import (
    regression_consumer,
    two_diff_regression_consumer,
    invariance_consumer,
    two_diff_invariance_consumer,
    raising_consumer,
    segfaulting_consumer,
)


def _get_mcule_id(molfile: str) -> str:
    molfile_id_pattern = re.compile(r"<Mcule_ID>(.*?)>", re.DOTALL)
    molfile_id_match = molfile_id_pattern.search(molfile)
    molfile_id = molfile_id_match.group(1).strip() if molfile_id_match else ""
    return molfile_id


@pytest.fixture
def sdf_path():
    return Path(__file__).parent.absolute().joinpath("data/mcule_20000.sdf.gz")


@pytest.fixture
def reference_path():
    return (
        Path(__file__)
        .parent.absolute()
        .joinpath("data/mcule_20000_regression_reference.sqlite")
    )


def test_regression_reference_driver(sdf_path, tmp_path):
    exit_code = drivers.regression_reference(
        sdf_path=sdf_path,
        reference_path=tmp_path / "regression_reference.sqlite",
        consumer_function=regression_consumer,
        get_molfile_id=_get_mcule_id,
        number_of_consumer_processes=2,
    )
    assert exit_code == 0
    with sqlite3.connect(tmp_path / "regression_reference.sqlite") as db:
        results = db.execute(
            "SELECT result FROM results ",
        ).fetchall()
        assert len(results) == 20000
        assert (
            reduce(
                add,
                [int(json.loads(result[0])["molfile_length"]) for result in results],
            )
            == 31062992
        )


@pytest.mark.parametrize("expected_failures,exit_code", [({}, 1), ({"9261759198"}, 0)])
def test_regression_driver(
    sdf_path, reference_path, caplog, expected_failures, exit_code
):
    caplog.set_level(logging.INFO, logger="sdf_pipeline")
    assert (
        drivers.regression(
            sdf_path=sdf_path,
            reference_path=reference_path,
            consumer_function=regression_consumer,
            get_molfile_id=_get_mcule_id,
            number_of_consumer_processes=2,
            expected_failures=expected_failures,
        )
        == exit_code
    )
    assert len(caplog.records) == 1
    log_message = caplog.records[0].message
    if exit_code == 0:
        assert "regression test failed expectedly" in log_message
    start_log_entry = log_message.index("{")
    log_entry = json.loads(log_message[start_log_entry:])
    assert log_entry["molfile_id"] == "9261759198"
    assert log_entry["sdf"] == "mcule_20000.sdf.gz"
    assert log_entry["info"] == {"consumer": "regression", "parameters": ""}
    assert log_entry["diff"] == {
        "current": '{"molfile_length": 42}',
        "reference": '{"molfile_length": 926}',
    }


def test_regression_driver_does_not_mask_unexpected_failure(
    sdf_path, reference_path, caplog
):
    # "9261759198" is the first record and "1690718558" is the last record in
    # mcule_20000.sdf.gz. With a single consumer process, results are yielded in
    # SDF order, so the unexpected failure is seen before the expected one.
    # A later expected failure must not reset the exit code back to 0.
    caplog.set_level(logging.INFO, logger="sdf_pipeline")
    exit_code = drivers.regression(
        sdf_path=sdf_path,
        reference_path=reference_path,
        consumer_function=two_diff_regression_consumer,
        get_molfile_id=_get_mcule_id,
        number_of_consumer_processes=1,
        expected_failures={"1690718558"},
    )
    assert exit_code == 1
    logged_ids = {
        json.loads(record.message[record.message.index("{") :])["molfile_id"]
        for record in caplog.records
    }
    assert logged_ids == {"9261759198", "1690718558"}


@pytest.mark.parametrize("expected_failures,exit_code", [({}, 1), ({"9261759198"}, 0)])
def test_invariance_driver(sdf_path, caplog, expected_failures, exit_code):
    caplog.set_level(logging.INFO, logger="sdf_pipeline")
    assert (
        drivers.invariance(
            sdf_path=sdf_path,
            consumer_function=invariance_consumer,
            get_molfile_id=_get_mcule_id,
            number_of_consumer_processes=2,
            expected_failures=expected_failures,
        )
        == exit_code
    )
    assert len(caplog.records) == 1
    log_message = caplog.records[0].message
    if exit_code == 0:
        assert "invariance test failed expectedly" in log_message
    start_log_entry = log_message.index("{")
    log_entry = json.loads(log_message[start_log_entry:])
    assert log_entry["molfile_id"] == "9261759198"
    assert log_entry["sdf"] == "mcule_20000.sdf.gz"
    assert log_entry["info"] == {"consumer": "invariance", "parameters": ""}
    assert log_entry["variants"] == ["A", "B"]


def test_invariance_driver_does_not_mask_unexpected_failure(sdf_path, caplog):
    # See test_regression_driver_does_not_mask_unexpected_failure: with a single
    # consumer process the unexpected failure ("9261759198", first record) is
    # seen before the expected one ("1690718558", last record), and a later
    # expected failure must not reset the exit code back to 0.
    caplog.set_level(logging.INFO, logger="sdf_pipeline")
    exit_code = drivers.invariance(
        sdf_path=sdf_path,
        consumer_function=two_diff_invariance_consumer,
        get_molfile_id=_get_mcule_id,
        number_of_consumer_processes=1,
        expected_failures={"1690718558"},
    )
    assert exit_code == 1
    logged_ids = {
        json.loads(record.message[record.message.index("{") :])["molfile_id"]
        for record in caplog.records
    }
    assert logged_ids == {"9261759198", "1690718558"}


@pytest.mark.parametrize("consumer", [raising_consumer, segfaulting_consumer])
def test_core_raises(sdf_path, caplog, consumer):
    caplog.set_level(logging.ERROR, logger="sdf_pipeline")
    with pytest.raises(RuntimeError):
        for _ in core.run(
            sdf_path=sdf_path,
            consumer_function=partial(consumer, get_molfile_id=_get_mcule_id),
            number_of_consumer_processes=2,
            timeout_seconds_per_molfile=5,
        ):
            pass
    assert (
        caplog.records[0].message
        == f"could not process {sdf_path}: A process terminated unexpectedly."
    )


@pytest.mark.parametrize("expected", [False, True])
@pytest.mark.parametrize("error", ["", "first\nsecond\nfirst"])
@pytest.mark.parametrize(
    "variants",
    [
        [],
        [{"inchi": "InChI=valid", "key": "KEY"}],
        [
            {"inchi": "InChI=valid", "key": "KEY"},
            {"inchi": "InChI=other", "key": "OTHER"},
        ],
    ],
)
def test_driver_logs_errors_separately_and_parses_only_test_failures(
    monkeypatch, caplog, tmp_path, expected, error, variants
):
    result = drivers.ConsumerResult(
        molfile_id="id",
        info=drivers.ConsumerInfo(consumer="invariance"),
        error=error,
        result={"variants": variants},
    )
    monkeypatch.setattr(drivers.core, "run", lambda **_: iter([result]))
    caplog.set_level(logging.INFO, logger="sdf_pipeline")
    sdf_path = tmp_path / "input.sdf.gz"

    exit_code = drivers.invariance(
        sdf_path, Mock(), Mock(), expected_failures={"id"} if expected else set()
    )

    test_failure = len(variants) > 1
    assert exit_code == int(bool(error) or (test_failure and not expected))
    assert len(caplog.records) == int(bool(error)) + int(test_failure)
    suffix = " expectedly" if expected else ""
    for record in caplog.records:
        message = record.message
        entry = json.loads(message[message.index("{") :])
        assert entry["info"]["consumer"] == "invariance"
        if message.startswith("error"):
            assert entry["error"] == error
            assert "variants" not in entry
        else:
            assert message.startswith(f"invariance test failed{suffix}:")
            assert entry["variants"] == variants
            assert "error" not in entry

    log_path = tmp_path / "20261109_invariance_test.log"
    log_path.write_text(
        "".join(f"INFO:sdf_pipeline:{record.message}\n" for record in caplog.records)
    )
    monkeypatch.setattr(
        parse_log,
        "select_records_from_gzipped_sdf",
        lambda *_: iter([("id", "original")]),
    )
    parse_log.main(
        "invariance",
        SimpleNamespace(name="test", path=tmp_path, molfile_id_getter=Mock()),
    )
    summary_path = tmp_path / f"{log_path.stem}_input.sdf.html"
    assert summary_path.exists() == test_failure
    if test_failure:
        summary = summary_path.read_text()
        assert "<h1>id</h1>" in summary
        assert "InChI=valid" in summary
        assert "InChI=other" in summary
        assert "KEY" in summary
        assert "second" not in summary


def test_driver_error_failure_is_not_reset_by_expected_failure(monkeypatch):
    results = [
        drivers.ConsumerResult(
            molfile_id=molfile_id,
            info=drivers.ConsumerInfo(consumer="invariance"),
            error="failed",
            result={"variants": []},
        )
        for molfile_id in ["unexpected", "expected"]
    ]
    monkeypatch.setattr(drivers.core, "run", lambda **_: iter(results))
    assert (
        drivers.invariance(
            Path("unused"), Mock(), Mock(), expected_failures={"expected"}
        )
        == 1
    )


def test_driver_timeout_is_logged_as_consumer_error_and_fails(
    monkeypatch, caplog, tmp_path
):
    timeout = drivers.core.TimedOut(molfile_id="id", seconds=60.04)
    monkeypatch.setattr(drivers.core, "run", lambda **_: iter([timeout]))
    caplog.set_level(logging.INFO, logger="sdf_pipeline")
    sdf_path = tmp_path / "input.sdf.gz"

    exit_code = drivers.invariance(sdf_path, Mock(), Mock(), expected_failures={"id"})

    assert exit_code == 1
    assert len(caplog.records) == 1
    message = caplog.records[0].message
    assert message.startswith("error:")
    entry = json.loads(message[message.index("{") :])
    assert entry["molfile_id"] == "id"
    assert entry["sdf"] == "input.sdf.gz"
    assert entry["error"] == "Timed out after 60.0 seconds."
