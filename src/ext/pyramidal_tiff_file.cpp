//#include <cstdlib>
//#include <cstring>
#include <map>
#include <math.h>
#include <iostream>
#include <string> 

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

#include <tiffio.h>

namespace py = pybind11;

#ifdef DEBUG
#define DEBUG_PRINTF(fmt, args...) printf("DEBUG: %s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, ##args)
#else
#define DEBUG_PRINTF(fmt, args...)
#endif

const int& min(const int& a, const int& b) {
    return (b < a) ? b : a;
}
const int& max(const int& a, const int& b) {
    return (a < b) ? b : a;
}


class PyramidalTiffFile {
    std::string filePath;
    
    struct BaselineTiffTags {
        uint16 photometric;
        uint16 compress;
        uint16 planarConfig;
        uint16 bitsPerSample;
        uint32 imageWidth, imageLength;
        uint32 tileWidth, tileLength;
        uint16 sampleFormat;
        uint16 samplesPerPixel;
        // ...
    };
    
    //TiffTags* pageTags = nullptr;
    
    std::map<uint16, BaselineTiffTags> pageTags;
    
    /*
    uint16 photometric;
    uint16 compress;
    uint16 planarConfig;
    uint16 bitsPerSample;
    uint32 imageWidth, imageLength;
    uint32 tileWidth, tileLength;
    uint16 sampleFormat;
    uint16 samplesPerPixel;
    */
    
    uint16 pageCount;
    
    public:
        enum MultiPageStrategy {
          FITPAGETILE = 0
        };
        
        PyramidalTiffFile(const std::string &filePath) : filePath(filePath) {
            TIFF *tiff = TIFFOpen(filePath.c_str(), "r");
            if (tiff == nullptr)
                throw std::runtime_error("Could not read file '" + filePath + "'!");
            
            //uint16 pageNumber[2];
            //TIFFGetField(tiff, TIFFTAG_PAGENUMBER, &pageNumber[0], &pageNumber[1]);
            
            //printf("%d, %d\n", pageNumber[0], pageNumber[1]);
            
            uint16 pageNumber = 0;
            do {
                BaselineTiffTags tiffTags;
                
                TIFFGetField(tiff, TIFFTAG_PHOTOMETRIC, &tiffTags.photometric);
                TIFFGetField(tiff, TIFFTAG_COMPRESSION, &tiffTags.compress);
                TIFFGetField(tiff, TIFFTAG_PLANARCONFIG, &tiffTags.planarConfig);
                TIFFGetField(tiff, TIFFTAG_BITSPERSAMPLE, &tiffTags.bitsPerSample);
                TIFFGetField(tiff, TIFFTAG_IMAGEWIDTH, &tiffTags.imageWidth);
                TIFFGetField(tiff, TIFFTAG_IMAGELENGTH, &tiffTags.imageLength);
                TIFFGetField(tiff, TIFFTAG_TILEWIDTH, &tiffTags.tileWidth);
                TIFFGetField(tiff, TIFFTAG_TILELENGTH, &tiffTags.tileLength);
                TIFFGetField(tiff, TIFFTAG_SAMPLEFORMAT, &tiffTags.sampleFormat);
                TIFFGetField(tiff, TIFFTAG_SAMPLESPERPIXEL, &tiffTags.samplesPerPixel);
                
                pageTags[pageNumber] = tiffTags;
                
                pageNumber += 1;
            } while (TIFFReadDirectory(tiff) > 0);
            
            /*
            pageCount = pageNumber[1];
            if (pageCount == 0) {  // total no of pages is not available
                pageCount = 1;
                while (TIFFReadDirectory(tiff) > 0)
                    pageCount += 1;
            }
            */
            pageCount = pageNumber;
            
            TIFFClose(tiff);
        }
        /*
        ~PyramidalTiffFile() {
            //if (pageTags) {
            //    
            //}
        }
        */
        
