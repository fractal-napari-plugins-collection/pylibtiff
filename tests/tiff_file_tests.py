"""
Unittests for pylibtiff.tiff_file module.
"""
import numpy as np
import os
import unittest

from pylibtiff import TiffFile, TiffTags


def parameterized(params_list):
    def parameterized_decorator(func):
        def func_wrapper(self):
            for params in params_list:
                func(self, **params)
        return func_wrapper
    return parameterized_decorator


class TiffFileTests(unittest.TestCase):
    """
    A simple test suite for the pylibtiff.tiff_file module.
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
        Test for the TiffFile.__init__() method.
        """
        ptif = TiffFile(file_path)
        self.assertEqual(len(ptif.subfile_tags), 2)

    @parameterized(parameter_list)
    def test_file_path(self, file_path, is_tiled, bits_per_sample):
        """
        Test for the TiffFile.file_path() method.
        """
        ptif = TiffFile(file_path)
        self.assertEqual(ptif.file_path, file_path)

    @parameterized(parameter_list)
    def test_version(self, file_path, is_tiled, bits_per_sample):
        """
        Test for the TiffFile.version() method.
        """
        ptif = TiffFile(file_path)
        self.assertEqual(ptif.version, 42)

    @parameterized(parameter_list)
    def test_subfile_tags(self, file_path, is_tiled, bits_per_sample):
        """
        Test for the TiffFile.subfile_tags() method.
        """
        ptif = TiffFile(file_path)
        self.assertTrue(
            isinstance(ptif.subfile_tags[0], TiffTags)
        )
        self.assertEqual(ptif.subfile_tags[0].image_width, 1024)
        self.assertEqual(ptif.subfile_tags[1].image_width, 512)

    @parameterized(parameter_list)
    def test_read(self, file_path, is_tiled, bits_per_sample):
        """
        Test for the TiffFile.read() methods.
        """
        ptif = TiffFile(file_path)
        arr = ptif.read()

        self.assertEqual(arr.shape, (1024, 1024))
        if bits_per_sample == 8:
            self.assertEqual(arr.dtype, np.uint8)
        else:
            self.assertEqual(arr.dtype, np.uint16)

    @parameterized(parameter_list)
    def test_read_subfile(self, file_path, is_tiled, bits_per_sample):
        """
        Test for the TiffFile.read_subfile() methods.
        """
        ptif = TiffFile(file_path)
        arr = ptif.read_subfile(1)

        self.assertEqual(arr.shape, (512, 512))
        if bits_per_sample == 8:
            self.assertEqual(arr.dtype, np.uint8)
        else:
            self.assertEqual(arr.dtype, np.uint16)

    @parameterized(parameter_list)
    def test_read_subfile_region(self, file_path, is_tiled, bits_per_sample):
        """
        Test for the TiffFile.read_subfile_region() methods.
        """
        ptif = TiffFile(file_path)
        arr = ptif.read_subfile_region(0, 0, 0, 1, 1)

        self.assertEqual(arr, 2 ** bits_per_sample - 1)
        if bits_per_sample == 8:
            self.assertEqual(arr.dtype, np.uint8)
        else:
            self.assertEqual(arr.dtype, np.uint16)

    def test_write_subfile(self):
        """
        Test for the TiffFile.write() methods.
        """
        try:
            ptif = TiffFile('./tests/data/test.tif')
            self.assertEqual(len(ptif.subfile_tags), 0)

            arr = np.array(
                [[0, 255, 0], [255, 255, 255], [0, 255, 0]], dtype=np.uint8
            )
            ptif.write(arr, tile_size=16)
            ptif.write(arr, tile_size=16)

            self.assertEqual(len(ptif.subfile_tags), 2)
            self.assertEqual(ptif.subfile_tags[0].tile_length, 16)
        finally:
            if os.path.exists('./tests/data/test.tif'):
                os.remove('./tests/data/test.tif')

    def test_write_subfile(self):
        """
        Test for the TiffFile.write_subfile() methods.
        """
        try:
            ptif = TiffFile('./tests/data/test.tif')
            self.assertEqual(len(ptif.subfile_tags), 0)

            arr = np.array(
                [[0, 255, 0], [255, 255, 255], [0, 255, 0]], dtype=np.uint8
            )
            ptif.write_subfile(arr, tile_size=16)
            ptif.write_subfile(arr, tile_size=16)

            self.assertEqual(len(ptif.subfile_tags), 2)
            self.assertEqual(ptif.subfile_tags[0].tile_length, 16)
        finally:
            if os.path.exists('./tests/data/test.tif'):
                os.remove('./tests/data/test.tif')

    def test_write_subfile_region(self):
        """
        Test for the TiffFile.write_subfile_region() methods.
        """
        ptif = TiffFile("./tests/data/grad1024_tiled_8bpp_32bit.tif")
        arr = np.ones((512, 512), dtype=np.uint8) * 255
        with self.assertRaises(RuntimeError):
            ptif.write_subfile_region(arr, 0, 256, 256, 768, 768)

    def test_write_multiscale_subfile(self):
        """
        Test for the TiffFile.write_multiscale_subfile() methods.
        """
        try:
            ptif = TiffFile('./tests/data/test.tif')
            self.assertEqual(len(ptif.subfile_tags), 0)

            arr = np.zeros((100, 100), dtype=np.uint8)
            radius = 50
            for y in range(-radius, radius):
                for x in range(-radius, radius):
                    arr[y+radius, x+radius] = x**2 + y**2 <= radius**2
            
            ptif.write_multiscale_subfile(arr, tile_size=16)

            self.assertEqual(len(ptif.subfile_tags), 5)
            self.assertEqual(ptif.subfile_tags[0].tile_length, 16)
        finally:
            if os.path.exists('./tests/data/test.tif'):
                os.remove('./tests/data/test.tif')


if __name__ == '__main__':
    unittest.main()
