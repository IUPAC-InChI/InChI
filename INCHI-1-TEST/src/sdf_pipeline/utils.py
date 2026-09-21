import gzip
from pathlib import Path
from typing import Generator, Callable


def read_records_from_gzipped_sdf(sdf_path: Path) -> Generator[str, None, None]:
    # https://en.wikipedia.org/wiki/Chemical_table_file#SDF"
    current_record = ""
    # A data field's value can itself be the line `$$$$`: PubChem substance
    # 172438852 carries one as its PUBCHEM_EXT_DATASOURCE_REGID. Splitting on
    # every such line cuts that record in three, and the two remainders collide
    # on whatever `get_molfile_id` makes of them.
    #
    # Only a `$$$$` outside a field value delimits a record, so track where we
    # are: a `>` header opens a value, a blank line closes it, and everything
    # between belongs to the value however it reads.
    # http://www.dalkescientific.com/writings/diary/archive/2020/09/18/handling_the_sdf_record_delimiter.html
    inside_data_value = False
    # TODO: guard file opening.
    with gzip.open(sdf_path, "rb") as gzipped_sdf:
        # Decompress SDF line-by-line to avoid loading entire SDF into memory.
        for decompressed_line in gzipped_sdf:
            decoded_line = decompressed_line.decode("utf-8", "backslashreplace")
            current_record += decoded_line
            # Test the first character before anything that allocates: almost
            # every line is a connection-table or value line that is none of the
            # three cases, and this runs once per line of the corpus. `>` and
            # `$$$$` both start at column 1 in an SDF.
            first_character = decoded_line[:1]
            if first_character == "$":
                if not inside_data_value and decoded_line.strip() == "$$$$":
                    yield current_record
                    current_record = ""
            elif first_character == ">":
                inside_data_value = True
            elif decoded_line.isspace():
                inside_data_value = False

    return None


def select_records_from_gzipped_sdf(
    sdf_path: Path, molfile_ids: set[str], get_molfile_id: Callable
) -> Generator[tuple[str, str], None, None]:

    for molfile in read_records_from_gzipped_sdf(sdf_path):
        molfile_id = get_molfile_id(molfile)
        if molfile_id in molfile_ids:
            yield molfile_id, molfile

    return None
