import os
import subprocess
import sys
from pathlib import Path


def test_parse_log_cli_accepts_the_runner_arguments(tmp_path):
    """Every consumer of `get_config_args` must survive the runner gaining an option.

    `parse_log.py` shares the parser with `run_tests.py` and unpacked a fixed-width
    tuple, so adding an option broke the CI step that renders the regression log
    -- after the unit, executable and regression tests had all passed."""
    repo = Path(__file__).resolve().parents[3]
    data = tmp_path / "data"
    data.mkdir()
    config = tmp_path / "config_tmp.py"
    config.write_text(
        "from pathlib import Path\n"
        "from inchi_tests.config_models import DataConfig\n"
        f"BASEPATH = {str(data)!r}\n"
        "config = DataConfig(\n"
        "    name='tmp',\n"
        "    path=Path(BASEPATH),\n"
        "    sdf_paths=sorted(Path(BASEPATH).glob('*.sdf.gz')),\n"
        "    molfile_id_getter=lambda molfile: molfile.splitlines()[0].strip(),\n"
        "    expected_failures={},\n"
        ")\n",
        encoding="utf-8",
    )
    lib = tmp_path / "libinchi.so"
    lib.write_text("")

    env = {
        **os.environ,
        "PYTHONPATH": os.pathsep.join(
            [
                str(repo / "INCHI-1-TEST" / "src"),
                str(repo / "INCHI-1-TEST" / "tests" / "test_library"),
                os.environ.get("PYTHONPATH", ""),
            ]
        ),
    }
    result = subprocess.run(
        [
            sys.executable,
            str(repo / "INCHI-1-TEST/tests/test_library/inchi_tests/parse_log.py"),
            "--test=regression",
            f"--lib-path={lib}",
            f"--data-config={config}",
        ],
        capture_output=True,
        text=True,
        cwd=repo,
        env=env,
    )
    assert result.returncode == 0, result.stderr
