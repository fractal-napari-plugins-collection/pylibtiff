"""
This module contains a wrapper around libtiff's TIFF handle.
"""

import numpy as np

from pylibtiff.utils import wrap_index

from pylibtiff.ext.tiff_file import TiffFile as TiffFileExtension


class TiffFile:
    """
    Wrapper class around libtiff's TIFF handle.
    """

    _tiff_file_ext = None
    """
    Reference to the internal TiffFile extension.
    """
    subfile_tags = []
    """
    Dictionary of all TIFF Tags per subfile.
    """

    def __init__(self, file_path, version=42):
        """
        TiffFile constructor.

        :param file_path: Path to the TIFF file.
        :param version: Version of the TIFF file (default = 42, BigTIFF = 43).
        """
        self._tiff_file_ext = TiffFileExtension(file_path, version)
        self.subfile_tags = [
            self._tiff_file_ext.get_subfile_tags(subfile_idx)
            for subfile_idx in range(self._tiff_file_ext.get_subfile_count())
        ]

    @property
    def file_path(self):
        """
        Path to the TIFF file.

        :return: The path to the TIFF file.
        """
        return self._tiff_file_ext.get_file_path()

    @property
    def version(self):
        """
        Version of the TIFF file (default = 42, BigTIFF = 43).

        :return: The version of the TIFF file.
        """
        return self._tiff_file_ext.get_version()

    def read(self):
        """
        Reads the first subfile.

        :return: An image as Numpy array.
        """
        if self.subfile_tags[0].bits_per_sample == 8:
            return self._tiff_file_ext.read_8()
        elif self.subfile_tags[0].bits_per_sample == 16:
            return self._tiff_file_ext.read_16()
        else:
            raise RuntimeError(
                "Cannot read from TIFF file! " +
                "Only 8bit and 16bit images are supported."
            )

    def read_subfile(self, subfile_idx):
        """
        Reads a subfile.

        :param subfile_idx: Index of the subfile.
        :return: An image as Numpy array.
        """
        subfile_idx = wrap_index(subfile_idx, len(self.subfile_tags))

        if self.subfile_tags[subfile_idx].bits_per_sample == 8:
            return self._tiff_file_ext.read_subfile_8(subfile_idx)
        elif self.subfile_tags[subfile_idx].bits_per_sample == 16:
            return self._tiff_file_ext.read_subfile_16(subfile_idx)
        else:
            raise RuntimeError(
                "Cannot read from TIFF file! " +
                "Only 8bit and 16bit images are supported."
            )

    def read_subfile_region(self, subfile_idx, x1, y1, x2, y2):
        """
        Reads a region from a subfile.

        :param subfile_idx: Index of the subfile.
        :param x1: Upper left x-coordinate (incl).
        :param y1: Upper left y-coordinate (incl).
        :param x2: Lower right x-coordinate (excl).
        :param y2: Lower right y-coordinate (excl).
        :return: A region as Numpy array.
        """
        subfile_idx = wrap_index(subfile_idx, len(self.subfile_tags))

        if self.subfile_tags[subfile_idx].bits_per_sample == 8:
            return self._tiff_file_ext.read_subfile_region_8(
                subfile_idx, x1, y1, x2, y2
            )
        elif self.subfile_tags[subfile_idx].bits_per_sample == 16:
            return self._tiff_file_ext.read_subfile_region_16(
                subfile_idx, x1, y1, x2, y2
            )
        else:
            raise RuntimeError(
                "Cannot read from TIFF file! " +
                "Only 8bit and 16bit images are supported."
            )

    def write(self, np_array, tile_size=0):
        """
        Writes a new subfile to the end of the TIFF file.

        .. seealso:: :func:`write_subfile`

        :param np_array: Image data as a Numpy array.
        :param tile_size: If set, writes the image in tiles.
                          Otherwise, writes the image in strips.
        """
        return self.write_subfile(np_array, tile_size)

    def write_subfile(self, np_array, tile_size=0):
        """
        Writes a new subfile to the end of the TIFF file.

        :param np_array: Image data as a Numpy array.
        :param tile_size: If set, writes the image in tiles.
                          Otherwise, writes the image in strips.
        """
        bits_per_sample = 0
        if np_array.dtype == np.uint8:
            bits_per_sample = 8
        if np_array.dtype == np.uint16:
            bits_per_sample = 16

        tiff_tags = TiffFileExtension.TiffTags()
        tiff_tags.new_subfile_type = 0  # undefined
        tiff_tags.image_width = np_array.shape[1]
        tiff_tags.image_length = np_array.shape[0]
        tiff_tags.bits_per_sample = bits_per_sample
        tiff_tags.compression = 5  # LZW
        tiff_tags.photometric = 1  # min is black
        tiff_tags.samples_per_pixel = 1
        tiff_tags.rows_per_strip = 2**32 - 1
        tiff_tags.min_sample_value = np.min(np_array)
        tiff_tags.max_sample_value = np.max(np_array)
        tiff_tags.planar_config = 1  # chunky format
        tiff_tags.tile_width = tile_size
        tiff_tags.tile_length = tile_size
        tiff_tags.sample_format = 1  # unsigned integer

        if np_array.dtype == np.uint8:
            self._tiff_file_ext.write_subfile_8(
                np_array, tiff_tags, tile_size > 0
            )
        elif np_array.dtype == np.uint16:
            self._tiff_file_ext.write_subfile_16(
                np_array, tiff_tags, tile_size > 0
            )
        else:
            raise RuntimeError(
                "Cannot write to TIFF file! " +
                "Only 8bit and 16bit Numpy arrays are supported."
            )

        self.subfile_tags.append(tiff_tags)

    def write_subfile_region(self, np_array, subfile_idx, x1, y1, x2, y2):
        """
        Writes a region into an existing subfile.

        .. note:: libtiff does no support altering the contents of a
                  TIFF file.

        :param np_array: Region data as a Numpy array.
        :param subfile_idx: Index of the subfile.
        :param x1: Upper left x-coordinate (incl).
        :param y1: Upper left y-coordinate (incl).
        :param x2: Lower right x-coordinate (excl).
        :param y2: Lower right y-coordinate (excl).
        """
        subfile_idx = wrap_index(subfile_idx, len(self.subfile_tags))

        if np_array.dtype == np.uint8:
            return self._tiff_file_ext.write_subfile_region_8(
                np_array, subfile_idx, x1, y1, x2, y2
            )
        elif np_array.dtype == np.uint16:
            return self._tiff_file_ext.write_subfile_region_16(
                np_array, subfile_idx, x1, y1, x2, y2
            )
        else:
            raise RuntimeError(
                "Cannot write to TIFF file! " +
                "Only 8bit and 16bit Numpy arrays are supported."
            )

    def write_multiscale_subfile(self, np_array, tile_size):
        """
        Writes a new multi-scale subfile into a TIFF file.

        :param np_array: Image data as a Numpy array.
        :param tile_size: size of the tile width and tile length.
        """
        tiff_tags = TiffFileExtension.TiffTags()
        tiff_tags.new_subfile_type = 0  # undefined
        tiff_tags.image_width = np_array.shape[1]
        tiff_tags.image_length = np_array.shape[0]
        tiff_tags.bits_per_sample = 8
        tiff_tags.compression = 5  # LZW
        tiff_tags.photometric = 1  # min is black
        tiff_tags.samples_per_pixel = 1
        tiff_tags.rows_per_strip = 2**32 - 1
        tiff_tags.min_sample_value = 0
        tiff_tags.max_sample_value = 255
        tiff_tags.planar_config = 1  # chunky format
        tiff_tags.tile_width = tile_size
        tiff_tags.tile_length = tile_size
        tiff_tags.sample_format = 1  # unsigned integer

        if np_array.dtype == np.uint8:
            self._tiff_file_ext.write_multiscale_subfile_8(
                np_array, tiff_tags
            )
        elif np_array.dtype == np.uint16:
            self._tiff_file_ext.write_multiscale_subfile_16(
                np_array, tiff_tags
            )
        else:
            raise RuntimeError(
                "Cannot write to TIFF file! " +
                "Only 8bit and 16bit Numpy arrays are supported."
            )

        self.subfile_tags = [
            self._tiff_file_ext.get_subfile_tags(subfile_idx)
            for subfile_idx in range(self._tiff_file_ext.get_subfile_count())
        ]
