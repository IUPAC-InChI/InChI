"""Regression test for https://github.com/IUPAC-InChI/InChI/issues/105

An explicit zero valence (``VAL=-1`` in V3000, valence field ``15`` in V2000)
must not cause implicit hydrogens to be added. Before the fix, a terminal atom
carrying a single bond was treated as having a target valence of 15, so
``15 - 1 = 14`` hydrogens were "hallucinated" per atom, yielding
``InChI=1S/C2H28/c1-2/h1-2H14`` for ethane.

The correct behaviour is to honour the zero valence: no implicit hydrogens are
added while the drawn bond is kept, giving ``InChI=1S/C2/c1-2``.
"""

import pytest

from helpers import parse_inchi_from_executable_output

EXPECTED_INCHI = "InChI=1S/C2/c1-2"

ETHANE_V3000_ZERO_VALENCE = """
  ChemDraw04232514542D

  0  0  0     0  0              0 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 2 1 0 0 0
M  V30 BEGIN ATOM
M  V30 1 C 0.000000 0.000000 0.000000 0 VAL=-1
M  V30 2 C 1.000000 0.000000 0.000000 0 VAL=-1
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 1 1 1 2
M  V30 END BOND
M  V30 END CTAB
M  END
"""

ETHANE_V2000_ZERO_VALENCE = """
  test

  2  1  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 C   0  0  0  0  0 15  0  0  0  0  0  0
    1.0000    0.0000    0.0000 C   0  0  0  0  0 15  0  0  0  0  0  0
  1  2  1  0  0  0  0
M  END
"""


@pytest.mark.parametrize(
    "molfile",
    [ETHANE_V3000_ZERO_VALENCE, ETHANE_V2000_ZERO_VALENCE],
    ids=["v3000", "v2000"],
)
def test_zero_valence_does_not_hallucinate_hydrogens(molfile, run_inchi_exe):
    result = run_inchi_exe(molfile)
    inchi = parse_inchi_from_executable_output(result.output)

    assert inchi == EXPECTED_INCHI
