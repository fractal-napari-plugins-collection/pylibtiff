## Installation 

To avoid conflicts with other packages, it is recommended to install PyLibTIFF
within a virtual environment. Please refer to "The Python Tutorial"
for further information (https://docs.python.org/3/tutorial/venv.html).

### Installation from Wheel package

PyLibTIFF can be installed using a pre-compiled Wheel package available at
https://github.com/fmi-basel/pylibtiff/releases.
After downloading a wheel matching your operating system, your CPU architecture
and Python version you can install the package via pip. E.g., the following
command installs PyLibTIFF v1.0.0 on a 64bit Windows with Python 3.6:

```text
pip install pylibtiff-1.0.0-cp36-cp36m-win_amd64.whl
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

```text
pip install .
```

...or, compiled as a Wheel package.

```text
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

```text
pip install . --global-option=build_ext --global-option="-I/path/to/include-dir" --global-option="-L/path/to/library-dir"
```
