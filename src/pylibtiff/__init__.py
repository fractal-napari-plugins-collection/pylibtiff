from pylibtiff.ext.pyramidal_tiff_file import PyramidalTiffFile


class TiffPage:
    page_no = None
    image_width = None
    image_height = None
    tile_width = None
    tile_height = None
    
    def __init__(
        self, page_no, image_width, image_height, tile_width, tile_height,
        crop_func
    ):
        self.page_no = page_no
        self.image_width = image_width
        self.image_height = image_height
        self.tile_width = tile_width
        self.tile_height = tile_height
        self.crop_func = crop_func
    
    def crop(self, y1, x1, y2, x2):
        return self.crop_func(y1, x1, y2, x2, self.page_no)

class PyramidalTiffImage:
    pyramidal_tiff_file = None
    pages = None
    
    def __init__(self, image_path):
        self.pyramidal_tiff_file = PyramidalTiffFile(image_path)
        self.pages = []
        
        for page_no in range(self.pyramidal_tiff_file.get_page_count()):
            self.pages.append(
                TiffPage(
                    page_no,
                    self.pyramidal_tiff_file.get_image_width(page_no),
                    self.pyramidal_tiff_file.get_image_length(page_no),
                    self.pyramidal_tiff_file.get_tile_width(page_no),
                    self.pyramidal_tiff_file.get_tile_length(page_no),
                    self.pyramidal_tiff_file.crop
                )
            )
    
    @property
    def image_width(self):
        return self.pages[-2].image_width
    
    @property
    def image_height(self):
        return self.pages[-2].image_height
    
    @property
    def tile_width(self):
        return self.pages[-2].tile_width
    
    @property
    def tile_height(self):
        return self.pages[-2].tile_height
    
    def multi_page_crop(self, y1, x1, y2, x2, strategy=PyramidalTiffFile.MultiPageStrategy.FITPAGETILE):
        return self.pyramidal_tiff_file.multi_page_crop(y1, x1, y2, x2, strategy)
