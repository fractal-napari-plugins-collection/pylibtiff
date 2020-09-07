## Documentation

PyLibTIFF comes with two documentations for Python
(using [Sphinx](https://www.sphinx-doc.org/en/master/)) and C++
(using [Doxygen](https://www.doxygen.nl/index.html)).

### Sphinx

Generating the main documentation is a two-step process. Firstly, you have to
generate the source files for the API documentation:

```text
sphinx-apidoc --module-first -o ./sphinx_docs/pylibtiff/api_reference ./src/pylibtiff ./src/pylibtiff/ext
```

In a second step, you can generate the full documentation e.g. as a series of
HTML files:

```text
sphinx-build -b html ./sphinx_docs ./_sbuild
```

### Doxygen

A secondary documentation can be generated for the C++ code:

doxygen ./doxygen_docs/doxyfile.cfg

This will generate HTML and LaTeX files within the directory "./_dbuild".

> Note: The C++ documentation only covers the API reference and does not
        include the user guide.
