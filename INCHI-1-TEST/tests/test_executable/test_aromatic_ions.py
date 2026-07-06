import pytest
from helpers import parse_inchi_from_executable_output


@pytest.fixture
def molfile_Cyclopentadiene_anion():
    return """
  ChemDraw02042509422D

  5  5  0  0  0  0  0  0  0  0999 V2000
   -0.6348    0.4125    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -0.6348   -0.4125    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.1498   -0.6674    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.6348    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.1498    0.6674    0.0000 C   0  5  0  0  0  0  0  0  0  0  0  0
  1  2  4  0
  2  3  4  0
  3  4  4  0
  4  5  4  0
  5  1  4  0
M  CHG  1   5  -1
M  END
    """


@pytest.fixture
def molfile_Cycloheptatriene_cation():
    return """
  ChemDraw02042509542D

  7  7  0  0  0  0  0  0  0  0999 V2000
   -0.9036    0.4125    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -0.9036   -0.4125    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -0.2586   -0.9269    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.5457   -0.7433    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.9036    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.5457    0.7433    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -0.2586    0.9269    0.0000 C   0  3  0  0  0  0  0  0  0  0  0  0
  1  2  4  0
  2  3  4  0
  3  4  4  0
  4  5  4  0
  5  6  4  0
  6  7  4  0
  7  1  4  0
M  CHG  1   7   1
M  END
"""


@pytest.fixture
def molfile_Cyclopropene_cation():
    return """
  ChemDraw02042509402D

  3  3  0  0  0  0  0  0  0  0999 V2000
   -0.3572    0.4125    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -0.3572   -0.4125    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.3572    0.0000    0.0000 C   0  3  0  0  0  0  0  0  0  0  0  0
  1  2  4  0
  2  3  4  0
  3  1  4  0
M  CHG  1   3   1
M  END
"""


def test_Cyclopentadiene_anion(molfile_Cyclopentadiene_anion, run_inchi_exe):
    result = run_inchi_exe(molfile_Cyclopentadiene_anion)

    assert "Cannot process aromatic bonds" not in result.log
    assert "InChI=1S/C5H5/c1-2-4-5-3-1/h1-5H/q-1" == parse_inchi_from_executable_output(
        result.output
    )


def test_Cycloheptatriene_cation(molfile_Cycloheptatriene_cation, run_inchi_exe):
    result = run_inchi_exe(molfile_Cycloheptatriene_cation)

    assert "Cannot process aromatic bonds" not in result.log
    assert (
        "InChI=1S/C7H7/c1-2-4-6-7-5-3-1/h1-7H/q+1"
        == parse_inchi_from_executable_output(result.output)
    )


def test_Cyclopropene_cation(molfile_Cyclopropene_cation, run_inchi_exe):
    result = run_inchi_exe(molfile_Cyclopropene_cation)

    assert "Cannot process aromatic bonds" not in result.log
    assert "InChI=1S/C3H3/c1-2-3-1/h1-3H/q+1" == parse_inchi_from_executable_output(
        result.output
    )


@pytest.fixture
def molfile_Pyridinium():
    return """
  ChemDraw

  6  6  0  0  0  0  0  0  0  0999 V2000
   -0.7145    0.4125    0.0000 N   0  0  0  0  0  0  0  0  0  0  0  0
   -0.7145   -0.4125    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000   -0.8250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.7145   -0.4125    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.7145    0.4125    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000    0.8250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  4  0
  2  3  4  0
  3  4  4  0
  4  5  4  0
  5  6  4  0
  6  1  4  0
M  CHG  1   1   1
M  END
"""


def test_Pyridinium_unchanged(molfile_Pyridinium, run_inchi_exe):
    result = run_inchi_exe(molfile_Pyridinium)

    assert "Cannot process aromatic bonds" not in result.log
    assert "InChI=1S/C5H5N/c1-2-4-6-5-3-1/h1-5H/p+1" == parse_inchi_from_executable_output(
        result.output
    )


@pytest.fixture
def molfile_Cyclopentadienyl_radical():
    return """
  ChemDraw

  5  5  0  0  0  0  0  0  0  0999 V2000
   -0.6348    0.4125    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -0.6348   -0.4125    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.1498   -0.6674    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.6348    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.1498    0.6674    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  4  0
  2  3  4  0
  3  4  4  0
  4  5  4  0
  5  1  4  0
M  RAD  1   1   2
M  END
"""


