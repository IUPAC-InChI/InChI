import gzip
import random
from pathlib import Path
from typing import Generator, Callable

try:
    # Optional import of RDKit.
    from rdkit import Chem
    from rdkit import RDLogger

    # Suppress RDKit console output.
    RDLogger.DisableLog("rdApp.*")

except ImportError:
    pass


def read_records_from_gzipped_sdf(sdf_path: Path) -> Generator[str, None, None]:
    # https://en.wikipedia.org/wiki/Chemical_table_file#SDF"
    current_record = ""
    # TODO: guard file opening.
    with gzip.open(sdf_path, "rb") as gzipped_sdf:
        # Decompress SDF line-by-line to avoid loading entire SDF into memory.
        for decompressed_line in gzipped_sdf:
            decoded_line = decompressed_line.decode("utf-8", "backslashreplace")
            current_record += decoded_line
            if decoded_line.strip() == "$$$$":
                # TODO: harden SDF parsing according to
                # http://www.dalkescientific.com/writings/diary/archive/2020/09/18/handling_the_sdf_record_delimiter.html.
                yield current_record
                current_record = ""

    return None


def select_records_from_gzipped_sdf(
    sdf_path: Path, molfile_ids: set[str], get_molfile_id: Callable
) -> Generator[tuple[str, str], None, None]:

    for molfile in read_records_from_gzipped_sdf(sdf_path):
        molfile_id = get_molfile_id(molfile)
        if molfile_id in molfile_ids:
            yield molfile_id, molfile

    return None


def _permute(indices: list[int]) -> list[int]:
    # Shuffle without in-place mutation.
    # See https://docs.python.org/3/library/random.html#random.shuffle.
    return random.sample(indices, len(indices))


def permute_molblock(molblock: str) -> str | None:
    mol = Chem.MolFromMolBlock(
        molblock, sanitize=False, removeHs=False, strictParsing=False
    )
    if mol is None:
        return None

    atom_indices = [atom.GetIdx() for atom in mol.GetAtoms()]
    if not atom_indices:
        return None

    atom_indices_permuted = _permute(atom_indices)
    if len(atom_indices) > 1:
        # Enforce different permutation of atom indices.
        while atom_indices_permuted == atom_indices:
            atom_indices_permuted = _permute(atom_indices)
    mol_permuted = Chem.RenumberAtoms(mol, atom_indices_permuted)

    return Chem.MolToMolBlock(mol_permuted, kekulize=False)
