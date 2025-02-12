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

@pytest.mark.xfail(strict=True, raises=AssertionError)
def test_Cyclopentadiene_anion(molfile_Cyclopentadiene_anion, run_inchi_exe):
    result = run_inchi_exe(molfile_Cyclopentadiene_anion)
    assert (
        "InChI=1S/C5H5/c1-2-4-5-3-1/h1-5H/q-1"
        == parse_inchi_from_executable_output(result.output)
    )

@pytest.mark.xfail(strict=True, raises=AssertionError)
def test_Cycloheptatriene_cation(molfile_Cycloheptatriene_cation, run_inchi_exe):
    result = run_inchi_exe(molfile_Cycloheptatriene_cation)
    assert (
        "InChI=1S/C7H7/c1-2-4-6-7-5-3-1/h1-7H/q+1"
        == parse_inchi_from_executable_output(result.output)
    )

@pytest.mark.xfail(strict=True, raises=AssertionError)
def test_Cyclopropene_cation(molfile_Cyclopropene_cation, run_inchi_exe):
    result = run_inchi_exe(molfile_Cyclopropene_cation)
    assert (
        "InChI=1S/C3H3/c1-2-3-1/h1-3H/q+1"
        == parse_inchi_from_executable_output(result.output)
    )
