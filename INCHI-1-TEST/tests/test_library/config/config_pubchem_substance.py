from pathlib import Path
from inchi_tests.config_models import DataConfig
from inchi_tests.utils import get_molfile_id_pubchem

BASEPATH = "INCHI-1-TEST/tests/test_library/data/pubchem/substance"

# SID 141382403 (Substance_141000001_141500000) is 238 atoms, 204 of them
# phosphorus and the rest sulfur, with 237 bonds -- a near-tree of two element
# types, so almost every atom is equivalent under the initial invariants and
# canonical ranking has to break the ties by refinement. v1.07.5 ran it for over
# ten minutes without finishing; it is not known to terminate. The consumer
# timeout keeps it from costing the whole shard, and listing it here keeps the
# expected timeout from failing the run.
SLOW_TO_CANONICALIZE = {"141382403"}

config = DataConfig(
    name="pubchem-substance",
    path=Path(BASEPATH),
    sdf_paths=sorted(Path(BASEPATH).glob("*.sdf.gz")),
    molfile_id_getter=get_molfile_id_pubchem,
    expected_failures={
        "regression": SLOW_TO_CANONICALIZE,
        "invariance": SLOW_TO_CANONICALIZE,
    },
)
