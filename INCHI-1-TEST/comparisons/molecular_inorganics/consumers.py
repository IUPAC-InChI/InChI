import ctypes
from typing import Callable
from sdf_pipeline import drivers
from inchi_tests.inchi_api import make_inchi_from_molfile_text, get_inchi_key_from_inchi


# What every pass stores per structure. `run.check_reference_format` refuses a
# reference written with any other set, so changing this forces the reference
# pass to be re-run rather than silently comparing against stale rows.
RESULT_FIELDS = frozenset({"inchi", "key", "exit", "message"})


def raw_regression_consumer(
    molfile: str,
    get_molfile_id: Callable,
    inchi_lib_path: str,
    inchi_api_parameters: str,
) -> drivers.ConsumerResult:
    """Stores the raw InChI, key, exit code and warning message.

    Nothing is normalised on the way in: the reference and the logs record exactly
    what the library emitted, prefixes included. Normalisation happens at comparison
    time, in `inchi_tests.comparators.PrefixInsensitiveComparator`.

    `message` is the short warning text, e.g. "Metal was disconnected": it names
    the disconnection route and is what a warning-level change looks like, and it
    is empty for most structures. `aux` and `log` are omitted: they dominate
    reference size at PubChem scale."""
    inchi_lib = ctypes.CDLL(inchi_lib_path)
    exit_code, inchi_string, _, message, _ = make_inchi_from_molfile_text(
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
            "message": message,
        },
    )
