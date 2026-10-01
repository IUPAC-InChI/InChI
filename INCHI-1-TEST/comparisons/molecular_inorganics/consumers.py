import ctypes
from typing import Callable
from sdf_pipeline import drivers
from inchi_tests.inchi_api import make_inchi_from_molfile_text, get_inchi_key_from_inchi


def raw_regression_consumer(
    molfile: str,
    get_molfile_id: Callable,
    inchi_lib_path: str,
    inchi_api_parameters: str,
) -> drivers.ConsumerResult:
    """Stores the raw InChI, key, and exit code.

    Nothing is normalised on the way in: the reference and the logs record exactly
    what the library emitted, prefixes included. Normalisation happens at comparison
    time, in `inchi_tests.comparators.PrefixInsensitiveComparator`.

    `aux`, `log`, and `message` are omitted: they differ across versions and option
    sets for reasons unrelated to identity, and they dominate reference size at
    PubChem scale. `classify.explain_failures` recovers `message` for the small
    subset that needs it."""
    inchi_lib = ctypes.CDLL(inchi_lib_path)
    exit_code, inchi_string, _, _, _ = make_inchi_from_molfile_text(
        inchi_lib, molfile, inchi_api_parameters
    )
    _, inchi_key = get_inchi_key_from_inchi(inchi_lib, inchi_string)

    return drivers.ConsumerResult(
        molfile_id=get_molfile_id(molfile),
        info=drivers.ConsumerInfo(
            consumer="raw-regression", parameters=inchi_api_parameters
        ),
        result={
            "inchi": inchi_string,
            "key": inchi_key,
            "exit": exit_code,
        },
    )
