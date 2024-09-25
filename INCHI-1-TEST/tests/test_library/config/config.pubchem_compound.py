from pathlib import Path
from inchi_tests.config_models import DataConfig
from inchi_tests.utils import get_molfile_id_pubchem

BASEPATH = "INCHI-1-TEST/tests/test_library/data/pubchem/compound"

config = DataConfig(
    name="pubchem-compound",
    path=Path(BASEPATH),
    sdf_paths=sorted(Path(BASEPATH).glob("*.sdf.gz")),
    molfile_id_getter=get_molfile_id_pubchem,
)
