import gzip
from pathlib import Path
from typing import Generator, Callable


def read_records_from_gzipped_sdf(sdf_path: Path) -> Generator[str, None, None]:
    # https://en.wikipedia.org/wiki/Chemical_table_file#SDF"
    current_record = ""
    # TODO: guard file opening.
    with gzip.open(sdf_path, "rb") as gzipped_sdf:
        # Decompress SDF line-by-line to avoid loading entire SDF into memory.
        for decompressed_line in gzipped_sdf:
            decoded_line = decompressed_line.decode("utf-8", "backslashreplace")
            current_record += decoded_line
            if decoded_line.strip() == "$$$$":
                # TODO: harden SDF parsing according to
                # http://www.dalkescientific.com/writings/diary/archive/2020/09/18/handling_the_sdf_record_delimiter.html.
                yield current_record
                current_record = ""

    return None


def select_records_from_gzipped_sdf(
    sdf_path: Path, molfile_ids: set[str], get_molfile_id: Callable
) -> Generator[tuple[str, str], None, None]:

    for molfile in read_records_from_gzipped_sdf(sdf_path):
        molfile_id = get_molfile_id(molfile)
        if molfile_id in molfile_ids:
            yield molfile_id, molfile

    return None
