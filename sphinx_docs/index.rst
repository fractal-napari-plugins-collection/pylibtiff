
=====================================
Welcome to PyLibTIFF's documentation!
=====================================

PyLibTIFF is a lightwight wrapper around the LibTIFF library (http://www.libtiff.org/).
LibTIFF is a widely used C library for reading and writing Tagged Image File
Format (TIFF) files.

The current implementation supports:

- Version: 42 (TIFF), 43 (BigTIFF)
- Storage organization: strip-based or tile-based
- Color depth: 8bit and 16bit
- Sub-File type: reduced image, page, mask

.. note: The library was tested on Windows and Linux - not (yet) on MacOS!

-----------------
Table of contents
-----------------

.. toctree::
    :maxdepth: 2
    :numbered:
    
    pylibtiff/user_guide
    pylibtiff/api_reference
