from helpers import parse_aux_info_from_executable_output

# A charge-separated metal salt with 2D coordinates. Molecular Inorganics mode
# disconnects the Na-Cl bond, but the AuxInfo reversibility layers (/rA, /rB,
# /rC) must still describe the original input, including the atom coordinates.
NACL_MOL = """
  Test

  2  1  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 Na  0  0  0  0  0  0  0  0  0  0  0  0
    1.5000    0.0000    0.0000 Cl  0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0  0  0  0
M  END
"""


def test_molecular_inorganics_auxinfo_has_coordinates(run_inchi_exe):
    result = run_inchi_exe(NACL_MOL, "-MolecularInorganics")
    aux_info = parse_aux_info_from_executable_output(result.output)

    assert aux_info, "Molecular Inorganics run produced no AuxInfo."
    assert "/rA:" in aux_info, f"AuxInfo missing atom (/rA) layer: {aux_info!r}"
    assert "/rC:" in aux_info, f"AuxInfo missing coordinate (/rC) layer: {aux_info!r}"
    # The input coordinate of the Cl atom must be preserved.
    assert "1.5,0,0" in aux_info, f"AuxInfo missing input coordinates: {aux_info!r}"
