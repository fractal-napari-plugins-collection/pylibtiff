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
