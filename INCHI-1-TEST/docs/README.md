# Regression tests

<img src="./regression.svg" alt="schematic" width="400"/>

Preferably run the tests in the [Visual Studio Code devcontainer](https://code.visualstudio.com/docs/devcontainers/containers)
that's specified under [.devcontainer.json](../../.devcontainer.json).

Alternatively, you can run the tests in a stand-alone Docker container.
Build a Docker image from the [Dockerfile](../Dockerfile) with

```Shell
docker build -t inchi-tests ./INCHI-1-TEST
```
, and subsequently drop into a terminal inside of the Docker container by running

```Shell
docker run --rm -it -v $(pwd):/inchi inchi-tests bash
```
On Windows, wrap the argument to `-v` in quotation marks: `"$(pwd):/inchi"`.

From the `inchi` directory in the Docker container you can run the commands from the
instructions below.

Alternatively, you can run single commands non-interactively:

```Shell
docker run --rm -v $(pwd):/inchi inchi-tests bash -c "cd inchi && <command>"
```

## Datasets

In the following instructions `<dataset>` refers to either `ci`
(i.e, continuous integration, aka the tests running on GitHub), or `pubchem_<subset>` (<subset> can be either `compound`, `compound3d`, or `substance`).
The `ci` data already lives in the repository (the `mcule.sdf.gz` and `inchi.sdf.gz` files under `INCHI-1-TEST/data/ci`).
The `pubchem_<subset>` data doesn't live in the repository since it's too large.
You can download the `pubchem_<subset>` data from https://ftp.ncbi.nlm.nih.gov/pubchem/ by running

```Shell
python -m INCHI-1-TEST.data.pubchem.download pubchem_<subset>
```

On completion of the download you'll find the data in `INCHI-1-TEST/data/pubchem/<subset>`.
Validate the integrity of `pubchem_<subset>` (i.e., make sure the downloads aren't corrupted) by running

```Shell
python -m INCHI-1-TEST.data.pubchem.validate pubchem_<subset>
```

Note that validation isn't available for `pubchem_compound3d`.

## Compute reference

```Shell
python -m INCHI-1-TEST.compute_regression_reference <dataset>
```
downloads `libinchi.so.1.06.00`, the shared library belonging to the current stable InChI release,
and generates an `<SDF>.regression_reference.sqlite` file for each SDF under `INCHI-1-TEST/data/<dataset>`.
The `sqlite` file contains a table with the results for each molfile.

For example,

| molfile_id | time | info | result |
| --- | --- | --- | --- |
| 9261759198 | 2023-05-31T09:36:50 | consumer: regression; parameters: '' | InChI=1S/C2BF6/c4-2(5,6)1-3(7,8)9/q-1 |
| 2139556156 | 2023-05-31T09:36:50 | consumer: regression; parameters: '' | InChI=1S/AsCl3/c2-1(3)4 |


## Run tests

```Shell
python -m INCHI-1-TEST.run_regression_tests <dataset>
```
compiles the shared library `libinchi.so.dev` from the current state of the repository.
It then uses this library to compute the results (e.g., InChI strings and keys) for each molfile in each SDF under `INCHI-1-TEST/data/<dataset>`.
Those results are compared with the corresponding reference.
Failed comparisons are logged to `<datetime>.regression_test.log` (where `<datetime>` reflects the start of the test run).

To convince yourself that the tests fail once a regression has been introduced,
change `INCHI_NAME` in `INCHI-1-SRC/INCHI_BASE/src/mode.h` and re-run the tests.
The tests should now fail and indicate that the difference between the reference results and the latest test run is the change you've made.


## Inspect test results

You can review the results by running

```Shell
python -m INCHI-1-TEST.parse_regression_log <dataset>
```

The command generates an HTML report for each SDF under `INCHI-1-TEST/data/<dataset>` that contains structures which failed the regression test.
You can view the HTML report in your browser.


## Inspect `.sqlite` files

For conveniently viewing `.sqlite` files, install the `SQLite Viewer` extension for VSCode: https://marketplace.visualstudio.com/items?itemName=qwtel.sqlite-viewer. Otherwise you can query the `.sqlite` files with the [sqlite command line utility](https://sqlite.org/cli.html).
