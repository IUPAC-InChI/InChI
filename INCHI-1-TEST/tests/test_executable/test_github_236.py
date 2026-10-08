import os
import sys
import time
from pathlib import Path

import pytest

# Regression tests for https://github.com/IUPAC-InChI/InChI/issues/236
#
# In 1.07.5 the CLI printed the help text and exited whenever the first
# argument started with the option prefix, so piping a structure on stdin
# ("cat file.mol | inchi-1 -STDIO ...") never read the input, and even an
# explicit input file was ignored when options preceded it. The help text is
# now shown only when no input file is given *and* stdin is an interactive
# terminal (otherwise the process would block forever). These tests cover the
# stdin pipe, an input file placed after the options at a real terminal, and
# the interactive no-input safeguard.

EXPECTED_INCHI = (
    "InChI=1S/C41H78O17P2/c1-5-9-13-17-21-25-38(43)51-31-36(57-40(45)"
    "27-23-19-15-11-7-3)33-55-59(47,48)53-29-35(42)30-54-60(49,50)56-"
    "34-37(58-41(46)28-24-20-16-12-8-4)32-52-39(44)26-22-18-14-10-6-2/"
    "h35-37,42H,5-34H2,1-4H3,(H,47,48)(H,49,50)/t36-,37-/m1/s1"
)
EXPECTED_INCHIKEY = "InChIKey=PVSPUBLWCTXHDH-FZNHDDJXSA-N"


@pytest.fixture
def molfile():
    return '102383106\n  CDK     1202211636\n\n 60 59  0  0  0  0  0  0  0  0999 V2000\n   14.5108   -2.5534    0.0000 P   0  0  0  0  0  0  0  0  0  0  0  0\n   10.2239   -2.5534    0.0000 P   0  0  0  0  0  0  0  0  0  0  0  0\n   17.3688   -2.5534    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0\n    7.3659   -2.5534    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0\n   17.3688   -0.9034    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0\n    7.3659   -0.9034    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0\n   15.2253   -2.1409    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0\n    9.5094   -2.1409    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0\n   18.0832   -1.3159    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0\n    6.6515   -1.3159    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0\n   16.6542    0.3341    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0\n    8.0804    0.3341    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0\n   13.7963   -2.9659    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0\n   10.9383   -2.9659    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0\n   14.0983   -1.8390    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0\n    9.8114   -3.2679    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0\n   12.3674   -3.7910    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0\n   14.9233   -3.2679    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0\n   10.6364   -1.8390    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0\n   20.2267   -2.5534    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n    4.5080   -2.5534    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n   20.9412   -2.1409    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n    3.7935   -2.1409    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n   18.7977    1.5717    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n    5.9370    1.5717    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n   18.7977    2.3967    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n    5.9370    2.3967    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n   19.5122   -2.1409    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n    5.2224   -2.1409    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n   21.6557   -2.5534    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n    3.0790   -2.5534    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n   18.0832    1.1592    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n    6.6515    1.1592    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n   19.5122    2.8092    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n    5.2224    2.8092    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n   18.7977   -2.5534    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n    5.9370   -2.5534    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n   22.3701   -2.1409    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n    2.3645   -2.1409    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n   18.0832    0.3341    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n    6.6515    0.3341    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n   19.5122    3.6342    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n    5.2224    3.6342    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n   16.6542   -2.1409    0.0000 C   0  0  2  0  0  0  0  0  0  0  0  0\n    8.0804   -2.1409    0.0000 C   0  0  2  0  0  0  0  0  0  0  0  0\n   16.6542   -1.3159    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n    8.0804   -1.3159    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n   18.0832   -2.1409    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n    6.6515   -2.1409    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n   15.9398   -2.5534    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n    8.7949   -2.5534    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n   17.3688   -0.0784    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n    7.3659   -0.0784    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n   23.0847   -2.5534    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n    1.6500   -2.5534    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n   20.2267    4.0467    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n    4.5080    4.0467    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n   12.3674   -2.9659    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n   13.0818   -2.5534    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n   11.6529   -2.5534    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n  1  7  1  0  0  0  0\n  1 13  1  0  0  0  0\n  1 15  1  0  0  0  0\n  1 18  2  0  0  0  0\n  2  8  1  0  0  0  0\n  2 14  1  0  0  0  0\n  2 16  1  0  0  0  0\n  2 19  2  0  0  0  0\n 44  3  1  1  0  0  0\n  3 48  1  0  0  0  0\n 45  4  1  6  0  0  0\n  4 49  1  0  0  0  0\n  5 46  1  0  0  0  0\n  5 52  1  0  0  0  0\n  6 47  1  0  0  0  0\n  6 53  1  0  0  0  0\n  7 50  1  0  0  0  0\n  8 51  1  0  0  0  0\n  9 48  2  0  0  0  0\n 10 49  2  0  0  0  0\n 11 52  2  0  0  0  0\n 12 53  2  0  0  0  0\n 13 59  1  0  0  0  0\n 14 60  1  0  0  0  0\n 17 58  1  0  0  0  0\n 20 22  1  0  0  0  0\n 20 28  1  0  0  0  0\n 21 23  1  0  0  0  0\n 21 29  1  0  0  0  0\n 22 30  1  0  0  0  0\n 23 31  1  0  0  0  0\n 24 26  1  0  0  0  0\n 24 32  1  0  0  0  0\n 25 27  1  0  0  0  0\n 25 33  1  0  0  0  0\n 26 34  1  0  0  0  0\n 27 35  1  0  0  0  0\n 28 36  1  0  0  0  0\n 29 37  1  0  0  0  0\n 30 38  1  0  0  0  0\n 31 39  1  0  0  0  0\n 32 40  1  0  0  0  0\n 33 41  1  0  0  0  0\n 34 42  1  0  0  0  0\n 35 43  1  0  0  0  0\n 36 48  1  0  0  0  0\n 37 49  1  0  0  0  0\n 38 54  1  0  0  0  0\n 39 55  1  0  0  0  0\n 40 52  1  0  0  0  0\n 41 53  1  0  0  0  0\n 42 56  1  0  0  0  0\n 43 57  1  0  0  0  0\n 44 46  1  0  0  0  0\n 44 50  1  0  0  0  0\n 45 47  1  0  0  0  0\n 45 51  1  0  0  0  0\n 58 59  1  0  0  0  0\n 58 60  1  0  0  0  0\nM  END\n'


