"""Parts of an InChI, an InChIKey and a stored result that the comparison reads.

Since #280 the version prefix is a calculation-class designator, optionally
followed by the Molecular Inorganics subversion designator: `InChI=1S/`
(standard), `InChI=1N/` (non-standard, e.g. `-LegacyMetalHandling`) and
`InChI=1BB/` (a structure processed by Molecular Inorganics). v1.07.5 writes
`InChI=1S/` and, for non-standard output, `InChI=1/`. The InChIKey mirrors the
prefix in the last two characters of its second block, e.g. `…SA-N`, `…NA-N`,
`…BB-N`.
"""

import re

# Elements InChI treats as metals: the rows of `INCHI_BASE/src/eldata.c` whose
# type is METAL or METAL2, i.e. what `is_el_a_metal` returns true for. Not the
# textbook list: Ge, As and Te are not metals here, Sb, Po, Ts and Og are.
METALS = frozenset(
    "Li Be Na Mg Al K Ca Sc Ti V Cr Mn Fe Co Ni Cu Zn Ga Rb Sr Y Zr Nb Mo Tc Ru "
    "Rh Pd Ag Cd In Sn Sb Cs Ba La Ce Pr Nd Pm Sm Eu Gd Tb Dy Ho Er Tm Yb Lu Hf "
    "Ta W Re Os Ir Pt Au Hg Tl Pb Bi Po Fr Ra Ac Th Pa U Np Pu Am Cm Bk Cf Es Fm "
    "Md No Lr Rf Db Sg Bh Hs Mt Ds Rg Cn Nh Fl Mc Lv Ts Og".split()
)
_ELEMENT = re.compile(r"[A-Z][a-z]?")


def inchi_prefix(inchi: str) -> str:
    """The version prefix of an InChI, e.g. `InChI=1S`; empty if there is none."""
    return inchi.split("/", 1)[0] if "/" in inchi else ""


def inchi_body(inchi: str) -> str:
    """The InChI without its version prefix; empty if there is none."""
    return inchi.split("/", 1)[1] if "/" in inchi else ""


def key_without_flags(key: str) -> str:
    """The InChIKey without the two characters that mirror the version prefix.

    The hash blocks are computed from the InChI without its prefix, so they stay
    the same when only the prefix changes. A failed run has no key; it is
    returned as is."""
    if len(key) != 27:
        return key

    return key[:23] + key[25:]


def is_failed(result: dict) -> bool:
    """Whether a result is a failure rather than a warning.

    `inchi_api.h`: `inchi_Ret_WARNING = 1`, `inchi_Ret_ERROR = 2`. A metal
    disconnection always warns, so exit code 1 is common on structures with
    metals and is not a failure. A reference row for a molfile that timed out
    stores only `timeout_seconds` and counts as failed."""
    return result.get("exit", 2) >= 2 or not result.get("inchi")


def formula_elements(inchi: str) -> set[str]:
    """The element symbols in the formula layer of an InChI."""
    body = inchi_body(inchi)
    if not body:
        return set()

    return set(_ELEMENT.findall(body.split("/", 1)[0]))


def has_metal(inchi: str) -> bool:
    return not formula_elements(inchi).isdisjoint(METALS)
