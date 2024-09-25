from pathlib import Path
from inchi_tests.config_models import DataConfig
from inchi_tests.utils import get_molfile_id_pubchem

BASEPATH = "INCHI-1-TEST"

config = DataConfig(
    name="pubchem-compound3d",
    path=Path(BASEPATH).joinpath("data/pubchem/compound3d"),
    sdf_paths=sorted(
        Path(BASEPATH).joinpath("data/pubchem/compound3d").glob("*.sdf.gz")
    ),
    molfile_id_getter=get_molfile_id_pubchem,
)
