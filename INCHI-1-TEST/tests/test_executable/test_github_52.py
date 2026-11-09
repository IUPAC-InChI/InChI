import pytest


@pytest.fixture
def molfile_empty_bondblock():
    return """
  -INDIGO-08292417452D

  0  0  0  0  0  0  0  0  0  0  0 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 1 0 0 0 0
M  V30 BEGIN ATOM
M  V30 1 C 9.35 -4.8 0.0 0
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 END BOND
M  V30 END CTAB
M  END
"""


@pytest.fixture
def molfile_no_bondblock():
    return """
  -INDIGO-08292417452D

  0  0  0  0  0  0  0  0  0  0  0 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 1 0 0 0 0
M  V30 BEGIN ATOM
M  V30 1 C 9.35 -4.8 0.0 0
M  V30 END ATOM
M  V30 END CTAB
M  END
"""


def test_empty_bondblock(molfile_empty_bondblock, run_inchi_exe):
    result = run_inchi_exe(molfile_empty_bondblock)

    inchi_lines = [
        line for line in result.output.splitlines() if line.startswith("InChI=")
    ]

    assert inchi_lines == ["InChI=1S/CH4/h1H4"]

    assert (
        "Error 71 (no InChI; Error: No V3000 CTAB end marker) inp structure #1."
        not in result.stderr
    )


def test_no_bondblock(molfile_no_bondblock, run_inchi_exe):
    result = run_inchi_exe(molfile_no_bondblock)

    inchi_lines = [
        line for line in result.output.splitlines() if line.startswith("InChI=")
    ]

    assert inchi_lines == ["InChI=1S/CH4/h1H4"]

    assert (
        "Error 71 (no InChI; Error: No V3000 CTAB end marker) inp structure #1."
        not in result.stderr
    )


def test_nonempty_bondblock_with_zero_bond_count(
    molfile_empty_bondblock, run_inchi_exe
):
    molfile = (
        molfile_empty_bondblock.replace(
            "M  V30 COUNTS 1 0 0 0 0", "M  V30 COUNTS 2 0 0 0 0"
        )
        .replace("M  V30 END ATOM", "M  V30 2 C 10.35 -4.8 0.0 0\nM  V30 END ATOM")
        .replace("M  V30 END BOND", "M  V30 1 1 1 2\nM  V30 END BOND")
    )

    result = run_inchi_exe(molfile)

    assert "No V3000 Bond block end marker" in result.stderr
    assert not any(line.startswith("InChI=") for line in result.output.splitlines())
