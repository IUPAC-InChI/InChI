"""https://depth-first.com/articles/2010/02/09/big-data-in-chemistry-mirroring-pubchem-the-easy-way-part-2/"""
import subprocess
import shlex
from ...config import get_dataset_arg, DATASETS

if __name__ == "__main__":
    dataset = get_dataset_arg()

    download_command = (
        f"wget --mirror --directory-prefix {DATASETS[dataset]['log_path']} "
        + "--no-directories --continue --accept '*.sdf.gz,*.sdf.gz.md5' "
        + f"ftp://ftp.ncbi.nlm.nih.gov/pubchem/{DATASETS[dataset]['download_path']}/SDF/"
    )

    subprocess.run(shlex.split(download_command), check=True)
