# Executable tests

Test specific behaviors of the executable.
Ensure that specific input (molfile and arguments) elicits specific output (e.g., error).

Where possible, stick to the following principles:
- Define input inline rather than using external files (having entire test scenario on screen is nice).
- Keep number of tests per file to a minimum.
- When replicating a bug, reference the (GitHub) issue number in the file name (e.g., `test_github_42.py`).

Have a look at the existing tests for examples on how to write a test.

Run with `pytest INCHI-1-TEST/tests/test_executable`.
Note that by default, the tests expect the InChI executable to live at `INCHI-1-TEST/exes/inchi-1`.
You can specify another InChI executable like so:

```shell
pytest INCHI-1-TEST/tests/test_executable --exe-path=path/to/executable
```
