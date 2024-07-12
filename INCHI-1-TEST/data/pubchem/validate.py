import hashlib
from inchi_tests.utils import get_progress
from .utils import get_dataset_arg, PUBCHEM_DIR


if __name__ == "__main__":
    dataset = get_dataset_arg()
    sdf_paths = sorted(PUBCHEM_DIR.joinpath(dataset).glob("*.sdf.gz"))
    n_sdf = len(sdf_paths)

    for i, sdf_path in enumerate(sdf_paths):
        with open(sdf_path, "rb") as sdf_file:
            print(
                f"{get_progress(i + 1, n_sdf)}; Validating integrity of {sdf_path.name}."
            )
            local_hash = hashlib.file_digest(sdf_file, "md5").hexdigest()
            try:
                with open(sdf_path.with_suffix(".gz.md5"), "r") as md5_file:
                    server_hash = md5_file.read().split()[0].strip()
                    if local_hash != server_hash:
                        print(
                            f"{sdf_path.name}: local hash {local_hash} does not match server hash {server_hash}."
                        )
            except FileNotFoundError as e:
                print(e)
