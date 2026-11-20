import gzip
from pathlib import Path
from typing import Generator, Callable


def read_records_from_gzipped_sdf(sdf_path: Path) -> Generator[str, None, None]:
    # https://en.wikipedia.org/wiki/Chemical_table_file#SDF"
    current_record = ""
    # A data field's value can be the line `$$$$`: PubChem substance record
    # SID 172438852 has it as the value of PUBCHEM_EXT_DATASOURCE_REGID and of
    # PUBCHEM_XREF_EXT_ID. Splitting on every such line splits that record into
    # three, and the two extra parts get the same molfile ID from
    # `get_molfile_id`.
    #
    # Only a `$$$$` outside a field value ends a record. A `>` header line starts
    # a value, a blank line ends it, and every line in between is part of the
    # value.
    # http://www.dalkescientific.com/writings/diary/archive/2020/09/18/handling_the_sdf_record_delimiter.html
    inside_data_value = False
    # TODO: guard file opening.
    with gzip.open(sdf_path, "rb") as gzipped_sdf:
        # Decompress SDF line-by-line to avoid loading entire SDF into memory.
        for decompressed_line in gzipped_sdf:
            decoded_line = decompressed_line.decode("utf-8", "backslashreplace")
            current_record += decoded_line
            # Check the first character first: it is cheap, and most lines are
            # connection-table or value lines that match none of the three
            # cases. `>` and `$$$$` both start in column 1 of an SDF.
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
