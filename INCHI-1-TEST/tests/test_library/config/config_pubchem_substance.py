from pathlib import Path
from inchi_tests.config_models import DataConfig
from inchi_tests.utils import get_molfile_id_pubchem

BASEPATH = "INCHI-1-TEST/tests/test_library/data/pubchem/substance"

# PubChem substance record SID 141382403 in Substance_141000001_141500000.sdf.gz
# (238 atoms, 204 of them phosphorus) did not finish within ten minutes in
# v1.07.5. `sdf_pipeline.core.run` stops a consumer after 60 s on one molfile;
# listing the SID here makes that timeout an expected failure.
EXCEEDS_TIME_LIMIT = {"141382403"}

config = DataConfig(
    name="pubchem-substance",
    path=Path(BASEPATH),
    sdf_paths=sorted(Path(BASEPATH).glob("*.sdf.gz")),
    molfile_id_getter=get_molfile_id_pubchem,
    expected_failures={
        "regression": EXCEEDS_TIME_LIMIT,
        "invariance": EXCEEDS_TIME_LIMIT,
    },
)
