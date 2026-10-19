import pytest
from pathlib import Path
from molecular_inorganics.classify import reconnected_layer
from molecular_inorganics.comparators import LegacyComparator
from molecular_inorganics.consumers import raw_regression_consumer
from molecular_inorganics.identifiers import inchi_body

DEV_LIB = "CMake_build/full_build/INCHI-1-SRC/INCHI_API/libinchi/src/lib/libinchi.so"
# The baseline worktree path `run_comparison.sh` derives from BASELINE_TAG. Keep
# these in step, or these cross-version tests silently skip instead of running.
BASELINE_TAG = "v1.07.5"
V1075_LIB = (
    f"../inchi-{BASELINE_TAG}/CMake_build/full_build"
    "/INCHI-1-SRC/INCHI_API/libinchi/src/lib/libinchi.so"
)

# Pt(en)Cl2: a chelate that Molecular Inorganics keeps bonded to the metal, while
# v1.07.5 and -LegacyMetalHandling disconnect it.
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
        pytest.skip(
            f"{lib} not built; run "
            "./INCHI-1-TEST/comparisons/molecular_inorganics/run_comparison.sh to create it"
        )
    return raw_regression_consumer(
        molfile,
        get_molfile_id=lambda m: m.splitlines()[0].strip(),
        inchi_lib_path=str(path),
        inchi_api_parameters=options,
    ).result


PTEN_MI = "InChI=1BB/C2H6Cl2N2Pt/c3-7(4)5-1-2-6-7/h5-6H,1-2H2"
PTEN_DISCONNECTED = "C2H6N2.2ClH.Pt/c3-1-2-4;;;/h3-4H,1-2H2;2*1H;/q-2;;;+4/p-2"


def test_molecular_inorganics_is_the_default_and_keeps_the_metal_bonded():
    default = _result(DEV_LIB, PTEN_MOLFILE, "")
    assert default == _result(DEV_LIB, PTEN_MOLFILE, "-MolecularInorganics")
    assert default["inchi"] == PTEN_MI
    assert default["key"].endswith("BB-N")
    assert default["exit"] == 0


def test_legacy_handling_disconnects_the_metal_like_v1075():
    legacy = _result(DEV_LIB, PTEN_MOLFILE, "-LegacyMetalHandling")
    baseline = _result(V1075_LIB, PTEN_MOLFILE, "")
    assert legacy["inchi"] == f"InChI=1N/{PTEN_DISCONNECTED}"
    assert baseline["inchi"] == f"InChI=1S/{PTEN_DISCONNECTED}"
    assert legacy["message"] == "Metal was disconnected; Proton(s) added/removed"
    # Run C's rule: the same apart from the prefix and the key flags.
    assert LegacyComparator()(legacy, baseline) is True


def test_recmet_reconnected_layer_matches_molecular_inorganics():
    recmet = _result(V1075_LIB, PTEN_MOLFILE, "-RecMet")["inchi"]
    molecular_inorganics = _result(DEV_LIB, PTEN_MOLFILE, "")["inchi"]
    assert "/r" in recmet
    # The comparison's central equality: RecMet's /r layer vs the MI InChI without prefix.
    assert reconnected_layer(recmet) == inchi_body(molecular_inorganics)


def test_a_structure_without_a_metal_is_unchanged_by_default():
    assert _result(DEV_LIB, ETHANOL_MOLFILE, "") == _result(V1075_LIB, ETHANOL_MOLFILE, "")


def test_legacy_handling_only_changes_the_prefix_of_a_structure_without_a_metal():
    legacy = _result(DEV_LIB, ETHANOL_MOLFILE, "-LegacyMetalHandling")
    assert legacy["inchi"] == "InChI=1N/C2H6O/c1-2-3/h3H,2H2,1H3"
    assert legacy["key"] == "LFQSCWFLJHTTHZ-UHFFFAOYNA-N"
    assert LegacyComparator()(legacy, _result(V1075_LIB, ETHANOL_MOLFILE, "")) is True
