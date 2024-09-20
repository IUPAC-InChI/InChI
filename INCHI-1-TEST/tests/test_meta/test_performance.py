import pytest
import timeit
from pathlib import Path
from functools import partial
from sdf_pipeline import drivers, core
from consumers import busy_consumer, regression_consumer


def _get_pubchem_id(molfile: str) -> str:
    return molfile.split()[0].strip()


@pytest.fixture
def sdf_path():
    return (
        Path(__file__)
        .parent.absolute()
        .joinpath("data/Compound_029500001_030000000.sdf.gz")
    )


@pytest.mark.skip
def test_execution_time_scales_with_cores(sdf_path):
    def run_core(n_processes):
        def _run_core():
            for _ in core.run(
                sdf_path=str(sdf_path),
                consumer_function=partial(
                    busy_consumer, get_molfile_id=_get_pubchem_id
                ),
                number_of_consumer_processes=n_processes,
            ):
                pass

        return _run_core

    previous_execution_time = 0
    for n in [1, 2, 4, 8, 16]:
        execution_time = timeit.Timer(run_core(n)).timeit(1)
        print(f"{n} processes finished in {execution_time} seconds.")

        if previous_execution_time:
            assert execution_time < previous_execution_time

        previous_execution_time = execution_time


def test_execution_time(sdf_path, tmp_path):

    def compute_references():
        def _compute_references():
            drivers.regression_reference(
                sdf_path=sdf_path,
                reference_path=tmp_path / "reference.sqlite",
                consumer_function=regression_consumer,
                get_molfile_id=_get_pubchem_id,
                number_of_consumer_processes=8,
            )

        return _compute_references

    execution_time = timeit.Timer(compute_references()).timeit(1)
    print(f"Finished in {execution_time} seconds.")

    assert execution_time < 10