        // default: request tags from highest resolution page (idx = -1)
        uint32 get_image_width(int16 page=-1) {
            page = (pageCount + (page % pageCount)) % pageCount;
            return pageTags[page].imageWidth;
        }
        uint32 get_image_length(int16 page=-1) {
            page = (pageCount + (page % pageCount)) % pageCount;
            return pageTags[page].imageLength;
        }
        uint32 get_tile_width(int16 page=-1) {
            page = (pageCount + (page % pageCount)) % pageCount;
            return pageTags[page].tileWidth;
        }
        uint32 get_tile_length(int16 page=-1) {
            page = (pageCount + (page % pageCount)) % pageCount;
            return pageTags[page].tileLength;
        }
        
        uint16 get_page_count() { return pageCount; }
        
        /**
         * crop tiff image.
         * @param cropY1 upper left y-coordinate (incl).
         * @param cropX1 upper left x-coordinate (incl).
         * @param cropY2 lower right y-coordinate (excl).
         * @param cropX2 lower right x-coordinate (excl).
         * @param strategy defines which page to crop from.
         * @return uint8 numpy array
         */
        py::array_t<uint8> multi_page_crop(
            int32 cropY1, int32 cropX1, int32 cropY2, int32 cropX2, MultiPageStrategy strategy=MultiPageStrategy::FITPAGETILE
        ) {
            if (cropY1 >= cropY2 || cropX1 >= cropX2)
                throw std::runtime_error("Invalid crop dimensions defined!");
            
            /*
            TIFF *tiff = TIFFOpen(filePath.c_str(), "r");
            if (tiff == nullptr)
                throw std::runtime_error("Could not read file '" + filePath + "'!");
            */
            
            /*
            uint16 pageNumber[2];
            TIFFGetField(tiff, TIFFTAG_PAGENUMBER, &pageNumber[0], &pageNumber[1]);
            
            noPages = pageNumber[1];
            if (noPages == 0) {  // total no of pages is not available
                noPages += 1;
                if (~TIFFReadDirectory(tiff))
                    break;
            }
            */
            
            //float32 cropRatio = (cropY2 - cropY1) / imageLength * (cropX2 - cropX1) / imageWidth;
            
            uint8 maxResPage = pageCount - 1;  // -1: ignores the ROI page
            uint8 cropPage = maxResPage;
            
            switch(strategy) {
                case MultiPageStrategy::FITPAGETILE:
                {
                    uint32 majorCropSize = max(cropY2 - cropY1, cropX2 - cropX1);
                    //uint8 cropPage = 0;
                    /*
                    while (cropPage+1 < pageCount) {
                        if (majorCropSize <= get_tile_width(cropPage) << cropPage)
                            break;
                        else
                            cropPage += 1;
                    }
                    */
                    while (cropPage > 0) {
                        if (majorCropSize <= get_tile_width(cropPage) << (maxResPage - cropPage))
                            break;
                        else
                            cropPage -= 1;
                    }
                    
                    /*
                    return crop(
                        cropY1 >> cropPage, cropX1 >> cropPage,
                        cropY2 >> cropPage, cropX2 >> cropPage,
                        cropPage
                    );
                    */
                    
                    cropY1 = cropY1 >> (maxResPage - cropPage);
                    cropX1 = cropX1 >> (maxResPage - cropPage);
                    cropY2 = cropY2 >> (maxResPage - cropPage);
                    cropX2 = cropX2 >> (maxResPage - cropPage);
                    
                    break;
                }
                default:
                {
                    throw std::runtime_error("Invalid strategy parameter defined!");
                }
            }
            
            DEBUG_PRINTF("%d: %d, %d\n", cropPage, cropY1, cropY2);
            
            return crop(cropY1, cropX1, cropY2, cropX2, cropPage);
        }
        
