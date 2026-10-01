import pytest
from inchi_tests.comparators import PrefixInsensitiveComparator
from molecular_inorganics.run import (
    get_args,
    log_filename,
    reference_filename,
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
        "--log-tag=run_b_dev_mi",
    ]


def test_defaults(argv_base):
    args = get_args(argv_base)
    assert args.run_tag == "ref_v1_07_5"
    assert args.log_tag == "run_b_dev_mi"
    assert args.inchi_api_parameters == ""
    # Leniency is opt-in: an unqualified pass compares byte-for-byte.
    assert args.compare == "exact"
    assert args.timeout_seconds_per_molfile == 60


def test_options_and_compare_mode(argv_base):
    args = get_args(
        argv_base
        + [
            "--inchi-api-parameters=-MolecularInorganics",
            "--compare=prefix-insensitive",
        ]
    )
    assert args.inchi_api_parameters == "-MolecularInorganics"
    assert args.compare == "prefix-insensitive"


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


class TestComparatorSelection:
    """`--compare` alone decides leniency; `--run-tag` never does.

    Pass C reads pass A's tagged reference and is the control that licenses
    attributing pass B's differences to the option. If it compared leniently it
    could not see a changed prefix, InChIKey or warning level and would certify a
    drifting build."""

    def test_default_is_byte_for_byte(self):
        assert select_comparator("regression", "exact") is None

    def test_prefix_insensitive_is_opt_in(self):
        assert isinstance(
            select_comparator("regression", "prefix-insensitive"),
            PrefixInsensitiveComparator,
        )

    def test_reference_pass_never_compares(self):
        # A comparator here would log an all-zero summary that the report would read.
        assert select_comparator("regression-reference", "prefix-insensitive") is None
