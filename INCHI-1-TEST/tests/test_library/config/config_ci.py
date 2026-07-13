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
        "invariance": {
            "_Tech_Man_Chapt_IV_b_Step_5_1a",
            "_Tech_Man_Table02-2_full",
            "_Tech_Man_Table05_12b",
            "_Tech_Man_Table05_07-09",
            "iso-H-Bridge",
            "large_NSC-682379",
            "large_NSC-682379m",
            "large_NSC-666457-2D",
            "large_PUBCHEM_COMPOUND_CID-3001801",
            "large_PUBCHEM_COMPOUND_CID-3009855",
            "large_PUBCHEM_COMPOUND_CID-3013619",
            "large_PUBCHEM_COMPOUND_CID-3013620",
            "large_PUBCHEM_COMPOUND_CID-3013621",
            "large_PUBCHEM_COMPOUND_CID-3013622",
            "large_PUBCHEM_COMPOUND_CID-3247035",
            "large_PUBCHEM_COMPOUND_CID-3338043",
            "large_PUBCHEM_COMPOUND_CID-3812506",
            "large_PUBCHEM_COMPOUND_CID-406953",
            "large_PUBCHEM_COMPOUND_CID-406952",
            "large_PUBCHEM_COMPOUND_CID-414315",
            "large_PUBCHEM_COMPOUND_CID-440817",
            "large_PUBCHEM_COMPOUND_CID-452196",
            "large_PUBCHEM_COMPOUND_CID-452197",
            "large_PUBCHEM_COMPOUND_CID-452199",
            "large_PUBCHEM_COMPOUND_CID-452198",
            "large_PUBCHEM_COMPOUND_CID-467519",
            "large_PUBCHEM_COMPOUND_CID-4660115",
            "large_PUBCHEM_COMPOUND_CID-479173",
            "large_PUBCHEM_COMPOUND_CID-479186",
            "large_PUBCHEM_COMPOUND_CID-5090984",
            "large_PUBCHEM_COMPOUND_CID-644058",
            "large_PUBCHEM_COMPOUND_CID-644059",
            "large_PUBCHEM_COMPOUND_CID-656474",
            "large_PUBCHEM_COMPOUND_CID-656478",
            "3442296542",
            "1980967343",
        },
    },
)
