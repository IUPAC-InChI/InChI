import pytest
from inchi_tests import consumers


@pytest.fixture
def stable_molfile():
    return """
  test

  2  1  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 C   0  0  0  0  0 15  0  0  0  0  0  0
    1.0000    0.0000    0.0000 C   0  0  0  0  0 15  0  0  0  0  0  0
  1  2  1  0  0  0  0
M  END
"""


def test_invariance_consumer_separates_errors_from_successful_variants(
    inchi_lib_path, stable_molfile
):
    result = consumers.invariance_consumer(
        stable_molfile,
        lambda _: "id",
        inchi_lib_path,
        "",
        10,
    )

    assert result.error == ""
    assert len(result.result["variants"]) == 1
    variant = result.result["variants"][0]
    assert variant["inchi"].startswith("InChI=1")
    assert variant["key"]
    assert "M  END" in variant["molfile"]


def test_invariance_consumer_collects_all_repeated_errors_without_variants(
    inchi_lib_path,
):
    result = consumers.invariance_consumer(
        "not a molfile",
        lambda _: "id",
        inchi_lib_path,
        "",
        10,
    )

    errors = result.error.splitlines()
    assert len(errors) == 10
    assert len(set(errors)) == 1
    assert result.result["variants"] == []


def test_invariance_consumer_preserves_successful_variants(
    inchi_lib_path, stable_molfile
):
    first = consumers.invariance_consumer(
        stable_molfile,
        lambda _: "id",
        inchi_lib_path,
        "",
        10,
    )
    second = consumers.invariance_consumer(
        stable_molfile,
        lambda _: "id",
        inchi_lib_path,
        "",
        10,
    )

    assert first.error == ""
    assert second.error == ""
    assert first.result["variants"] == second.result["variants"]