@pytest.mark.xfail(
    strict=True,
    reason="#154: a neutral aromatic radical (cyclopentadienyl radical) fails inside "
    "the pre-existing FIX_AROM_RADICAL path, before the charged-ion relaxation runs. "
    "Untangling radical handling is out of scope for the charged-ion fix and needs the "
    "aromaticity rework.",
)
def test_Cyclopentadienyl_radical(molfile_Cyclopentadienyl_radical, run_inchi_exe):
    result = run_inchi_exe(molfile_Cyclopentadienyl_radical)

    assert "Cannot process aromatic bonds" not in result.log
    inchi = parse_inchi_from_executable_output(result.output)
    assert inchi == "InChI=1S/C5H5/c1-2-4-5-3-1/h1-5H"


@pytest.fixture
def molfile_Ferrocene_heavy_atoms():
    # Two cyclopentadienyl rings (heavy atoms only) eta5-coordinated to Fe via
    # type-9 (coordinative) bonds. Aromatic ring bonds are type 4.
    return """
  ChemDraw

 11 20  0  0  0  0  0  0  0  0999 V2000
    0.0000    1.2000    0.0000 Fe  0  0  0  0  0  0  0  0  0  0  0  0
   -0.9511    2.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -0.5878    2.9000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.5878    2.9000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.9511    2.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000    1.4000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -0.9511    0.4000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -0.5878   -0.5000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.5878   -0.5000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.9511    0.4000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000    1.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  2  3  4  0
  3  4  4  0
  4  5  4  0
  5  6  4  0
  6  2  4  0
  7  8  4  0
  8  9  4  0
  9 10  4  0
 10 11  4  0
 11  7  4  0
  1  2  9  0
  1  3  9  0
  1  4  9  0
  1  5  9  0
  1  6  9  0
  1  7  9  0
  1  8  9  0
  1  9  9  0
  1 10  9  0
  1 11  9  0
M  END
"""


@pytest.mark.xfail(
    strict=True,
    reason="#82: ferrocene Cp carbons have metal bonds; organometallic pi-coordination "
    "is out of scope for the charged-pi-center fix and needs the aromaticity rework.",
)
def test_Ferrocene_heavy_atoms(molfile_Ferrocene_heavy_atoms, run_inchi_exe):
    result = run_inchi_exe(molfile_Ferrocene_heavy_atoms)

    assert "Cannot process aromatic bonds" not in result.log


def _molfile_methyl_cyclopentadienyl_anion(charged_atom):
    """Methylcyclopentadienyl anion [C5H4-CH3]- with the -1 formal charge placed on
    ring atom `charged_atom` (1-5; atom 1 carries the methyl). Every placement denotes
    the same delocalized anion, so all must yield one identical, placement-independent
    InChI (schatzsc, PR #234)."""
    return f"""
  test

  6  6  0  0  0  0  0  0  0  0999 V2000
   -0.6348    0.4125    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -0.6348   -0.4125    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.1498   -0.6674    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.6348    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.1498    0.6674    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -1.4200    0.8000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  4  0
  2  3  4  0
  3  4  4  0
  4  5  4  0
  5  1  4  0
  1  6  1  0
M  CHG  1   {charged_atom}  -1
M  END
"""


@pytest.mark.xfail(
    strict=True,
    reason="#154 (schatzsc on PR #234): the charged-ion relaxation acts on the specific "
    "atom charged in the molfile, before InChI's mobile-charge normalization, so the "
    "result is placement-dependent (and -9986s for most placements). A delocalized "
    "anion must give one placement-independent InChI; needs the aromaticity rework.",
)
def test_MethylCyclopentadienyl_anion_charge_placement_invariance(run_inchi_exe):
    """All -1 placements on the methylcyclopentadienyl ring are the same molecule and
    must produce a single identical InChI (no hallucinated isomers, no -9986)."""
    inchis = set()
    for charged_atom in (1, 2, 3, 4, 5):
        result = run_inchi_exe(_molfile_methyl_cyclopentadienyl_anion(charged_atom))
        assert "Cannot process aromatic bonds" not in result.log, (
            f"charge on ring atom {charged_atom} failed with -9986"
        )
        inchis.add(parse_inchi_from_executable_output(result.output))
    assert len(inchis) == 1, f"charge placement changed the InChI: {sorted(inchis)}"
