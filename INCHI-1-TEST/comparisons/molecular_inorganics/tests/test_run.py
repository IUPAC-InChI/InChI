import json
import logging
import sqlite3
import pytest
from sdf_pipeline import core, drivers
from molecular_inorganics import run
from molecular_inorganics.comparators import LegacyComparator, OptionComparator
from molecular_inorganics.run import (
    compare_with_reference,
    get_args,
    log_filename,
    reference_filename,
    check_reference_format,
    select_comparator,
)


@pytest.fixture
def argv_base(tmp_path):
    lib = tmp_path / "libinchi.so"
    lib.write_text("")
    cfg = tmp_path / "config_x.py"
    cfg.write_text("")
    return [
        "--test=regression",
        f"--lib-path={lib}",
        f"--data-config={cfg}",
        "--run-tag=ref_v1_07_5",
        "--log-tag=run_b_mi",
    ]


def test_defaults(argv_base):
    args = get_args(argv_base)
    assert args.run_tag == "ref_v1_07_5"
    assert args.log_tag == "run_b_mi"
    assert args.inchi_api_parameters == ""
    assert args.compare == "prefix-insensitive"


def test_options_and_compare_mode(argv_base):
    args = get_args(
        argv_base
        + [
            "--inchi-api-parameters=-LegacyMetalHandling",
            "--compare=except-prefix",
        ]
    )
    assert args.inchi_api_parameters == "-LegacyMetalHandling"
    assert args.compare == "except-prefix"


@pytest.mark.parametrize("missing", ["--run-tag", "--log-tag"])
def test_tags_are_required(argv_base, missing):
    """An untagged pass would read or write the committed references."""
    argv = [arg for arg in argv_base if not arg.startswith(missing)]
    with pytest.raises(SystemExit):
        get_args(argv)


@pytest.mark.parametrize(
    "bad", ["--run-tag=../evil", "--log-tag=a/b", "--run-tag=", "--log-tag="]
)
def test_tags_reject_path_separators_and_empty_values(argv_base, bad):
    with pytest.raises(SystemExit):
        get_args(argv_base + [bad])


def test_compare_rejects_unknown_mode(argv_base):
    with pytest.raises(SystemExit):
        get_args(argv_base + ["--compare=whatever"])


def test_invariance_is_not_a_comparison_pass(argv_base):
    with pytest.raises(SystemExit):
        get_args(argv_base + ["--test=invariance"])


def test_filenames_are_namespaced_by_tag():
    assert reference_filename("Compound_000000001_000500000.sdf", "ref_v1_07_5") == (
        "Compound_000000001_000500000.sdf.ref_v1_07_5.regression_reference.sqlite"
    )
    assert log_filename("20260914T120000", "regression", "pubchem-compound", "run_b") == (
        "20260914T120000_regression_pubchem-compound.run_b.log"
    )


def _reference(path, *results):
    with sqlite3.connect(path) as db:
        db.execute("CREATE TABLE results (molfile_id TEXT, time TEXT, info TEXT, result TEXT)")
        db.executemany(
            "INSERT INTO results VALUES (?, '', '{}', ?)",
            [(str(i), json.dumps(result)) for i, result in enumerate(results)],
        )
    return path


class TestCheckReferenceFormat:
    CURRENT = {"inchi": "InChI=1S/CH4/h1H4", "key": "K", "exit": 0, "message": ""}

    def test_accepts_the_current_fields_after_a_timeout_row(self, tmp_path):
        check_reference_format(
            _reference(tmp_path / "x.sqlite", {"timeout_seconds": 60.0}, self.CURRENT)
        )

    def test_refuses_a_reference_without_the_message(self, tmp_path):
        """The 1 Oct references store inchi, key and exit only."""
        old = {k: v for k, v in self.CURRENT.items() if k != "message"}
        path = _reference(tmp_path / "Substance_1.sdf.ref_v1_07_5.regression_reference.sqlite", old)

        with pytest.raises(ValueError, match="delete it and re-run the reference pass"):
            check_reference_format(path)


def test_each_compare_mode_has_its_rule():
    assert isinstance(select_comparator("prefix-insensitive"), OptionComparator)
    assert isinstance(select_comparator("except-prefix"), LegacyComparator)


ETHANOL = "C2H6O/c1-2-3/h3H,2H2,1H3"


def _row(inchi, exit=0):
    return {"inchi": inchi, "key": "K", "exit": exit, "message": ""}


@pytest.fixture
def reference(tmp_path):
    return _reference(
        tmp_path / "reference.sqlite",
        _row(f"InChI=1S/{ETHANOL}"),    # molfile ID "0"
        _row("InChI=1S/CH4/h1H4"),       # molfile ID "1"
        _row("InChI=1S/H2O/h1H2"),       # molfile ID "2"
    )


def _fake_run(results):
    """Replace `core.run` with fixed results; `compare_with_reference` is tested
    without processes."""

    def _run(sdf_path, consumer_function, number_of_consumer_processes, **_):
        for molfile_id, result in results:
            if result is None:
                yield core.TimedOut(molfile_id=molfile_id, seconds=60.0)
            else:
                yield drivers.ConsumerResult(
                    molfile_id=molfile_id,
                    info=drivers.ConsumerInfo(consumer="test"),
                    result=result,
                )

    return _run


def _compare(tmp_path, reference, expected_failures=frozenset()):
    return compare_with_reference(
        sdf_path=tmp_path / "test.sdf.gz",
        reference_path=reference,
        consumer_function=lambda molfile, get_molfile_id: None,
        get_molfile_id=lambda molfile: "",
        number_of_consumer_processes=1,
        expected_failures=set(expected_failures),
        comparator=OptionComparator(),
    )


def test_compare_with_reference_logs_mismatches_and_timeouts(
    monkeypatch, tmp_path, reference, caplog
):
    monkeypatch.setattr(
        run.core,
        "run",
        _fake_run(
            [
                ("0", _row(f"InChI=1BB/{ETHANOL}")),   # prefix only: matches
                ("1", _row("InChI=1BB/CH3/h1H3")),    # body changed
                ("2", None),                          # timed out
            ]
        ),
    )
    caplog.set_level(logging.INFO, logger="sdf_pipeline")

    assert _compare(tmp_path, reference) == 1
    # The log lines classify.py parses.
    failed = [r.message for r in caplog.records if r.message.startswith("regression test failed:")]
    timed_out = [r.message for r in caplog.records if r.message.startswith("timed out:")]
    assert len(failed) == 1 and json.loads(failed[0].split(":", 1)[1])["molfile_id"] == "1"
    assert len(timed_out) == 1 and json.loads(timed_out[0].split(":", 1)[1])["molfile_id"] == "2"


def test_compare_with_reference_honours_expected_failures(monkeypatch, tmp_path, reference):
    monkeypatch.setattr(
        run.core,
        "run",
        _fake_run(
            [
                ("0", _row(f"InChI=1BB/{ETHANOL}")),
                ("1", _row("InChI=1BB/CH3/h1H3")),
                ("2", None),
            ]
        ),
    )

    assert _compare(tmp_path, reference, {"1", "2"}) == 0


def test_compare_with_reference_fails_on_a_molfile_it_never_saw(
    monkeypatch, tmp_path, reference
):
    monkeypatch.setattr(run.core, "run", _fake_run([("0", _row(f"InChI=1BB/{ETHANOL}"))]))

    with pytest.raises(AssertionError, match="haven't been processed"):
        _compare(tmp_path, reference)
