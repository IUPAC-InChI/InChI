import pytest
from pathlib import Path
from helpers import parse_inchi_from_executable_output
from sdf_pipeline.utils import read_records_from_gzipped_sdf
import tempfile


@pytest.fixture
def sdf_path():
    return (
        Path(__file__)
        .parent.absolute()
        .joinpath("data/organometallic_structures_pubchem.sdf.gz")
    )


def test_organometallics_pubchem(sdf_path, run_inchi_exe):
    exectuable_results = [
        run_inchi_exe(mol, "-RecMet") for mol in read_records_from_gzipped_sdf(sdf_path)
    ]
    inchis = [
        parse_inchi_from_executable_output(result.output)
        for result in exectuable_results
    ]
    assert all(inchis)


@pytest.mark.skip(reason="used to get statistics and insights")
def test_organometallics_pubchem_statistics(sdf_path, run_inchi_exe):
    error_ids = []
    inchis = {}
    inchi_ids = []
    error_mols = []
    errors = {}
    mol_counter = 0

    for mol in read_records_from_gzipped_sdf(sdf_path):
        mol_counter += 1
        result = run_inchi_exe(mol, "-RecMet")
        inchi = parse_inchi_from_executable_output(result.output)

        id = mol.split("\n")[0]
        if inchi == "":
            error_ids.append(id)
            error_mols.append(mol)
            for line in result.stderr.splitlines():
                if line.startswith("Error"):
                    errors[id] = line
        else:
            inchi_ids.append(id)
            inchis[id] = inchi

    with open(f"{tempfile.gettempdir()}/inchis.txt", "w") as f:
        for id, inchi in inchis.items():
            f.write(f"{id}: {inchi}\n")

    with open(f"{tempfile.gettempdir()}/inchi_ids.txt", "w") as f:
        for id in inchi_ids:
            f.write(f"{id}\n")

    with open(f"{tempfile.gettempdir()}/error_ids.txt", "w") as f:
        for id in error_ids:
            f.write(f"{id}\n")

    with open(f"{tempfile.gettempdir()}/error_mols.sdf", "w") as f:
        for mol in error_mols:
            f.write(f"{mol}$$$$\n")

    with open(f"{tempfile.gettempdir()}/errors.txt", "w") as f:
        for id, error in errors.items():
            f.write(f"{id}: {error}\n")
    print(f"{len(inchis)}/{mol_counter} InChIs converted.")
