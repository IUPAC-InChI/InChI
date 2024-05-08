# https://py-pkgs.org/04-package-structure
import logging
from importlib.metadata import version

__version__ = version("sdf_pipeline")  # read version from pyproject.toml
logger = logging.getLogger(__name__)
