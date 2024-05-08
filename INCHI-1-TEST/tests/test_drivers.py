import pytest
import re
import sqlite3
import logging
import json
from operator import add
from functools import reduce
from pathlib import Path
from functools import partial
from sdf_pipeline import drivers, core
from consumers import (
    regression_consumer,
    invariance_consumer,
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


def test_regression_driver(sdf_path, reference_path, caplog):
    caplog.set_level(logging.INFO, logger="sdf_pipeline")
    exit_code = drivers.regression(
        sdf_path=sdf_path,
        reference_path=reference_path,
        consumer_function=regression_consumer,
        get_molfile_id=_get_mcule_id,
        number_of_consumer_processes=2,
    )
    assert exit_code == 1
    assert len(caplog.records) == 1
    log_entry = json.loads(caplog.records[0].message.lstrip("regression test failed:"))
    assert log_entry["molfile_id"] == "9261759198"
    assert log_entry["sdf"] == "mcule_20000.sdf.gz"
    assert log_entry["info"] == {"consumer": "regression", "parameters": ""}
    assert log_entry["diff"] == {
        "current": '{"molfile_length": 42}',
        "reference": '{"molfile_length": 926}',
    }


def test_invariance_driver(sdf_path, caplog):
    caplog.set_level(logging.INFO, logger="sdf_pipeline")
    exit_code = drivers.invariance(
        sdf_path=sdf_path,
        consumer_function=invariance_consumer,
        get_molfile_id=_get_mcule_id,
        number_of_consumer_processes=2,
    )
    assert exit_code == 1
    assert len(caplog.records) == 1
    log_entry = json.loads(caplog.records[0].message.lstrip("invariance test failed:"))
    assert log_entry["molfile_id"] == "9261759198"
    assert log_entry["sdf"] == "mcule_20000.sdf.gz"
    assert log_entry["info"] == {"consumer": "invariance", "parameters": ""}
    assert log_entry["variants"] == ["A", "B"]


@pytest.mark.parametrize("consumer", [raising_consumer, segfaulting_consumer])
def test_core_raises(sdf_path, caplog, consumer):
    caplog.set_level(logging.ERROR, logger="sdf_pipeline")
    with pytest.raises(RuntimeError):
        for _ in core.run(
            sdf_path=str(sdf_path),
            consumer_function=partial(consumer, get_molfile_id=_get_mcule_id),
            number_of_consumer_processes=2,
            timeout_seconds_per_molfile=5,
        ):
            pass
    assert (
        caplog.records[0].message
        == f"could not process {sdf_path}: A process terminated unexpectedly."
    )
