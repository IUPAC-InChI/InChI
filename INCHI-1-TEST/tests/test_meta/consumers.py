import ctypes
from typing import Callable
from sdf_pipeline import drivers


def regression_consumer(
    molfile: str, get_molfile_id: Callable
) -> drivers.ConsumerResult:

    return drivers.ConsumerResult(
        molfile_id=get_molfile_id(molfile),
        info={"consumer": "regression"},
        result={
            "molfile_length": (
                42 if get_molfile_id(molfile) == "9261759198" else len(molfile)
            )
        },
    )


def invariance_consumer(
    molfile: str, get_molfile_id: Callable
) -> drivers.ConsumerResult:

    return drivers.ConsumerResult(
        molfile_id=get_molfile_id(molfile),
        info={"consumer": "invariance"},
        result={
            "variants": ["A", "B"] if get_molfile_id(molfile) == "9261759198" else ["A"]
        },
    )


def two_diff_regression_consumer(
    molfile: str, get_molfile_id: Callable
) -> drivers.ConsumerResult:
    # Diffs against the reference for both the first ("9261759198") and the
    # last ("1690718558") molfile in mcule_20000.sdf.gz, so a single run can
    # contain both an unexpected and an expected failure.
    return drivers.ConsumerResult(
        molfile_id=get_molfile_id(molfile),
        info={"consumer": "regression"},
        result={
            "molfile_length": (
                42
                if get_molfile_id(molfile) in ("9261759198", "1690718558")
                else len(molfile)
            )
        },
    )


def two_diff_invariance_consumer(
    molfile: str, get_molfile_id: Callable
) -> drivers.ConsumerResult:
    # Fails invariance for both the first ("9261759198") and the last
    # ("1690718558") molfile in mcule_20000.sdf.gz.
    return drivers.ConsumerResult(
        molfile_id=get_molfile_id(molfile),
        info={"consumer": "invariance"},
        result={
            "variants": (
                ["A", "B"]
                if get_molfile_id(molfile) in ("9261759198", "1690718558")
                else ["A"]
            )
        },
    )


def busy_consumer(molfile: str, get_molfile_id: Callable) -> drivers.ConsumerResult:
    n = 0
    for i in range(100):
        n += sum([len(line) ** i for line in molfile.split("\n")])

    return drivers.ConsumerResult(
        molfile_id=get_molfile_id(molfile),
        info={"consumer": "busy_consumer"},
        result={"large_result": n},
    )


def segfaulting_consumer(
    molfile: str, get_molfile_id: Callable
) -> drivers.ConsumerResult:
    ctypes.string_at(0)


def raising_consumer(molfile: str, get_molfile_id: Callable) -> drivers.ConsumerResult:
    1 / 0
