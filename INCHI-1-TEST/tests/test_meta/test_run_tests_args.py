import sys
import pytest
from inchi_tests.utils import get_config_args, reference_filename, log_filename


@pytest.fixture
def argv_base(tmp_path):
    lib = tmp_path / "libinchi.so"
    lib.write_text("")
    cfg = tmp_path / "config_x.py"
    cfg.write_text("")
    return [
        "run_tests.py",
        "--test=regression",
        f"--lib-path={lib}",
        f"--data-config={cfg}",
    ]


def test_get_config_args_defaults(monkeypatch, argv_base):
    monkeypatch.setattr(sys, "argv", argv_base)
    test, _, _, parameters, run_tag, log_tag, compare = get_config_args()
    assert test == "regression"
    assert parameters == ""
    assert run_tag == ""
    assert log_tag == ""
    # Leniency is opt-in: an unqualified run compares byte-for-byte.
    assert compare == "exact"


def test_get_config_args_options_and_tags(monkeypatch, argv_base):
    monkeypatch.setattr(
        sys,
        "argv",
        argv_base
        + [
            "--inchi-api-parameters=-MolecularInorganics",
            "--run-tag=ref_1075",
            "--log-tag=run_b_dev_mi",
        ],
    )
    _, _, _, parameters, run_tag, log_tag, compare = get_config_args()
    assert parameters == "-MolecularInorganics"
    assert run_tag == "ref_1075"
    assert log_tag == "run_b_dev_mi"
    assert compare == "exact"


def test_run_tag_alone_does_not_loosen_comparison(monkeypatch, argv_base):
    """A tagged run is a campaign run, not automatically a lenient one.

    Run C is tagged -- it reads Run A's tagged reference -- but it is the control
    and must stay byte-for-byte, or it cannot see the prefix, InChIKey and warning
    drift it exists to catch."""
    monkeypatch.setattr(sys, "argv", argv_base + ["--run-tag=ref_1075"])
    *_, compare = get_config_args()
    assert compare == "exact"


def test_compare_mode_is_explicit(monkeypatch, argv_base):
    monkeypatch.setattr(
        sys, "argv", argv_base + ["--run-tag=ref_1075", "--compare=prefix-insensitive"]
    )
    *_, compare = get_config_args()
    assert compare == "prefix-insensitive"


def test_compare_rejects_unknown_mode(monkeypatch, argv_base):
    monkeypatch.setattr(sys, "argv", argv_base + ["--compare=whatever"])
    with pytest.raises(SystemExit):
        get_config_args()


def test_options_without_a_run_tag_are_rejected(monkeypatch, argv_base):
    """Options change the output but not the reference filename.

    Without a tag, `--test=regression-reference --inchi-api-parameters=...` would
    write the canonical reference that plain regression runs read."""
    monkeypatch.setattr(
        sys, "argv", argv_base + ["--inchi-api-parameters=-MolecularInorganics"]
    )
    with pytest.raises(SystemExit):
        get_config_args()


@pytest.mark.parametrize("bad", ["--run-tag=../evil", "--log-tag=a/b"])
def test_tags_reject_path_separators(monkeypatch, argv_base, bad):
    monkeypatch.setattr(sys, "argv", argv_base + [bad])
    with pytest.raises(SystemExit):
        get_config_args()


def test_filenames_are_namespaced_by_tag():
    assert reference_filename("Compound_000000001_000500000.sdf", "ref_1075") == (
        "Compound_000000001_000500000.sdf.ref_1075.regression_reference.sqlite"
    )
    assert reference_filename("Compound_000000001_000500000.sdf", "") == (
        "Compound_000000001_000500000.sdf.regression_reference.sqlite"
    )
    assert log_filename("20260914T120000", "regression", "pubchem-compound", "run_b") == (
        "20260914T120000_regression_pubchem-compound.run_b.log"
    )
    assert log_filename("20260914T120000", "regression", "ci", "") == (
        "20260914T120000_regression_ci.log"
    )
