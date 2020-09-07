"""
This module contains wrappers for data structure and functions available within
the the libtiff library.
"""

from pylibtiff.tiff_file import TiffFile
from pylibtiff.ext.tiff_file import TiffFile as TiffFileExtension
TiffTags = TiffFileExtension.TiffTags  # type alias
