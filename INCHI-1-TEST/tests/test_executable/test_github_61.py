import subprocess
import re


def test_version_in_exe_stdout(request):
    exe_path: str = request.config.getoption("--exe-path")
    result = subprocess.run(
        [exe_path, ""],
        input="",
        capture_output=True,
        text=True,
    )

    assert re.search(r"InChI version 1, Software \d{4}\.[1-9]\d*", result.stderr)
