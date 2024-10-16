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


@pytest.mark.xfail(strict=True, raises=AssertionError)
def test_empty_bondblock(molfile_empty_bondblock, run_inchi_exe):
    result = run_inchi_exe(molfile_empty_bondblock)

    assert (
        "Error 71 (no InChI; Error: No V3000 CTAB end marker) inp structure #1."
        not in result.stdout
    )


def test_no_bondblock(molfile_no_bondblock, run_inchi_exe):
    result = run_inchi_exe(molfile_no_bondblock)

    assert (
        "Error 71 (no InChI; Error: No V3000 CTAB end marker) inp structure #1."
        not in result.stdout
    )