        /**
         * crop tiff image.
         * @param cropY1 upper left y-coordinate (incl).
         * @param cropX1 upper left x-coordinate (incl).
         * @param cropY2 lower right y-coordinate (excl).
         * @param cropX2 lower right x-coordinate (excl).
         * @param page page id for source image.
         * @return uint8 numpy array
         */
        py::array_t<uint8> crop(int32 cropY1, int32 cropX1, int32 cropY2, int32 cropX2, uint8 page=0) {
            if (cropY1 >= cropY2 || cropX1 >= cropX2)
                throw std::runtime_error("Invalid crop dimensions defined!");
            
            TIFF *tiff = TIFFOpen(filePath.c_str(), "r");
            if (tiff == nullptr)
                throw std::runtime_error("Could not read file '" + filePath + "'!");
            
            for (uint8 pageItr = page; pageItr > 0; pageItr -= 1) {
                if (TIFFReadDirectory(tiff) == 0)
                    throw std::runtime_error("Page " + std::to_string(page) + " was not found!");
            }
            
            uint32 pageLength = get_image_length(page);
            uint32 pageWidth = get_image_width(page);
            //uint32 pageTileWidth = tileWidth >> page;
            //uint32 pageTileLength = tileLength >> page;
            
            uint32 pageTileLength = get_tile_length(page);
            uint32 pageTileWidth = get_tile_width(page);

            DEBUG_PRINTF(
                "[%d] pageLength/Width tileLength/Width: %d/%d %d/%d\n",
                page, pageLength, pageWidth, pageTileLength, pageTileWidth
            );

            //uint32 noTilesY = pageLength / pageTileLength;
            //uint32 noTilesX = pageWidth / pageTileWidth;
            uint32 noTilesY = (pageLength + pageTileLength - 1) / pageTileLength;
            uint32 noTilesX = (pageWidth + pageTileWidth - 1) / pageTileWidth;

            DEBUG_PRINTF("noTilesY/noTilesX: %d, %d\n", noTilesX, noTilesY);
            
            uint32 cropWidth = cropX2 - cropX1;
            uint32 cropLength = cropY2 - cropY1;
            
            DEBUG_PRINTF("cropLength/cropWidth: %d, %d\n", cropLength, cropWidth);
            
            py::array_t<uint8> cropInfo = py::array(
                py::buffer_info(
                    nullptr,            /* Pointer to data (nullptr -> ask NumPy to allocate!) */
                    sizeof(uint8),      /* Size of one item */
                    py::format_descriptor<uint8>::value, /* Buffer format */
                    2,                  /* How many dimensions? */
                    { cropLength, cropWidth },  /* Number of elements for each dimension */
                    { sizeof(uint8) * cropWidth, sizeof(uint8) }  /* Strides for each dimension */
                )
            );
            uint8* p_crop = static_cast<uint8*>(cropInfo.request().ptr);
            uint8* p_tile = (uint8*) _TIFFmalloc(TIFFTileSize(tiff));
            
            // initialize the crop with a default value (e.g. black or transparent)
            std::memset(p_crop, 0, cropWidth * cropLength * sizeof(uint8));
            
            int32 tileNoY1 = floor(float(cropY1) / pageTileLength);  // incl.
            int32 tileNoX1 = floor(float(cropX1) / pageTileWidth);  // incl.
            int32 tileNoY2 = floor(float(cropY2 -1) / pageTileLength) + 1;  // excl.
            int32 tileNoX2 = floor(float(cropX2 -1) / pageTileWidth) + 1;  // excl.
            
            DEBUG_PRINTF("tileNoY/tileNoX: (%d, %d), (%d, %d)\n", tileNoY1, tileNoX1, tileNoY2, tileNoX2);
            
            // re-compute tiles to read, ignoring areas outside of the image
            tileNoY1 = max(0, min(noTilesY, tileNoY1));
            tileNoX1 = max(0, min(noTilesX, tileNoX1));
            tileNoY2 = max(0, min(noTilesY, tileNoY2));
            tileNoX2 = max(0, min(noTilesX, tileNoX2));
            
            DEBUG_PRINTF("tileNoY/tileNoX (map): (%d, %d), (%d, %d)\n", tileNoY1, tileNoX1, tileNoY2, tileNoX2);
            
            for (uint32 tileNoY = tileNoY1; tileNoY < tileNoY2; tileNoY += 1) {
                for (uint32 tileNoX = tileNoX1; tileNoX < tileNoX2; tileNoX += 1) {
                    uint32 tileNo = tileNoY * noTilesX + tileNoX;
                    
                    TIFFReadEncodedTile(tiff, tileNo, p_tile, (tsize_t) -1);
                    
                    DEBUG_PRINTF("tileNo: %d\n", tileNo);
                    
                    uint32 tileY1 = tileNoY * pageTileLength;
                    uint32 tileX1 = tileNoX * pageTileWidth;
                    uint32 tileY2 = (tileNoY+1) * pageTileLength;
                    uint32 tileX2 = (tileNoX+1) * pageTileWidth;
                    
                    DEBUG_PRINTF("tileY/tileX: (%d, %d), (%d, %d)\n", tileY1, tileX1, tileY2, tileX2);
                    
                    uint32 tileRoiY1 = max(0, cropY1 - tileY1);
                    uint32 tileRoiX1 = max(0, cropX1 - tileX1);
                    //uint32 tileRoiY2 = pageTileLength - max(0, tileY2 - cropY2);
                    //uint32 tileRoiX2 = pageTileWidth - max(0, tileX2 - cropX2);
                    uint32 tileRoiY2 = pageTileLength - max(max(0, tileY2 - pageLength), tileY2 - cropY2);
                    uint32 tileRoiX2 = pageTileWidth - max(max(0, tileX2 - pageWidth), tileX2 - cropX2);
                    
                    DEBUG_PRINTF("tileRoiY/tileRoiX: (%d, %d), (%d, %d)\n", tileRoiY1, tileRoiX1, tileRoiY2, tileRoiX2);
                    
                    uint32 cropRoiY1 = max(0, tileY1 - cropY1);
                    uint32 cropRoiX1 = max(0, tileX1 - cropX1);
                    //uint32 cropRoiY2 = cropLength - max(0, cropY2 - tileY2);
                    //uint32 cropRoiX2 = cropWidth - max(0, cropX2 - tileX2);
                    uint32 cropRoiY2 = cropLength - max(max(0, tileY2 - pageLength), cropY2 - tileY2);
                    uint32 cropRoiX2 = cropWidth - max(max(0, tileX2 - pageWidth), cropX2 - tileX2);
                    
                    DEBUG_PRINTF("cropRoiY/cropRoiX: (%d, %d), (%d, %d)\n", cropRoiY1, cropRoiX1, cropRoiY2, cropRoiX2);
                    
                    while (tileRoiY1 < tileRoiY2) {
                        // copy scanline
                        std::memcpy(
                            &p_crop[cropRoiY1 * cropWidth + cropRoiX1],
                            &p_tile[tileRoiY1 * pageTileWidth + tileRoiX1],
                            sizeof(uint8) * (tileRoiX2 - tileRoiX1)
                        );
                        tileRoiY1 += 1;
                        cropRoiY1 += 1;
                    }
                }
            }
            
            _TIFFfree(p_tile);
            TIFFClose(tiff);
            
            return cropInfo;
        }
};


