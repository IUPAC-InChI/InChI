"""
This script is meant to trigger segfaults.
Unix systems return exit code 139 when a process terminates due to a segfault.
139 = 128 + 11,
where 128+n means "Fatal error signal 'n'", see https://tldp.org/LDP/abs/html/exitcodes.html,
and 11 means "SIGSEGV", i.e., segfault, see wikipedia.org/wiki/Signal_(IPC)#POSIX_signals.
"""

import threading
import ctypes
from pathlib import Path
from inchi_tests.inchi_api import make_inchi_from_molfile_text
from sdf_pipeline.utils import read_records_from_gzipped_sdf


SDF_PATH = Path("INCHI-1-TEST/tests/test_library/data/ci/inchi.sdf.gz")
INCHI_LIB_PATH = Path("INCHI-1-TEST/libs/libinchi.so.main")
# INCHI_LIB_PATH = Path("INCHI-1-TEST/libs/libinchi.so.v1.06")
INCHI_LIB = ctypes.CDLL(str(INCHI_LIB_PATH))


class ConsumerThread(threading.Thread):
    def __init__(self, inchi_lib, molfile):
        super().__init__()
        self.inchi_lib = inchi_lib
        self.molfile = molfile

    def run(self):
        (_, inchi, _, _, _) = make_inchi_from_molfile_text(
            self.inchi_lib,
            self.molfile,
            "",
        )
        print(inchi)


if __name__ == "__main__":

    threads = [
        ConsumerThread(INCHI_LIB, molfile)
        for molfile in read_records_from_gzipped_sdf(SDF_PATH)
    ]

    for thread in threads:
        thread.start()

    for thread in threads:
        thread.join(timeout=5)