def test_reads_structure_from_stdin(molfile, run_inchi_exe_stdin):
    result = run_inchi_exe_stdin(molfile, args="STDIO Key AuxNone")

    # The help text must not be emitted instead of processing the structure.
    assert "Usage:" not in result.stdout
    assert "Usage:" not in result.stderr

    assert EXPECTED_INCHI in result.stdout
    assert EXPECTED_INCHIKEY in result.stdout


def _run_under_pty(argv: list[str], timeout: float = 10.0) -> str:
    """Run argv with stdin/stdout attached to a pseudo-terminal.

    The interactive-no-input safeguard only triggers when stdin is a real tty,
    which a plain subprocess pipe is not, so drive the executable through a pty
    and return everything it wrote to the terminal.
    """
    import pty
    import select

    pid, fd = pty.fork()
    if pid == 0:  # child: exec the executable with the pty as its std streams
        os.execv(argv[0], argv)

    out = bytearray()
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        readable, _, _ = select.select([fd], [], [], 0.3)
        if readable:
            try:
                chunk = os.read(fd, 4096)
            except OSError:  # pty closed on child exit
                break
            if not chunk:
                break
            out += chunk
        else:
            try:
                if os.waitpid(pid, os.WNOHANG)[0]:
                    break
            except ChildProcessError:
                break
    try:
        os.waitpid(pid, 0)
    except (ChildProcessError, OSError):
        pass
    return out.decode(errors="replace")


@pytest.mark.skipif(sys.platform == "win32", reason="pty is POSIX-only")
def test_input_file_after_options_at_tty(molfile, tmp_path, request):
    """An input file must be honoured even when options precede it at a tty.

    Reproduces the case raised in review: "inchi-1 -Key -AuxNone -STDIO f.mol"
    used to print the help text at an interactive terminal because the guard
    only inspected argv[1].
    """
    exe_path: str = request.config.getoption("--exe-path")
    if not Path(exe_path).exists():
        raise FileNotFoundError(f"InChI executable not found at {exe_path}.")

    mol_path = tmp_path.joinpath("input.mol")
    mol_path.write_text(molfile)

    output = _run_under_pty(
        [exe_path, "-Key", "-AuxNone", "-STDIO", str(mol_path)]
    )

    assert "Usage:" not in output
    assert EXPECTED_INCHIKEY in output


@pytest.mark.skipif(sys.platform == "win32", reason="pty is POSIX-only")
def test_help_when_no_input_file_at_tty(request):
    """With only options and an interactive terminal, show help, do not hang."""
    exe_path: str = request.config.getoption("--exe-path")
    if not Path(exe_path).exists():
        raise FileNotFoundError(f"InChI executable not found at {exe_path}.")

    output = _run_under_pty([exe_path, "-Key", "-AuxNone", "-STDIO"])

    assert "Usage:" in output
    assert EXPECTED_INCHIKEY not in output
