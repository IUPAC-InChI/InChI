"""Regression test for https://github.com/IUPAC-InChI/InChI/issues/276."""

from helpers import parse_inchi_from_executable_output


EXPECTED_INCHI = (
    "InChI=1S/C10H16O2/c1-8-3-5-9(6-4-8)7-10(11)12-2/"
    "h7-8H,3-6H2,1-2H3"
)


MOLFILE = """\
RDKit          2D


 12 12  0  0  0  0  0  0  0  0999 V2000
   -4.8794    0.4883    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -3.7700   -0.5213    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0
   -2.3410   -0.0652    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -2.0214    1.4003    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0
   -1.2315   -1.0748    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.1975   -0.6188    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    1.3069   -1.6283    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    2.7359   -1.1723    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    3.0555    0.2932    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    4.4845    0.7493    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    1.9460    1.3028    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.5170    0.8468    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0
  2  3  1  0
  3  4  2  0
  3  5  1  0
  5  6  2  0
  6  7  1  0
  7  8  1  0
  8  9  1  0
  9 10  1  0
  9 11  1  0
 11 12  1  0
 12  6  1  0
M  END
"""


def test_coordinate_only_symmetric_ylidene_is_not_stereogenic(
    run_inchi_exe,
):
    result = run_inchi_exe(MOLFILE, "NoLabels NoWarnings AuxNone")

    assert parse_inchi_from_executable_output(result.output) == EXPECTED_INCHI
