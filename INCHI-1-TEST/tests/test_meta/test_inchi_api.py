import ctypes

from inchi_tests.inchi_api import make_inchi_from_molfile_text, permute_molfile_text


def _inline_permutable_molfile() -> str:
    return """Cyclohexane
  test

  6  6  0  0  0  0  0  0  0  0999 V2000
    1.2148    0.7014    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000    1.4028    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -1.2148    0.7014    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -1.2148   -0.7014    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000   -1.4028    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    1.2148   -0.7014    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0  0  0  0
  2  3  1  0  0  0  0
  3  4  1  0  0  0  0
  4  5  1  0  0  0  0
  5  6  1  0  0  0  0
  6  1  1  0  0  0  0
M  END
"""


def test_permute_molfile_text_keeps_inchi_invariant_across_three_permutations(
    inchi_lib_path,
):
    original_molfile = _inline_permutable_molfile()
    inchi_lib = ctypes.CDLL(inchi_lib_path)
    libc = ctypes.CDLL(None)
    libc.srand(ctypes.c_uint(0))

    permuted_molfiles: list[str] = []
    permuted_inchis: list[str] = []

    for _ in range(3):
        permutation_exit_code, permuted_molfile = permute_molfile_text(
            inchi_lib, original_molfile
        )
        assert permutation_exit_code == 0
        permuted_molfiles.append(permuted_molfile)
        inchi_exit_code, inchi, _, _, _ = make_inchi_from_molfile_text(
            inchi_lib, permuted_molfile, ""
        )
        assert inchi_exit_code in [0, 1]
        permuted_inchis.append(inchi)

    assert len(set(permuted_molfiles)) == 3
    assert all(permuted_inchis)
    assert len(set(permuted_inchis)) == 1
