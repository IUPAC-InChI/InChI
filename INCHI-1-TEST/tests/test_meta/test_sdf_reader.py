"""Record splitting in `sdf_pipeline.utils.read_records_from_gzipped_sdf`.

The delimiter `$$$$` is also legal content: it can be the value of a data field.
PubChem substance 172438852 has one as its `PUBCHEM_EXT_DATASOURCE_REGID`, which
used to cut that record into three and abort the whole shard on a duplicate
molfile ID.
"""

import gzip
import pytest
from sdf_pipeline.utils import read_records_from_gzipped_sdf


def _record(title: str, fields: list[tuple[str, str]]) -> str:
    """A minimal but structurally real SDF record."""
    ctab = f"{title}\n  test\n\n  0  0  0  0  0  0  0  0  0  0999 V2000\nM  END\n"
    data = "".join(f"> <{tag}>\n{value}\n\n" for tag, value in fields)

    return f"{ctab}{data}$$$$\n"


@pytest.fixture
def write_sdf(tmp_path):
    def _write(records: list[str]):
        path = tmp_path / "test.sdf.gz"
        with gzip.open(path, "wt", encoding="utf-8") as sdf:
            sdf.write("".join(records))
        return path

    return _write


def _ids(path) -> list[str]:
    # `get_molfile_id_pubchem` reads the first token of the record.
    return [record.split()[0].strip() for record in read_records_from_gzipped_sdf(path)]


def test_plain_records_split_on_the_delimiter(write_sdf):
    path = write_sdf(
        [
            _record("1001", [("PUBCHEM_SUBSTANCE_ID", "1001")]),
            _record("1002", [("PUBCHEM_SUBSTANCE_ID", "1002")]),
        ]
    )
    assert _ids(path) == ["1001", "1002"]


def test_a_field_whose_value_is_the_delimiter_stays_one_record(write_sdf):
    """The shape of PubChem substance 172438852."""
    path = write_sdf(
        [
            _record(
                "172438852",
                [
                    ("PUBCHEM_SUBSTANCE_ID", "172438852"),
                    ("PUBCHEM_EXT_DATASOURCE_REGID", "$$$$"),
                    ("PUBCHEM_EXT_DATASOURCE_URL", "http://example.invalid"),
                ],
            ),
            _record("172438853", [("PUBCHEM_SUBSTANCE_ID", "172438853")]),
        ]
    )
    records = list(read_records_from_gzipped_sdf(path))

    assert len(records) == 2
    assert _ids(path) == ["172438852", "172438853"]
    # The value must survive inside the record it belongs to, not start a new one.
    assert "PUBCHEM_EXT_DATASOURCE_URL" in records[0]


def test_consecutive_delimiter_valued_fields_stay_one_record(write_sdf):
    """Two in a row, which is what 172438852 actually has."""
    path = write_sdf(
        [
            _record(
                "172438852",
                [
                    ("PUBCHEM_EXT_DATASOURCE_REGID", "$$$$"),
                    ("PUBCHEM_XREF_EXT_ID", "$$$$"),
                    ("PUBCHEM_SUBSTANCE_ID", "172438852"),
                ],
            )
        ]
    )
    assert _ids(path) == ["172438852"]


def test_ids_stay_unique_where_they_used_to_collide(write_sdf):
    """The abort was `UNIQUE constraint failed: results.molfile_id`.

    Splitting inside the record left fragments whose first token was `>`, so two
    of them in one shard collided and took the whole shard down."""
    path = write_sdf(
        [
            _record("1", [("PUBCHEM_EXT_DATASOURCE_REGID", "$$$$")]),
            _record("2", [("PUBCHEM_XREF_EXT_ID", "$$$$")]),
        ]
    )
    ids = _ids(path)

    assert ids == ["1", "2"]
    assert len(ids) == len(set(ids))


def test_multiline_field_value_containing_the_delimiter(write_sdf):
    path = write_sdf([_record("7", [("NOTES", "first line\n$$$$\nlast line")])])
    records = list(read_records_from_gzipped_sdf(path))

    assert len(records) == 1
    assert "last line" in records[0]
