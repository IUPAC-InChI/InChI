import re
from pathlib import Path
from inchi_tests.config_models import DataConfig


def get_molfile_id_ci(molfile: str) -> str:
    molfile_id = ""

    mcule_id_patterns = re.compile(r"<Mcule_ID>(.*?)>", re.DOTALL)
    mcule_id_match = mcule_id_patterns.search(molfile)
    if mcule_id_match:
        # mcule test set.
        molfile_id = mcule_id_match.group(1).strip()
    else:
        # InChI legacy test set.
        molfile_id = molfile.splitlines()[-3].strip()

    return molfile_id


BASEPATH = "INCHI-1-TEST/tests/test_library/data/ci/"

config = DataConfig(
    name="ci",
    path=Path(BASEPATH),
    sdf_paths=sorted(Path(BASEPATH).glob("*.sdf.gz")),
    molfile_id_getter=get_molfile_id_ci,
    expected_failures={
        "regression": {
            "PUBCHEM_COMPOUND_CID-20733713",
            "PUBCHEM_COMPOUND_CID-166625356",
            "PUBCHEM_COMPOUND_CID-102182973",
            "PUBCHEM_COMPOUND_CID-92178689",
        },
        "invariance": {"D4h-allene", "PUBCHEM_COMPOUND_CID-102182973"},
    },
)
