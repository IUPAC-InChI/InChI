import ctypes
import json
from typing import Callable
from pathlib import Path
from sdf_pipeline import drivers
from .inchi_api import make_inchi_from_molfile_text, get_inchi_key_from_inchi
from .config import INCHI_API_PARAMETERS


def regression_consumer(
    molfile: str, get_molfile_id: Callable, inchi_lib_path: Path
) -> drivers.ConsumerResult:

    inchi_lib = ctypes.CDLL(str(inchi_lib_path))
    exit_code, inchi_string, log, message, aux_info = make_inchi_from_molfile_text(
        inchi_lib, molfile, INCHI_API_PARAMETERS
    )
    _, inchi_key = get_inchi_key_from_inchi(inchi_lib, inchi_string)

    return drivers.ConsumerResult(
        get_molfile_id(molfile),
        json.dumps({"consumer": "regression", "parameters": INCHI_API_PARAMETERS}),
        json.dumps(
            {
                "inchi": inchi_string,
                "key": inchi_key,
                "aux": aux_info,
                "log": log,
                "message": message,
                "exit": exit_code,
            }
        ),
    )
