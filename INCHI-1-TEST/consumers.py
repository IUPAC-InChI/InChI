import ctypes
import random
from typing import Callable
from pathlib import Path
from sdf_pipeline import drivers, utils
from .inchi_api import make_inchi_from_molfile_text, get_inchi_key_from_inchi


def regression_consumer(
    molfile: str,
    get_molfile_id: Callable,
    inchi_lib_path: Path,
    inchi_api_parameters: str,
) -> drivers.ConsumerResult:

    inchi_lib = ctypes.CDLL(str(inchi_lib_path))
    exit_code, inchi_string, log, message, aux_info = make_inchi_from_molfile_text(
        inchi_lib, molfile, inchi_api_parameters
    )
    _, inchi_key = get_inchi_key_from_inchi(inchi_lib, inchi_string)

    return drivers.ConsumerResult(
        molfile_id=get_molfile_id(molfile),
        info={"consumer": "regression", "parameters": inchi_api_parameters},
        result={
            "inchi": inchi_string,
            "key": inchi_key,
            "aux": aux_info,
            "log": log,
            "message": message,
            "exit": exit_code,
        },
    )


def invariance_consumer(
    molfile: str,
    get_molfile_id: Callable,
    inchi_lib_path: Path,
    inchi_api_parameters: str,
    n_invariance_runs: int,
) -> drivers.ConsumerResult:

    inchi_lib = ctypes.CDLL(str(inchi_lib_path))
    variants: list[dict[str, str | int]] = []
    inchi_string_variants = set()
    inchi_key_variants = set()
    random.seed(42)

    for _ in range(n_invariance_runs):
        molfile_permuted = utils.permute_molblock(molfile)
        if molfile_permuted is None:
            # Failure to parse `molfile`,
            # return empty result which will be handled upstream.
            break

        exit_code, inchi_string, log, message, aux_info = make_inchi_from_molfile_text(
            inchi_lib, molfile_permuted, inchi_api_parameters
        )
        _, inchi_key = get_inchi_key_from_inchi(inchi_lib, inchi_string)

        if (inchi_string not in inchi_string_variants) or (
            inchi_key not in inchi_key_variants
        ):
            # Explicitly check for variance in string or key,
            # since log, message, or aux_info differ across runs,
            # even when string or key don't differ.
            inchi_string_variants.add(inchi_string)
            inchi_key_variants.add(inchi_key)
            variants.append(
                {
                    "inchi": inchi_string,
                    "key": inchi_key,
                    "molfile": molfile_permuted,
                    "aux": aux_info,
                    "log": log,
                    "message": message,
                    "exit": exit_code,
                },
            )

    return drivers.ConsumerResult(
        molfile_id=get_molfile_id(molfile),
        info={
            "consumer": "invariance",
            "parameters": inchi_api_parameters
            + f"; number_of_runs={n_invariance_runs}",
        },
        result={
            "variants": variants,
        },
    )