PYBIND11_MODULE(pyramidal_tiff_file, m) {
    py::class_<PyramidalTiffFile> pyramidalTiffFile(m, "PyramidalTiffFile");
    
    py::enum_<PyramidalTiffFile::MultiPageStrategy> multiPageStrategy(pyramidalTiffFile, "MultiPageStrategy");
    multiPageStrategy.value("FITPAGETILE", PyramidalTiffFile::MultiPageStrategy::FITPAGETILE)
        .export_values();
    
    pyramidalTiffFile.def(py::init<const std::string &>())
        .def("get_image_width", &PyramidalTiffFile::get_image_width)
        .def("get_image_length", &PyramidalTiffFile::get_image_length)
        .def("get_tile_width", &PyramidalTiffFile::get_tile_width)
        .def("get_tile_length", &PyramidalTiffFile::get_tile_length)
        .def("get_page_count", &PyramidalTiffFile::get_page_count)
        .def(
            "multi_page_crop", &PyramidalTiffFile::multi_page_crop,
            py::arg("cropY1"), py::arg("cropX1"), py::arg("cropY2"), py::arg("cropX2"),
            py::arg("strategy") = PyramidalTiffFile::MultiPageStrategy::FITPAGETILE
        )
        .def("crop", &PyramidalTiffFile::crop);
        //.def("read_mask", &PyramidalTiffFile::read_mask)
        //.def("write", &PyramidalTiffFile::write);
    
    /*
    py::enum_<PyramidalTiffFile::MultiPageStrategy>(pyramidalTiffFile, "MultiPageStrategy")
        .value("FITPAGETILE", PyramidalTiffFile::MultiPageStrategy::FITPAGETILE)
        .export_values();
    */
}