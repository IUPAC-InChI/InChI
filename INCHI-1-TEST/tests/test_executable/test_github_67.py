import pytest


@pytest.mark.xfail(strict=True, raises=AssertionError)
def test_inchi_to_struct(run_inchi_exe):
    # https://pubchem.ncbi.nlm.nih.gov/compound/10785244
    inchi = "InChI=1S/C18H16N4O3S/c1-12-2-8-15(9-3-12)26-11-17(24)21-13-4-6-14(7-5-13)22-18(25)16(23)10-20-19/h2-10,19,23H,11H2,1H3/p+1/b16-10+"
    result = run_inchi_exe(inchi, "-InChI2Struct")
    assert "Add/Remove protons error; *Conversion failed*" not in result.stderr
