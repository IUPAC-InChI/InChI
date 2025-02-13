import pytest
import os
import os.path
from pathlib import Path
from helpers import parse_inchi_from_executable_output
import sys
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '../../src')))
from sdf_pipeline.utils import read_records_from_gzipped_sdf

@pytest.fixture
def sdf_path():
    return Path(__file__).parent.absolute().joinpath("data/AlexClark_structures_CCDC.sdf.gz")

# code commented out is used to get statistics and insights 
def test_organometallics_ccdc(sdf_path, run_inchi_exe):
    # error_ids = []
    # inchis = {}
    # inchi_ids = []
    # error_mols = []
    # errors = {}
    mol_counter = 0
    inchi_counter = 0

    for mol in read_records_from_gzipped_sdf(sdf_path):
        mol_counter += 1
        mol = mol.lstrip()
        result = run_inchi_exe(mol, "-RecMet")
        inchi = parse_inchi_from_executable_output(result.output)

        if inchi != "":
            inchi_counter += 1 

        # id = mol.split("\n")[0]
        # if inchi == "":
        #     error_ids.append(id)
        #     error_mols.append(mol)
        #     for line in result.stderr.splitlines():
        #         if line.startswith("Error"):
        #             errors[id] = line
        # else:
        #     inchi_ids.append(id)
        #     inchis[id] = inchi

    # with open(f'{tempfile.gettempdir()}/inchis.txt', 'w') as f:
    #     for id, inchi in inchis.items():
    #         f.write(f"{id}: {inchi}")

    # with open(f'{tempfile.gettempdir()}/inchi_ids.txt', 'w') as f:
    #     for id in inchi_ids:
    #         f.write(f"{id}\n")

    # with open(f'{tempfile.gettempdir()}/error_ids.txt', 'w') as f:
    #     for id in error_ids:
    #         f.write(f"{id}\n")

    # with open(f'{tempfile.gettempdir()}/error_mols.sdf', 'w') as f:
    #     for mol in error_mols:
    #         f.write(f"{mol}$$$$\n")

    # with open(f'{tempfile.gettempdir()}/errors.txt', 'w') as f:
    #     for id, error in errors.items():
    #         f.write(f"{id}: {error}\n")
    # print(f"{len(inchis)}/{mol_counter} InChIs converted.")
    
    #TODO: update if organometallics code has changed
    assert( inchi_counter == 438) # 438/529 structures can be passed into an InChI (February 12th, 2025)
