import subprocess
import shlex
from .utils import get_dataset_arg, DOWNLOAD_PATHS, PUBCHEM_DIR


if __name__ == "__main__":
    dataset = get_dataset_arg()

    # https://depth-first.com/articles/2010/02/09/big-data-in-chemistry-mirroring-pubchem-the-easy-way-part-2/
    download_command = (
        f"wget --mirror --directory-prefix {PUBCHEM_DIR.joinpath(dataset)} "
        + "--no-directories --continue --accept '*.sdf.gz,*.sdf.gz.md5' "
        + f"ftp://ftp.ncbi.nlm.nih.gov/pubchem/{DOWNLOAD_PATHS[dataset]}/SDF/"
    )

    subprocess.run(shlex.split(download_command), check=True)
