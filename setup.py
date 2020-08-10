import platform
from setuptools import setup, find_packages, Extension
import sys


if platform.system() == "Windows":
    library_dirs = [
        'ext/libtiff_4_0_7/wn_lib',
        'ext/jpeg-9d/wn_lib',
        'ext/zlib-1.2.11/wn_lib'
    ]
    include_dirs = [
        'ext/libtiff_4_0_7/include',
        'ext/jpeg-9d/include',
        'ext/zlib-1.2.11/include'
    ]
    extra_link_args=['-static']
else:
    library_dirs = []
    include_dirs = []
    extra_link_args=[]


class LazyPyBind11IncludeDirWrapper(object):
    def __init__(self, user=False):
        self.user = user

    def __str__(self):
        import pybind11
        return pybind11.get_include(self.user)


tiff_extention_module = Extension(
    'pylibtiff.ext.pyramidal_tiff_file',
    library_dirs=library_dirs,
    libraries=['tiff', 'jpeg', 'z'],
    sources=['src/ext/pyramidal_tiff_file.cpp'],
    include_dirs=[
        LazyPyBind11IncludeDirWrapper(),
        LazyPyBind11IncludeDirWrapper(user=True),
        *include_dirs
    ],
    extra_compile_args=['-std=c++11'],
    extra_link_args=extra_link_args,
    language='c++',
)


setup(
    name='pylibtiff',
    version='0.1.0',
    author='D. Vischi',
    author_email='dario.vischi@fmi.ch',
    classifiers=[
        "Programming Language :: Python :: 3.6",
        "Programming Language :: Python :: 3.8",
    ],
    packages=find_packages('src', exclude=['tests']),
    package_dir={'pylibtiff': './src/pylibtiff'},
    # read additional package data from MANIFEST.in
    # not compatible with package_data
    include_package_data=False,
    # install files which are within the package,
    # but e.g. not within a module (a folder including an __init__.py file)
    package_data={
        'pylibtiff': []
    },
    # scripts=['bin/script.py'],
    url='https://github.com/fmi-basel/pylibtiff/',
    license='LICENSE.txt',
    description='libtiff wrapper for Python.',
    long_description=open('README.md').read(),
    ext_modules=[tiff_extention_module],
    install_requires=[
        "numpy >= 1.14.0",
        "pybind11 >= 2.2.3",
    ],
    test_suite='tests',
)