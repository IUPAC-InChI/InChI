# Library tests

## Test environment

### Docker container

Use our [Dockerfile](../Dockerfile) to run the test suite.
Build a Docker image by running the following command from the root of the repository:

```Shell
docker compose -f INCHI-1-TEST/docker-compose.yml build --no-cache
```

The resulting image contains the machinery for running the tests.
Drop into a bash console inside the Docker container by running

```Shell
docker compose -f INCHI-1-TEST/docker-compose.yml run --rm inchi-test bash
```

You can now run the commands that are mentioned in the remainder of this README.

### Visual Studio Code devcontainer

If you prefer, run the tests in the [Visual Studio Code devcontainer](https://code.visualstudio.com/docs/devcontainers/containers)
that's specified under [.devcontainer.json](../../.devcontainer.json).
Note that in the devcontainer, you'll have to compile the InChI libraries yourself, see `INCHI-1-TEST/compile_inchi.sh` for details.
The test pipeline expects the library under `INCHI-1-TEST/libs`, see for example `INCHI-1-TEST/tests/test_library/config/config.invariance.py`.

## Datasets

In this README, `<dataset>` refers to either `ci`
(i.e, continuous integration, aka the tests running on GitHub), or a `<subset>` of PubChem.
`<subset>` can be either `compound`, `compound3d`, or `substance`.
The `ci` data already lives in the repository (i.e., `mcule.sdf.gz` and `inchi.sdf.gz` under `INCHI-1-TEST/tests/test_library/data/ci`).
The PubChem `<subset>` data doesn't live in the repository since it's too large.
You can download the `<subset>` data from <https://ftp.ncbi.nlm.nih.gov/pubchem/> by running

```Shell
python -m INCHI-1-TEST.tests.test_library.data.pubchem.download <subset>
```

On completion of the download you'll find the data in `INCHI-1-TEST/tests/test_library/data/pubchem/<subset>`.
Validate the integrity of `<subset>` (i.e., make sure the downloads aren't corrupted) by running

```Shell
python -m INCHI-1-TEST.tests.test_library.data.pubchem.validate <subset>
```

Note that validation isn't available for `compound3d` (PubChem doesn't provide file hashes).

## Invariance tests

Invariance tests are meant to detect problems with InChI's canonicalization algorithm.
During an invariance test, the atom indices of a structure are permuted repeatedly and each permutation is expected to result in the same InChI output.

<img src="./invariance.svg" alt="schematic" width="400"/>

```Shell
run-tests --test-config=INCHI-1-TEST/tests/test_library/config/config.invariance.py --data-config=INCHI-1-TEST/tests/test_library/config/config.<dataset>.py
```

uses `libinchi.so.main`, a shared library compiled from the `main` branch,
to compute the InChI output for multiple permutations of each molfile in each SDF under `<dataset>`.
If not all permutations produce the same InChI output,
a test failure is logged under `<datetime>.invariance_<dataset>.log`
(where `<datetime>` reflects the start of the test run).

## Regression tests

Regression tests investigate if the output of the current development version matches the reference output of a previous release.
In other word, regression test are meant to detect problems with the stability of the InChI across versions.
In the image below, the 1st and 2nd run represent tests runs
that are conducted after some alternations to the codebase.
On the first run, the output matches the reference.
The 2nd run results in a regression, since the output no longer matches the reference.

<img src="./regression.svg" alt="schematic" width="400"/>

### Compute references

```Shell
run-tests --test-config=INCHI-1-TEST/tests/test_library/config/config.regression_reference.py --data-config=INCHI-1-TEST/tests/test_library/config/config.<dataset>.py
```

uses `libinchi.so.<version>`, the shared library specified with `--test-config`,
and generates an `<SDF>.regression_reference.sqlite` file for each SDF under `INCHI-1-TEST/tests/test_library/data/<dataset>`.
The `sqlite` file contains a table with the results for each molfile.

### Run tests against the references

```Shell
run-tests --test-config=INCHI-1-TEST/tests/test_library/config/config.regression.py --data-config=INCHI-1-TEST/tests/test_library/config/config.<dataset>.py
```

uses `libinchi.so.main`, a shared library compiled from the `main` branch,
to compute the results (e.g., InChI strings and keys) for each molfile in each SDF under `INCHI-1-TEST/tests/test_library/data/<dataset>`.
Those results are compared with the corresponding reference.
Failed comparisons are logged to `<datetime>.regression_<dataset>.log` (where `<datetime>` reflects the start of the test run).

To convince yourself that the tests fail once a regression has been introduced,
change `INCHI_NAME` in `INCHI-1-SRC/INCHI_BASE/src/mode.h` and re-run the tests.
The tests should now fail and indicate that the difference between the reference results and the latest test run is the change you've made.

## Inspect test results

In addition to inspecting the raw logs, you can review the results by running

```Shell
parse-log --test-config=INCHI-1-TEST/tests/test_library/config/config.<test>.py --data-config=INCHI-1-TEST/tests/test_library/config/config.<dataset>.py
```

where `<test>` can be `regression` or `invariance`.
The command generates an HTML report for each SDF under `INCHI-1-TEST/tests/test_library/data/<dataset>` that contains structures which failed the test.
You can view the HTML report in your browser.

## Inspect `.sqlite` files

For conveniently viewing `.sqlite` files, install the `SQLite Viewer` extension for VSCode: <https://marketplace.visualstudio.com/items?itemName=qwtel.sqlite-viewer>. Otherwise you can query the `.sqlite` files with the [sqlite command line utility](https://sqlite.org/cli.html).

## Test customization

So far, we showed how to run the tests with our configuration against our datasets.
Alternatively, you can run the tests against your own data and/or adapt the configuration.

Before showing examples of how to customize the tests, let's look at how we're configuring them.
Our [docker-compose.yml](../docker-compose.yml) shows how to inject the data and configuration into the [container](#docker-container)
via [volumes](https://docs.docker.com/compose/compose-file/05-services/#volumes):

```yml
volumes:
  - type: bind
    source: tests/test_library/data
    target: /inchi/INCHI-1-TEST/data
  - type: bind
    source: tests/test_library/config
    target: /inchi/INCHI-1-TEST/config
```

Note that the `source` paths are relative to the location of the `docker-compose.yml` file.
We're mapping the `tests/test_library/data` directory on the host machine to the `/inchi/INCHI-1-TEST/data` directory inside the container.
Similarly we're mapping `tests/test_library/config`, a directory containing our [configuration files](#configuration-files), into `/inchi/INCHI-1-TEST/config`.

To customize the tests, start by adding your own `docker-compose.custom.yml` file:

```yml
services:
  inchi-custom-test:
    build:
      context: ..
      dockerfile: INCHI-1-TEST/Dockerfile
```

Make sure to adapt `context` to your directory structure. `context` needs to be the path to the `InChI` repository,
relative from `docker-compose.custom.yml`.

### Your own dataset

You can map a single SDF file or a directory containing SDF files into the container's `/inchi` directory by specifying a volume:

```yml
services:
  inchi-custom-test:
    build:
      context: ..
      dockerfile: INCHI-1-TEST/Dockerfile
    volumes:
      # `source` paths are relative to the `docker-compose.yml` file, not the build context.
      # `target` paths are absolute paths in the container. The `/inchi` directory already exists in the container.
      - type: bind
        source: host/machine/path/to/custom/data
        target: /inchi/data
```

### Your own configuration

Next, add a volume to inject your [configuration files](#configuration-files) into the container:

```yml
services:
  inchi-custom-test:
    build:
      context: ..
      dockerfile: INCHI-1-TEST/Dockerfile
    volumes:
      # `source` paths are relative to the `docker-compose.yml` file, not the build context.
      # `target` paths are absolute paths in the container. The `/inchi` directory already exists in the container.
      - type: bind
        source: host/machine/path/to/custom/data
        target: /inchi/data
      - type: bind
        source: host/machine/path/to/custom/config
        target: /inchi/config
```

### Configuration files

The tests can be configured with Python files (e.g., `config.py`).
We're not using other configuration formats (e.g., `config.yaml`),
since the configuration needs to be powerful enough to enable dynamic customization (e.g., parsing molfile ID).
We provide two [templates](INCHI-1-TEST/tests/test_library/inchi_tests/config_models.py) under `config_models.py` that allow you to customize the configuration:

#### `TestConfig`

Lets you customize the test itself, e.g.,
configuring what to run ("regression", "regression-reference", or "invariance"),
which InChI library to use, and which parameters to pass to the InChI API.
For details, have a look at the comments in the `TestConfig` class.
Your configuration file, e.g., `config/custom-regression.py` must contain an instance of `TestConfig` called `config`.
For an example of how to instantiate a `TestConfig` object, have a look at our [regression configuration](INCHI-1-TEST/tests/test_library/config/config.regression.py).

#### `DataConfig`

Lets you configure your custom data, e.g., location of the data.
For details, have a look at the comments in the `DataConfig` class.
Your configuration file, e.g., `config/custom-data.py` must contain an instance of `DataConfig` called `config`.
For an example of how to instantiate a `DataConfig` object, have a look at our [CI configuration](INCHI-1-TEST/tests/test_library/config/config.ci.py).
Note that the `DataConfig` object must point to data that you've [mounted into the container](#your-own-dataset).
For an example of how to instantiate a `DataConfig` object, have a look at our the configuration of our [CI data](INCHI-1-TEST/tests/test_library/config/config.ci.py).

### Run your custom tests

Once you've written the `docker-compose.custom.yml` and your [configuration files](#configuration-files), build a custom image with

```Shell
docker compose -f path/to/docker-compose.custom.yml build --no-cache
```

You can now start the container and drop into a bash shell

```Shell
docker compose -f path/to/docker-compose.custom.yml run --rm inchi-custom-test bash
```

and run the test according to your configuration against your data

```Shell
run-tests --test-config=config/custom-regression.py --data-config=config/custom-data.py
```
