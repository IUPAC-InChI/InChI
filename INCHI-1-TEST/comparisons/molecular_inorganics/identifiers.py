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
from collections import Counter

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


_COMPONENT = re.compile(r"^(\d*)([A-Z].*)$")
_ELEMENT_COUNT = re.compile(r"([A-Z][a-z]?)(\d*)")
_CONNECTION_TOKEN = re.compile(r"\d+|[(),]")
_REPEATED = re.compile(r"^(\d+)\*(.*)$")


def _component_atoms(formula: str) -> list[str]:
    """The elements of one component in canonical numbering order.

    InChI numbers the atoms of a component in formula order without hydrogen:
    carbon first, then alphabetically. A hydrogen that appears in `/c` (as
    Molecular Inorganics writes one bonded to a metal) comes after them."""
    counts = [(element, int(n or 1)) for element, n in _ELEMENT_COUNT.findall(formula)]
    heavy = sorted(
        ((e, n) for e, n in counts if e != "H"), key=lambda item: (item[0] != "C", item[0])
    )
    hydrogens = sum(n for e, n in counts if e == "H")

    return [e for e, n in heavy for _ in range(n)] + ["H"] * hydrogens


def _connections(layer: str) -> set[frozenset[int]]:
    """The bonds of one component's `/c` layer, as pairs of atom numbers."""
    bonds: set[frozenset[int]] = set()
    branches: list[int | None] = []
    previous: int | None = None
    for token in _CONNECTION_TOKEN.findall(layer):
        if token == "(":
            branches.append(previous)
        elif token == ",":
            previous = branches[-1]
        elif token == ")":
            previous = branches.pop()
        else:
            atom = int(token)
            if previous is not None:
                bonds.add(frozenset((previous, atom)))
            previous = atom

    return bonds


def _expand(items: list[str], pattern: re.Pattern) -> list[str]:
    expanded: list[str] = []
    for item in items:
        match = pattern.match(item)
        expanded += [match[2]] * int(match[1] or 1) if match else [item]

    return expanded


def most_bonds(body: str) -> int:
    """The largest number of neighbours any atom has in the `/c` layer of an
    InChI body (hydrogens in `/h` are not counted)."""
    connections = next(
        (layer[1:] for layer in body.split("/")[1:] if layer.startswith("c")), ""
    )
    most = 0
    for table in connections.split(";"):
        # Atom numbers restart in each component.
        degrees: Counter = Counter()
        for bond in _connections(_REPEATED.sub(r"\2", table)):
            degrees.update(bond)
        most = max(most, *degrees.values(), 0)

    return most


def metal_bonds(body: str) -> tuple[int, Counter] | None:
    """The component count and the bonds to metals of an InChI body.

    Bonds are counted per (metal, partner element) pair, so two InChIs of the
    same atoms can be compared although their canonical numbering differs.
    None when the formula and `/c` components do not line up."""
    layers = body.split("/")
    connections = next((layer[1:] for layer in layers[1:] if layer.startswith("c")), "")
    formulas = _expand(layers[0].split("."), _COMPONENT)
    tables = _expand(connections.split(";"), _REPEATED) if connections else []
    tables += [""] * (len(formulas) - len(tables))
    if len(tables) != len(formulas):
        return None

    bonds: Counter = Counter()
    for formula, table in zip(formulas, tables):
        atoms = _component_atoms(formula)
        for bond in _connections(table):
            pair = sorted(bond)
            if len(pair) != 2 or not all(0 < atom <= len(atoms) for atom in pair):
                return None
            first, second = (atoms[atom - 1] for atom in pair)
            if first in METALS:
                bonds[(first, second)] += 1
            if second in METALS and first != second:
                bonds[(second, first)] += 1

    return len(formulas), bonds
