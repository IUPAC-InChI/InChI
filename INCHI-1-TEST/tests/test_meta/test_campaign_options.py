import pytest
from pathlib import Path
from inchi_tests.consumers import campaign_regression_consumer, inchi_body
from inchi_tests.campaign import reconnected_layer

DEV_LIB = "CMake_build/full_build/INCHI-1-SRC/INCHI_API/libinchi/src/lib/libinchi.so"
V1075_LIB = (
    "../inchi-v1075/CMake_build/full_build/INCHI-1-SRC/INCHI_API/libinchi/src/lib/libinchi.so"
)

# Pt(en)Cl2: a chelate whose ring carries 4 C/N atoms, satisfying the ring
# criterion at strutil.c:4510, so -MolecularInorganics keeps the metal bonded.
PTEN_MOLFILE = """Pt(en)Cl2
  test

  7  7  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 C   0  0
    1.0000    0.0000    0.0000 C   0  0
   -0.5000    0.8660    0.0000 N   0  0
    1.5000    0.8660    0.0000 N   0  0
    0.0000    2.5000    0.0000 Cl  0  0
    2.0000    2.5000    0.0000 Cl  0  0
    0.5000    1.7320    0.0000 Pt  0  0
  1  2  1  0
  1  3  1  0
  2  4  1  0
  3  7  1  0
  4  7  1  0
  5  7  1  0
  6  7  1  0
M  END
$$$$
"""

ETHANOL_MOLFILE = """ethanol
  test

  3  2  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 C   0  0
    1.0000    0.0000    0.0000 C   0  0
    2.0000    0.0000    0.0000 O   0  0
  1  2  1  0
  2  3  1  0
M  END
$$$$
"""


def _result(lib, molfile, options):
    path = Path(lib)
    if not path.is_file():
        pytest.skip(f"{lib} not built; see Task 8 Step 1")
    return campaign_regression_consumer(
        molfile,
        get_molfile_id=lambda m: m.splitlines()[0].strip(),
        inchi_lib_path=str(path),
        inchi_api_parameters=options,
    ).result


def test_molecular_inorganics_keeps_the_metal_bonded():
    plain = _result(DEV_LIB, PTEN_MOLFILE, "")
    molecular_inorganics = _result(DEV_LIB, PTEN_MOLFILE, "-MolecularInorganics")
    assert plain["inchi"] == (
        "InChI=1S/C2H6N2.2ClH.Pt/c3-1-2-4;;;/h3-4H,1-2H2;2*1H;/q-2;;;+4/p-2"
    )
    assert plain["exit"] == 1  # warning: "Metal was disconnected"
    assert molecular_inorganics["inchi"] == (
        "InChI=1B/C2H6Cl2N2Pt/c3-7(4)5-1-2-6-7/h5-6H,1-2H2"
    )
    assert molecular_inorganics["exit"] == 0


def test_recmet_reconnected_layer_matches_molecular_inorganics():
    recmet = _result(V1075_LIB, PTEN_MOLFILE, "-RecMet")["inchi"]
    molecular_inorganics = _result(DEV_LIB, PTEN_MOLFILE, "-MolecularInorganics")["inchi"]
    assert "/r" in recmet
    # The campaign's central equality: RecMet's /r layer vs the MI InChI sans prefix.
    assert reconnected_layer(recmet) == inchi_body(molecular_inorganics)


def test_molecular_inorganics_changes_only_the_prefix_for_a_metal_free_structure():
    plain = _result(DEV_LIB, ETHANOL_MOLFILE, "")
    molecular_inorganics = _result(DEV_LIB, ETHANOL_MOLFILE, "-MolecularInorganics")
    assert plain["inchi"] != molecular_inorganics["inchi"]
    assert plain["key"] != molecular_inorganics["key"]
    assert inchi_body(plain["inchi"]) == inchi_body(molecular_inorganics["inchi"])


def test_dev_matches_v1075_without_options():
    for molfile in (ETHANOL_MOLFILE, PTEN_MOLFILE):
        assert _result(DEV_LIB, molfile, "") == _result(V1075_LIB, molfile, "")
