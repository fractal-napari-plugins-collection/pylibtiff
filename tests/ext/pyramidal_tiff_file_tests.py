"""
Unittests for pylibtiff.ext.pyramidal_tiff_file module.
"""
import unittest

from pylibtiff.ext.pyramidal_tiff_file import PyramidalTiffFile


class PyramidalTiffFileTests(unittest.TestCase):
    """
    A simple test suite for the pylibtiff.ext.pyramidal_tiff_file module.
    """

    @classmethod
    def setUpClass(cls):
        pass

    @classmethod
    def tearDownClass(cls):
        pass

    def test_get_page_count(self):
        """
        Test for the PyramidalTiffFile.get_page_count() method.
        """
        img_path = './tests/data/grad1024.tif'
        ptif = PyramidalTiffFile(img_path)

        self.assertEqual(ptif.get_page_count(), 5)

    def test_crop(self):
        """
        Test for the PyramidalTiffFile.crop() method.
        """
        img_path = './tests/data/grad1024.tif'
        ptif = PyramidalTiffFile(img_path)

        self.assertEqual(ptif.crop(0, 1023, 1, 1024, 0), 255)

if __name__ == '__main__':
    unittest.main()
