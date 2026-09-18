import ctypes
from typing import Callable
from sdf_pipeline import drivers
from inchi_tests.inchi_api import (
    make_inchi_from_molfile_text,
    get_inchi_key_from_inchi,
    permute_molfile_text,
)


def regression_consumer(
    molfile: str,
    get_molfile_id: Callable,
    inchi_lib_path: str,
    inchi_api_parameters: str,
) -> drivers.ConsumerResult:
    inchi_lib = ctypes.CDLL(inchi_lib_path)
    exit_code, inchi_string, log, message, aux_info = make_inchi_from_molfile_text(
        inchi_lib, molfile, inchi_api_parameters
    )
    _, inchi_key = get_inchi_key_from_inchi(inchi_lib, inchi_string)

    return drivers.ConsumerResult(
        molfile_id=get_molfile_id(molfile),
        info=drivers.ConsumerInfo(
            consumer="regression", parameters=inchi_api_parameters
        ),
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
    inchi_lib_path: str,
    inchi_api_parameters: str,
    n_invariance_runs: int,
) -> drivers.ConsumerResult:
    inchi_lib = ctypes.CDLL(inchi_lib_path)
    libc = ctypes.CDLL(None)
    variants: list[dict[str, str | int]] = []
    inchi_string_variants = set()
    inchi_key_variants = set()

    # Seed the random number generation that's used for molfile permutation.
    libc.srand(ctypes.c_uint(0))

    for _ in range(n_invariance_runs):
        _, molfile_permuted = permute_molfile_text(inchi_lib, molfile)
        if not molfile_permuted:
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
        info=drivers.ConsumerInfo(
            consumer="invariance",
            parameters=inchi_api_parameters + f"; number_of_runs={n_invariance_runs}",
        ),
        result={
            "variants": variants,
        },
    )


def inchi_body(inchi: str) -> str:
    """The InChI minus its version-and-kind prefix.

    `-MolecularInorganics` emits `InChI=1B/`, `-RecMet` emits `InChI=1/`, and the
    standard prefix is `InChI=1S/` -- for every structure, metal or not
    (`ichiprt1.c:1678` sets `is_beta` from the option alone). Comparing raw strings
    across option sets would therefore mismatch on 100% of any corpus."""
    return inchi.split("/", 1)[1] if "/" in inchi else ""


def is_failed(result: dict) -> bool:
    """Whether a campaign result represents a failure rather than a warning.

    `inchi_api.h:693-694`: `inchi_Ret_WARNING = 1`, `inchi_Ret_ERROR = 2`.
    `runichi3.c:683` warns on every metal disconnection, so exit code 1 is the norm
    on exactly the structures this campaign targets."""
    return result["exit"] >= 2 or not result["inchi"]


def campaign_regression_consumer(
    molfile: str,
    get_molfile_id: Callable,
    inchi_lib_path: str,
    inchi_api_parameters: str,
) -> drivers.ConsumerResult:
    """Stores the raw InChI, key, and exit code.

    Nothing is normalised on the way in: the reference and the logs record exactly
    what the library emitted, prefixes included. Normalisation happens at comparison
    time, in `comparators.PrefixInsensitiveComparator`.

    `aux`, `log`, and `message` are omitted: they differ across versions and option
    sets for reasons unrelated to identity, and they dominate reference size at
    PubChem scale. `campaign.explain_failures` recovers `message` for the small
    subset that needs it."""
    inchi_lib = ctypes.CDLL(inchi_lib_path)
    exit_code, inchi_string, _, _, _ = make_inchi_from_molfile_text(
        inchi_lib, molfile, inchi_api_parameters
    )
    _, inchi_key = get_inchi_key_from_inchi(inchi_lib, inchi_string)

    return drivers.ConsumerResult(
        molfile_id=get_molfile_id(molfile),
        info=drivers.ConsumerInfo(
            consumer="campaign-regression", parameters=inchi_api_parameters
        ),
        result={
            "inchi": inchi_string,
            "key": inchi_key,
            "exit": exit_code,
        },
    )
