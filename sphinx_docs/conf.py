# -- General configuration ------------------------------------------------

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = [
    'sphinx.ext.todo', 'sphinx.ext.autodoc', 'sphinx.ext.intersphinx',
    'sphinx.ext.mathjax', 'sphinx.ext.viewcode', 'sphinx.ext.graphviz',
    #'sphinx.ext.autosummary',
]

# Add any paths that contain templates here, relative to this directory.
# templates_path = ['_templates']

# The suffix(es) of source filenames.
# You can specify multiple suffix as a list of string:
#
# source_suffix = ['.rst', '.md']
source_suffix = ['.rst', '.md']

# The master toctree document.
master_doc = 'index'

# General information about the project.
project = u'Liberali Group - PyLibtiff'
copyright = (
    u'(c) 2020 Friedrich Miescher Institute for Biomedical Research'
)
author = u'Dario Vischi'

# The version info for the project you're documenting, acts as replacement for
# |version| and |release|, also used in various other places throughout the
# built documents.
#
# The short X.Y version.
version = u'1.0'
# The full version, including alpha/beta/rc tags.
release = u'1.0.0'

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# These patterns also affect html_static_path and html_extra_path
exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store']

# The name of the Pygments (syntax highlighting) style to use.
pygments_style = 'sphinx'

# If true, `todo` and `todoList` produce output, else they produce nothing.
todo_include_todos = False


# -- Options for HTML output ----------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
html_theme = 'nature'

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
# html_static_path = ['_static']

# Customize the HTML side menu
html_sidebars = {
    '**': [
        'globaltoc.html',
        #'relations.html',
        'sourcelink.html',
        'searchbox.html'
    ]
}

# -- Project configuration -----------------------------------------------------

import sys, os
sys.path.insert(0, os.path.join('.', 'src'))

extensions.append('sphinx.ext.githubpages')
extensions.append('recommonmark')

autosummary_generate = False

import json
with open('../info.json', 'r') as fh:
    info = json.load(fh)

release = info['version']
version = release[0:3]

# Apply substitutions
from jinja2 import Template
for templ in ['pylibtiff/user_guide/license.j2']:
    with open(templ, 'r') as th:
        with open(templ.replace('.j2', '.md'), 'w') as fh:
            fh.write(Template(th.read()).render(**info))
