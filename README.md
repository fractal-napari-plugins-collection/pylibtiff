![Release](https://img.shields.io/badge/dynamic/json?color=blue&label=release&prefix=v&query=version&url=https%3A%2F%2Fraw.githubusercontent.com%2Ffmi-basel%2Fpylibtiff%2Fmaster%2Finfo.json%3Ftoken%3DAFSHUIRGODW7LWHDAGLCMAK7HJ5H2)
![Python](https://img.shields.io/badge/python-3.6%20|%203.8-blue)
![Platform](https://img.shields.io/badge/platform-windows%20%7C%20linux%20%7C%20macos-lightgrey)
![Build](https://github.com/fmi-basel/pylibtiff/workflows/build/badge.svg?branch=master)

# PyLibTIFF
A lightwight wrapper around the LibTIFF library (http://www.libtiff.org/).
LibTIFF is a widely used C library for reading and writing Tagged Image File
Format (TIFF) files.

> The library was tested on Windows and Linux - not (yet) on MacOS!

## Installation 
To avoid conflicts with other packages, it is recommended to install PyLibTIFF
within a virtual environment. Please refer to "The Python Tutorial"
for further information (https://docs.python.org/3/tutorial/venv.html).

### Installation from Wheel package
PyLibTIFF can be installed using a pre-compiled Wheel package available at
https://github.com/fmi-basel/pylibtiff/releases.
After downloading a wheel matching your operating system, your CPU architecture
and Python version you can install the package via pip. E.g., the following
command installs PyLibTIFF v0.1.0 on a 64bit Windows with Python 3.6:

```
pip install pylibtiff-0.1.0-cp36-cp36m-win_amd64.whl
```

### Installation from source code
To install PyLibTIFF from the source code you need a C/C++ compiler (e.g. GCC)
and the following C libraries (as devel package or source code):
- zlib (https://zlib.net/) [tested: v1.2.11]
- libjpeg (https://www.ijg.org/) [tested: v9.4.0]
- libtiff (http://www.libtiff.org/) [tested: v4.0.7]

Moreover, the following Python packages are required:
- pybind11 [tested: v2.2.4]
- wheel (optional, required for compiling Wheel packages) [tested: v0.34.2]

PyLibTIFF can be installed via pip...

```
pip install .
```

...or, compiled as a Wheel package.

```
python setup.py bdist_wheel
```

> Under Linux, PyLibTIFF is linked agains dynamic libraries as recommended by
> Red Hat while on Windows it is linked agains static libraries. This reflects
> the expectation of the two user groups. While it is common to install devel
> packages and to compile binaries on Linux a Windows user expects a pre-build
> binary.

#### Troubleshooting
Depending on your development environment you many need to specify additional
include or library paths for the compiler or linker. This can be specified by
the "build_ext" option:

```
pip install . --global-option=build_ext --global-option="-I/path/to/include-dir" --global-option="-L/path/to/library-dir"
```

## Usage
The following code snippet shows how to use the Python package within another
project:

```python
from pylibtiff.ext.pyramidal_tiff_file import PyramidalTiffFile

image_path = "path/to/image.tif"
ptif = PyramidalTiffFile(image_path)

# iterate over all pages within the multi-scale TIFF
for page_id in range(ptif.get_page_count()):
    # read a 100x100 tile from the origin (10, 10) of the current resolution
    # as Numpy array
    y, x = 10, 10
    height, width = 100, 100
    a = ptif.crop(y, x, y+height, x+width, page_id)
```

## Tests
This repository comes with a set of automated tests that can be run by the
following command:

```
python setup.py test
```

## Documentation
Generating the documentation using
[Sphinx](https://www.sphinx-doc.org/en/master/) is a two-step process. Firstly,
you have to generate the Sphinx sources using the following command within the
"src" folder:

```
sphinx-apidoc -o ../docs ./pylibtiff
```

Still within the "src" folder, you can then generate e.g. the HTML
documentation as follows:

```
sphinx-build -M html ../docs ../docs/_build
```

## License
This project is licensed under the MIT License - see the [LICENSE](LICENSE)
file for details.
