"""
Unittests for pylibtiff.ext.tiff_file module.
"""
import numpy as np
import os
import unittest

from pylibtiff.ext.tiff_file import TiffFile


def parameterized(params_list):
    def parameterized_decorator(func):
        def func_wrapper(self):
            for params in params_list:
                func(self, **params)
        return func_wrapper
    return parameterized_decorator


class TiffFileTests(unittest.TestCase):
    """
    A simple test suite for the pylibtiff.ext.tiff_file module.
    """
    parameter_list = [
        {
            'file_path': './tests/data/grad1024_scanline_8bpp_32bit.tif',
            'is_tiled': False,
            'bits_per_sample': 8,
        },
        {
            'file_path': './tests/data/grad1024_tiled_8bpp_32bit.tif',
            'is_tiled': True,
            'bits_per_sample': 8,
        },
        {
            'file_path': './tests/data/grad1024_scanline_16bpp_32bit.tif',
            'is_tiled': False,
            'bits_per_sample': 16,
        },
        {
            'file_path': './tests/data/grad1024_tiled_16bpp_32bit.tif',
            'is_tiled': True,
            'bits_per_sample': 16,
        }
    ]

    @classmethod
    def setUpClass(cls):
        pass

    @classmethod
    def tearDownClass(cls):
        pass

    @parameterized(parameter_list)
    def test_init(self, file_path, is_tiled, bits_per_sample):
        """
        Test for the TiffFile.get_file_path() method.
        """
        ptif = TiffFile(file_path)
        self.assertEqual(ptif.get_subfile_count(), 2)

    @parameterized(parameter_list)
    def test_get_file_path(self, file_path, is_tiled, bits_per_sample):
        """
        Test for the TiffFile.get_file_path() method.
        """
        ptif = TiffFile(file_path)
        self.assertEqual(ptif.get_file_path(), file_path)

    @parameterized(parameter_list)
    def test_get_version(self, file_path, is_tiled, bits_per_sample):
        """
        Test for the TiffFile.get_version() method.
        """
        ptif = TiffFile(file_path)
        self.assertEqual(ptif.get_version(), 42)

    @parameterized(parameter_list)
    def test_get_subfile_count(self, file_path, is_tiled, bits_per_sample):
        """
        Test for the TiffFile.get_subfile_count() method.
        """
        ptif = TiffFile(file_path)
        self.assertEqual(ptif.get_subfile_count(), 2)

    @parameterized(parameter_list)
    def test_get_subfile_tags(self, file_path, is_tiled, bits_per_sample):
        """
        Test for the TiffFile.get_subfile_tags() method.
        """
        ptif = TiffFile(file_path)
        self.assertTrue(
            isinstance(ptif.get_subfile_tags(0), TiffFile.TiffTags)
        )

    @parameterized(parameter_list)
    def test_get_subfile_type(self, file_path, is_tiled, bits_per_sample):
        """
        Test for the TiffFile.get_subfile_type() method.
        """
        ptif = TiffFile(file_path)
        page_type = 2
        self.assertEqual(ptif.get_subfile_type(0), page_type)

    @parameterized(parameter_list)
    def test_get_image_width(self, file_path, is_tiled, bits_per_sample):
        """
        Test for the TiffFile.get_image_width() method.
        """
        ptif = TiffFile(file_path)
        self.assertEqual(ptif.get_image_width(0), 1024)

    @parameterized(parameter_list)
    def test_get_image_length(self, file_path, is_tiled, bits_per_sample):
        """
        Test for the TiffFile.get_image_length() method.
        """
        ptif = TiffFile(file_path)
        self.assertEqual(ptif.get_image_length(0), 1024)

    @parameterized(parameter_list)
    def test_get_bits_per_sample(self, file_path, is_tiled, bits_per_sample):
        """
        Test for the TiffFile.get_bits_per_sample() method.
        """
        ptif = TiffFile(file_path)
        self.assertEqual(ptif.get_bits_per_sample(0), bits_per_sample)

    @parameterized(parameter_list)
    def test_get_compression(self, file_path, is_tiled, bits_per_sample):
        """
        Test for the TiffFile.get_compression() method.
        """
        ptif = TiffFile(file_path)
        none, deflate = 1, 8
        self.assertEqual(ptif.get_compression(0), none)

    @parameterized(parameter_list)
    def test_get_photometric_interpretation(
        self, file_path, is_tiled, bits_per_sample
    ):
        """
        Test for the TiffFile.get_photometric_interpretation() method.
        """
        ptif = TiffFile(file_path)
        min_is_black = 1
        self.assertEqual(ptif.get_photometric_interpretation(0), min_is_black)

    @parameterized(parameter_list)
    def test_get_samples_per_pixel(self, file_path, is_tiled, bits_per_sample):
        """
        Test for the TiffFile.get_samples_per_pixel() method.
        """
        ptif = TiffFile(file_path)
        self.assertEqual(ptif.get_samples_per_pixel(0), 1)

    @parameterized(parameter_list)
    def test_get_rows_per_strip(self, file_path, is_tiled, bits_per_sample):
        """
        Test for the TiffFile.get_rows_per_strip() method.
        """
        ptif = TiffFile(file_path)
        if is_tiled:
            self.assertEqual(ptif.get_rows_per_strip(0), 4294967295)
        else:
            self.assertNotEqual(ptif.get_rows_per_strip(0), 4294967295)

    @parameterized(parameter_list)
    def test_get_min_sample_value(self, file_path, is_tiled, bits_per_sample):
        """
        Test for the TiffFile.get_min_sample_value() method.
        """
        ptif = TiffFile(file_path)
        self.assertEqual(ptif.get_min_sample_value(0), 0)

    @parameterized(parameter_list)
    def test_get_max_sample_value(self, file_path, is_tiled, bits_per_sample):
        """
        Test for the TiffFile.get_max_sample_value() method.
        """
        ptif = TiffFile(file_path)
        self.assertEqual(
            ptif.get_max_sample_value(0), 2 ** bits_per_sample - 1
        )

    @parameterized(parameter_list)
    def test_get_planar_configuration(
        self, file_path, is_tiled, bits_per_sample
    ):
        """
        Test for the TiffFile.get_planar_configuration() method.
        """
        ptif = TiffFile(file_path)
        contig = 1
        self.assertEqual(ptif.get_planar_configuration(0), contig)

    @parameterized(parameter_list)
    def test_get_page_number(self, file_path, is_tiled, bits_per_sample):
        """
        Test for the TiffFile.get_planar_configuration() method.
        """
        ptif = TiffFile(file_path)
        self.assertEqual(ptif.get_page_number(1), 1)

    @parameterized(parameter_list)
    def test_get_page_count(self, file_path, is_tiled, bits_per_sample):
        """
        Test for the TiffFile.get_page_count() method.
        """
        ptif = TiffFile(file_path)
        self.assertEqual(ptif.get_page_count(0), 2)

    @parameterized(parameter_list)
    def test_get_tile_width(self, file_path, is_tiled, bits_per_sample):
        """
        Test for the TiffFile.get_tile_width() method.
        """
        ptif = TiffFile(file_path)
        if is_tiled:
            self.assertNotEqual(ptif.get_tile_width(0), 0)
        else:
            self.assertEqual(ptif.get_tile_width(0), 0)

    @parameterized(parameter_list)
    def test_get_tile_length(self, file_path, is_tiled, bits_per_sample):
        """
        Test for the TiffFile.get_tile_length() method.
        """
        ptif = TiffFile(file_path)
        if is_tiled:
            self.assertNotEqual(ptif.get_tile_length(0), 0)
        else:
            self.assertEqual(ptif.get_tile_length(0), 0)

    @parameterized(parameter_list)
    def test_get_sample_format(self, file_path, is_tiled, bits_per_sample):
        """
        Test for the TiffFile.get_sample_format() method.
        """
        ptif = TiffFile(file_path)
        uint = 1
        self.assertEqual(ptif.get_sample_format(0), uint)

    @parameterized(parameter_list)
    def test_read(self, file_path, is_tiled, bits_per_sample):
        """
        Test for the TiffFile.read_8() and TiffFile.read_16() methods.
        """
        ptif = TiffFile(file_path)
        if bits_per_sample == 8:
            arr = ptif.read_8()
        else:
            arr = ptif.read_16()

        self.assertEqual(arr.shape, (1024, 1024))
        if bits_per_sample == 8:
            self.assertEqual(arr.dtype, np.uint8)
        else:
            self.assertEqual(arr.dtype, np.uint16)

    @parameterized(parameter_list)
    def test_read_subfile(self, file_path, is_tiled, bits_per_sample):
        """
        Test for the TiffFile.read_subfile_8() and TiffFile.read_subfile_16()
        methods.
        """
        ptif = TiffFile(file_path)
        if bits_per_sample == 8:
            arr = ptif.read_subfile_8(0)
        else:
            arr = ptif.read_subfile_16(0)

        self.assertEqual(arr.shape, (1024, 1024))
        if bits_per_sample == 8:
            self.assertEqual(arr.dtype, np.uint8)
        else:
            self.assertEqual(arr.dtype, np.uint16)

    @parameterized(parameter_list)
    def test_read_subfile_region(self, file_path, is_tiled, bits_per_sample):
        """
        Test for the TiffFile.read_subfile_region_8() and
        TiffFile.read_subfile_region_16() methods.
        """
        ptif = TiffFile(file_path)
        if bits_per_sample == 8:
            arr = ptif.read_subfile_region_8(0, 0, 0, 1, 1)
        else:
            arr = ptif.read_subfile_region_16(0, 0, 0, 1, 1)

        self.assertEqual(arr, 2 ** bits_per_sample - 1)
        if bits_per_sample == 8:
            self.assertEqual(arr.dtype, np.uint8)
        else:
            self.assertEqual(arr.dtype, np.uint16)

    def test_write(self):
        """
        Test for the TiffFile.write_8() methods.
        """
        try:
            ptif = TiffFile('./tests/data/test.tif')
            self.assertEqual(ptif.get_subfile_count(), 0)
            
            arr = np.array(
                [[0, 255, 0], [255, 255, 255], [0, 255, 0]], dtype=np.uint8
            )
            tiff_tags = TiffFile.TiffTags()
            tiff_tags.new_subfile_type = 2  # page type
            tiff_tags.image_width = 3
            tiff_tags.image_length = 3
            tiff_tags.bits_per_sample = 8
            tiff_tags.compression = 5  # LZW
            tiff_tags.photometric = 1  # min is black
            tiff_tags.samples_per_pixel = 1
            tiff_tags.rows_per_strip = 2**32 - 1
            tiff_tags.page_number.page_number = 1
            tiff_tags.page_number.page_count = 2
            tiff_tags.tile_width = 16
            tiff_tags.tile_length = 16
            ptif.write_8(arr, tiff_tags, True)

            tiff_tags.page_number.page_number = 2
            ptif.write_8(arr, tiff_tags, True)

            self.assertEqual(ptif.get_subfile_count(), 2)
            self.assertEqual(ptif.get_tile_length(0), 16)
        finally:
            if os.path.exists('./tests/data/test.tif'):
                os.remove('./tests/data/test.tif')

    def test_write_subfile(self):
        """
        Test for the TiffFile.write_subfile_16() methods.
        """
        try:
            ptif = TiffFile('./tests/data/test.tif')
            self.assertEqual(ptif.get_subfile_count(), 0)
            
            arr = np.array(
                [[0, 255, 0], [255, 255, 255], [0, 255, 0]], dtype=np.uint16
            )
            tiff_tags = TiffFile.TiffTags()
            tiff_tags.new_subfile_type = 2  # page type
            tiff_tags.image_width = 3
            tiff_tags.image_length = 3
            tiff_tags.bits_per_sample = 16
            tiff_tags.compression = 5  # LZW
            tiff_tags.photometric = 1  # min is black
            tiff_tags.samples_per_pixel = 1
            tiff_tags.rows_per_strip = 2**32 - 1
            tiff_tags.page_number.page_number = 1
            tiff_tags.page_number.page_count = 2
            tiff_tags.tile_width = 16
            tiff_tags.tile_length = 16
            ptif.write_subfile_16(arr, tiff_tags, True)

            tiff_tags.page_number.page_number = 2
            ptif.write_subfile_16(arr, tiff_tags, True)

            self.assertEqual(ptif.get_subfile_count(), 2)
            self.assertEqual(ptif.get_tile_length(0), 16)
        finally:
            if os.path.exists('./tests/data/test.tif'):
                os.remove('./tests/data/test.tif')

    def test_write_subfile_region(self):
        """
        Test for the TiffFile.write_subfile_region_8() and
        TiffFile.write_subfile_region_16() methods.
        """
        ptif = TiffFile("./tests/data/grad1024_tiled_8bpp_32bit.tif")
        arr8 = np.ones((512, 512), dtype=np.uint8) * 255
        arr16 = np.ones((512, 512), dtype=np.uint16) * 255
        with self.assertRaises(RuntimeError):
            ptif.write_subfile_region_8(arr8, 0, 256, 256, 768, 768)
        with self.assertRaises(RuntimeError):
            ptif.write_subfile_region_16(arr16, 0, 256, 256, 768, 768)

    def test_write_multiscale_subfile(self):
        """
        Test for the TiffFile.write_multiscale_subfile_8() methods.
        """
        try:
            ptif = TiffFile('./tests/data/test.tif')
            self.assertEqual(ptif.get_subfile_count(), 0)
            
            arr = np.zeros((100, 100), dtype=np.uint8)
            radius = 50
            for y in range(-radius, radius):
                for x in range(-radius, radius):
                    arr[y+radius, x+radius] = x**2 + y**2 <= radius**2

            tiff_tags = TiffFile.TiffTags()
            tiff_tags.new_subfile_type = 1  # page type
            tiff_tags.image_width = 100
            tiff_tags.image_length = 100
            tiff_tags.bits_per_sample = 8
            tiff_tags.compression = 5  # LZW
            tiff_tags.photometric = 1  # min is black
            tiff_tags.samples_per_pixel = 1
            tiff_tags.rows_per_strip = 2**32 - 1
            tiff_tags.tile_width = 16
            tiff_tags.tile_length = 16
            ptif.write_multiscale_subfile_8(arr, tiff_tags)

            self.assertEqual(ptif.get_subfile_count(), 5)
            self.assertEqual(ptif.get_tile_length(0), 16)
        finally:
            if os.path.exists('./tests/data/test.tif'):
                os.remove('./tests/data/test.tif')


if __name__ == '__main__':
    unittest.main()
