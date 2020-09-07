#ifndef __TIFFFILE_H__
#define __TIFFFILE_H__

#include <map>
#include <math.h>
#include <fstream>
#include <stdexcept>
#include <string>

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

#include <tiffio.h>
#include "tiff_reader.h"
#include "tiff_writer.h"

#include "utils.h"

namespace py = pybind11;


// check if LibTIFF supports the BigTIFF format, e.g. if LibTIFF >= 4.0.3.
#if (defined(TIFFLIB_VERSION))
    #define STRING_HELPER(x) #x
    #define STRING(x) STRING_HELPER(x)
    #pragma message("Found LibTiff with release date " STRING(TIFFLIB_VERSION))
#endif

#if (!defined(TIFFLIB_VERSION) || TIFFLIB_VERSION < 20120922)
    #error LibTIFF v4.0.3 or higher is required!
#endif


/**
 * Class for reading and writting TIFF files.
 */
class TiffFile {
    public:
        /**
         * Structure for common Baseline and Extension TIFF Tags.
         * The full reference is available at
         * https://www.awaresystems.be/imaging/tiff/tifftags.html.
         */
        struct TiffTags {
            TiffTags() = default;               /**< Constructor to initialize a TiffTags structure. */

            // Baseline
            uint32 new_subfile_type = 0;        /**< The kind of data contained in this subfile; 0 = undefined. */
            uint32 image_width;                 /**< The number of columns in the image. */
            uint32 image_length;                /**< The number of rows of pixels in the image. */
            uint16 bits_per_sample = 1;         /**< Number of bits per component. */
            uint16 compression = 1;             /**< Compression scheme used on the image data; 1 = uncompressed. */
            uint16 photometric;                 /**< The color space of the image data. */
            uint16 samples_per_pixel = 1;       /**< The number of components per pixel. */
            uint32 rows_per_strip = 4294967295; /**< The number of rows per strip; default = 2**32 - 1. */
            uint16 min_sample_value;            /**< The minimum component value used. */
            uint16 max_sample_value;            /**< The maximum component value used. */
            uint16 planar_config = 1;           /**< How the components of each pixel are stored; 1 = chunky format. */
            // ...

            // Extension
            /**
             * Structure for the page number.
             */
            struct PageNumber {
                uint8 page_number = 0;          /**< The page number. */
                uint8 page_count = 0;           /**< The total number of pages in the document. */
            } page_number;                      /**< The page number of the page from which this image was scanned. */
            uint32 tile_width = 0;              /**< The tile width in pixels. */
            uint32 tile_length = 0;             /**< The tile length (height) in pixels. */
            uint16 sample_format = 1;           /**< Specifies how to interpret each data sample in a pixel; 1 = unsigned integer. */
            // ...
        };

    private:
        std::string file_path_;                     /**< Path to the TIFF file. */
        uint8 version_;                             /**< Version of the TIFF file (default = 42, BigTIFF = 43). */
        std::map<uint16, TiffTags> subfile_tags_;   /**< Map of all TIFF Tags per subfile. */
        uint16 subfile_count_;                      /**< Total number of subfiles. */

    public:
        /**
         * Constructor to initialize a TiffFile.
         * @param file_path Path to the TIFF file.
         * @param version Version of the TIFF file (this parameter is overwritten if the file already exists).
         */
        TiffFile(const std::string& file_path, uint8 version=42);
        
        /**
         * Get the path to the TIFF file.
         * @return Path to the TIFF file
         */
        std::string GetFilePath() { return file_path_; }
        /**
         * Get the version of the TIFF file.
         * @return Version of the TIFF file
         */
        uint8 GetVersion() { return version_; }
        /**
         * Get the total number of subfiles.
         * @return Total number of subfiles
         */
        uint16 GetSubfileCount() { return subfile_count_; }

        /**
         * Get the TIFF Tags of a subfile.
         * @param subfile_idx Index of the subfile.
         * @return TIFF Tags of the subfile
         */
        TiffTags GetSubfileTags(uint16 subfile_idx=0);

        /**
         * Get the type of a subfile.
         * @param subfile_idx Index of the subfile.
         * @return Type of the subfile
         */
        uint32 GetSubfileType(uint16 subfile_idx=0);
        /**
         * Get the image width of a subfile.
         * @param subfile_idx Index of the subfile.
         * @return Image width of the subfile
         */
        uint32 GetImageWidth(uint16 subfile_idx=0);
        /**
         * Get the image length (height) of a subfile.
         * @param subfile_idx Index of the subfile.
         * @return Image length of the subfile
         */
        uint32 GetImageLength(uint16 subfile_idx=0);
        /**
         * Get the bits per sample of a subfile.
         * @param subfile_idx Index of the subfile.
         * @return Bits per sample of the subfile
         */
        uint16 GetBitsPerSample(uint16 subfile_idx=0);
        /**
         * Get the compression of a subfile.
         * @param subfile_idx Index of the subfile.
         * @return Compression of the subfile
         */
        uint16 GetCompression(uint16 subfile_idx=0);
        /**
         * Get the photometric interpretation of a subfile.
         * @param subfile_idx Index of the subfile.
         * @return Photometric interpretation of the subfile
         */
        uint16 GetPhotometricInterpretation(uint16 subfile_idx=0);
        /**
         * Get the samples per pixel of a subfile.
         * @param subfile_idx Index of the subfile.
         * @return Samples per pixel of the subfile
         */
        uint16 GetSamplesPerPixel(uint16 subfile_idx=0);
        /**
         * Get the rows per strip of a subfile.
         * @param subfile_idx Index of the subfile.
         * @return Rows per strip of the subfile
         */
        uint32 GetRowsPerStrip(uint16 subfile_idx=0);
        /**
         * Get the minimum sample value of a subfile.
         * @param subfile_idx Index of the subfile.
         * @return Minimum sample value of the subfile
         */
        uint16 GetMinSampleValue(uint16 subfile_idx=0);
        /**
         * Get the maximum sample value of a subfile.
         * @param subfile_idx Index of the subfile.
         * @return Maximum sample value of the subfile
         */
        uint16 GetMaxSampleValue(uint16 subfile_idx=0);
        /**
         * Get the planar configuration of a subfile.
         * @param subfile_idx Index of the subfile.
         * @return Planar configuration of the subfile
         */
        uint16 GetPlanarConfiguration(uint16 subfile_idx=0);
        /**
         * Get the page number of a subfile.
         * @param subfile_idx Index of the subfile.
         * @return Page number of the subfile
         */
        uint16 GetPageNumber(uint16 subfile_idx=0);
        /**
         * Get the total number of pages in the document.
         * This number should be the same for all subfiles.
         * @param subfile_idx Index of the subfile.
         * @return Total number of pages in the document
         */
        uint16 GetPageCount(uint16 subfile_idx=0);
        /**
         * Get the tile width of a subfile.
         * @param subfile_idx Index of the subfile.
         * @return Tile width of the subfile
         */
        uint32 GetTileWidth(uint16 subfile_idx=0);
        /**
         * Get the tile length (height) of a subfile.
         * @param subfile_idx Index of the subfile.
         * @return Tile length of the subfile
         */
        uint32 GetTileLength(uint16 subfile_idx=0);
        /**
         * Get the sample format of a subfile.
         * @param subfile_idx Index of the subfile.
         * @return Sample format of the subfile
         */
        uint16 GetSampleFormat(uint16 subfile_idx=0);

        /**
         * Reads the first subfile.
         * @tparam T Data type of a subfile component (i.e. pixel).
         * @see ReadSubfile(uint16)
         * @return Image as a Numpy array
         */
        template <typename T>
        py::array_t<T> Read() {
            return ReadSubfile<T>(0);
        }

        /**
         * Reads a subfile.
         * @tparam T Data type of a subfile component (i.e. pixel).
         * @param subfile_idx Index of the subfile.
         * @return Image as a Numpy array
         */
        template <typename T>
        py::array_t<T> ReadSubfile(uint16 subfile_idx=0);

        /**
         * Reads a region from a subfile.
         * @tparam T Data type of a subfile component (i.e. pixel).
         * @param subfile_idx Index of the subfile.
         * @param x1 Upper left x-coordinate (incl).
         * @param y1 Upper left y-coordinate (incl).
         * @param x2 Lower right x-coordinate (excl).
         * @param y2 Lower right y-coordinate (excl).
         * @return Region as a Numpy array
         */
        template <typename T>
        py::array_t<T> ReadSubfileRegion(uint16 subfile_idx, uint32 x1, uint32 y1, uint32 x2, uint32 y2);

        /**
         * Writes a new subfile to the end of the TIFF file.
         * @tparam T Data type of a subfile component (i.e. pixel).
         * @param image Image data as a Numpy array.
         * @param tiff_tags TIFF Tags for the new subfile.
         * @param tiled If true, writes the image in tiles. Otherwise, writes the image in strips.
         * @see WriteSubfile(py::array_t<T>, TiffTags, bool)
         */
        template <typename T>
        void Write(py::array_t<T> image, TiffTags tiff_tags, bool tiled) {
            WriteSubfile<T>(image, tiff_tags, tiled);
        }

        /**
         * Writes a new subfile to the end of the TIFF file.
         * @tparam T Data type of a subfile component (i.e. pixel).
         * @param image Image data as a Numpy array.
         * @param tiff_tags TIFF Tags for the new subfile.
         * @param tiled If true, writes the image in tiles. Otherwise, writes the image in strips.
         */
        template <typename T>
        void WriteSubfile(py::array_t<T> image, TiffTags tiff_tags, bool tiled);

        /**
         * Writes a region into an existing subfile.
         * @note libtiff does no support altering the contents of a TIFF file.
         * Which is true for compressed tiles/scanlines where a source
         * scanline/tile uses less disk space than required by the modified
         * scanline/tile.
         * However, the operation could be implemented for uncompressed subfiles.
         * @tparam T Data type of a subfile component (i.e. pixel).
         * @param image Region data as a Numpy array.
         * @param subfile_idx Index of the subfile.
         * @param x1 Upper left x-coordinate (incl).
         * @param y1 Upper left y-coordinate (incl).
         * @param x2 Lower right x-coordinate (excl).
         * @param y2 Lower right y-coordinate (excl).
         */
        template <typename T>
        void WriteSubfileRegion(
            py::array_t<T> image, uint16 subfile_idx,
            uint32 x1, uint32 y1, uint32 x2, uint32 y2
        );

        /**
         * Writes a multi-scale subfile into a TIFF file.
         * @note Existing data is overwritten!
         * @tparam T Data type of a subfile component (i.e. pixel).
         * @param image Baseline image data as a Numpy array.
         * @param tiff_tags TIFF Tags for the new subfile.
         */
        template <typename T>
        void WriteMultiscaleSubfile(py::array_t<T> image, TiffTags tiff_tags);
};


PYBIND11_MODULE(tiff_file, m) {
    py::class_<TiffFile> cls_tiff_file(m, "TiffFile");
    py::class_<TiffFile::TiffTags> cls_tiff_tags(cls_tiff_file, "TiffTags");
    py::class_<TiffFile::TiffTags::PageNumber> cls_page_number(cls_tiff_tags, "PageNumber");

    cls_tiff_tags
        .def(py::init<>());

    cls_tiff_tags
        .def_readwrite("new_subfile_type", &TiffFile::TiffTags::new_subfile_type)
        .def_readwrite("image_width", &TiffFile::TiffTags::image_width)
        .def_readwrite("image_length", &TiffFile::TiffTags::image_length)
        .def_readwrite("bits_per_sample", &TiffFile::TiffTags::bits_per_sample)
        .def_readwrite("compression", &TiffFile::TiffTags::compression)
        .def_readwrite("photometric", &TiffFile::TiffTags::photometric)
        .def_readwrite("samples_per_pixel", &TiffFile::TiffTags::samples_per_pixel)
        .def_readwrite("rows_per_strip", &TiffFile::TiffTags::rows_per_strip)
        .def_readwrite("min_sample_value", &TiffFile::TiffTags::min_sample_value)
        .def_readwrite("max_sample_value", &TiffFile::TiffTags::max_sample_value)
        .def_readwrite("planar_config", &TiffFile::TiffTags::planar_config)
        .def_readwrite("page_number", &TiffFile::TiffTags::page_number)
        .def_readwrite("tile_width", &TiffFile::TiffTags::tile_width)
        .def_readwrite("tile_length", &TiffFile::TiffTags::tile_length)
        .def_readwrite("sample_format", &TiffFile::TiffTags::sample_format);

    cls_page_number
        .def(py::init<>());

    cls_page_number
        .def_readwrite("page_number", &TiffFile::TiffTags::PageNumber::page_number)
        .def_readwrite("page_count", &TiffFile::TiffTags::PageNumber::page_count);

    cls_tiff_file
        .def(py::init<const std::string&, uint8>(), py::arg("file_path"), py::arg("version") = 42);

    cls_tiff_file
        .def("get_file_path", &TiffFile::GetFilePath)
        .def("get_version", &TiffFile::GetVersion)
        .def("get_subfile_count", &TiffFile::GetSubfileCount);

    auto read_8 = static_cast<py::array_t<uint8> (TiffFile::*)()>(&TiffFile::Read);
    auto read_16 = static_cast<py::array_t<uint16> (TiffFile::*)()>(&TiffFile::Read);

    auto read_subfile_8 = static_cast<py::array_t<uint8> (TiffFile::*)(uint16)>(&TiffFile::ReadSubfile);
    auto read_subfile_16 = static_cast<py::array_t<uint16> (TiffFile::*)(uint16)>(&TiffFile::ReadSubfile);

    auto read_subfile_region_8 = static_cast<py::array_t<uint8> (TiffFile::*)(uint16, uint32, uint32, uint32, uint32)>(&TiffFile::ReadSubfileRegion);
    auto read_subfile_region_16 = static_cast<py::array_t<uint16> (TiffFile::*)(uint16, uint32, uint32, uint32, uint32)>(&TiffFile::ReadSubfileRegion);

    auto write_8 = static_cast<void (TiffFile::*)(py::array_t<uint8>, TiffFile::TiffTags, bool)>(&TiffFile::Write);
    auto write_16 = static_cast<void (TiffFile::*)(py::array_t<uint16>, TiffFile::TiffTags, bool)>(&TiffFile::Write);

    auto write_subfile_8 = static_cast<void (TiffFile::*)(py::array_t<uint8>, TiffFile::TiffTags, bool)>(&TiffFile::WriteSubfile);
    auto write_subfile_16 = static_cast<void (TiffFile::*)(py::array_t<uint16>, TiffFile::TiffTags, bool)>(&TiffFile::WriteSubfile);

    auto write_subfile_region_8 = static_cast<void (TiffFile::*)(py::array_t<uint8>, uint16, uint32, uint32, uint32, uint32)>(&TiffFile::WriteSubfileRegion);
    auto write_subfile_region_16 = static_cast<void (TiffFile::*)(py::array_t<uint16>, uint16, uint32, uint32, uint32, uint32)>(&TiffFile::WriteSubfileRegion);

    auto write_multiscale_subfile_8 = static_cast<void (TiffFile::*)(py::array_t<uint8>, TiffFile::TiffTags)>(&TiffFile::WriteMultiscaleSubfile);
    auto write_multiscale_subfile_16 = static_cast<void (TiffFile::*)(py::array_t<uint16>, TiffFile::TiffTags)>(&TiffFile::WriteMultiscaleSubfile);

    cls_tiff_file
        .def("get_subfile_tags", &TiffFile::GetSubfileTags)
        .def("get_subfile_type", &TiffFile::GetSubfileType)
        .def("get_image_width", &TiffFile::GetImageWidth)
        .def("get_image_length", &TiffFile::GetImageLength)
        .def("get_bits_per_sample", &TiffFile::GetBitsPerSample)
        .def("get_compression", &TiffFile::GetCompression)
        .def("get_photometric_interpretation", &TiffFile::GetPhotometricInterpretation)
        .def("get_samples_per_pixel", &TiffFile::GetSamplesPerPixel)
        .def("get_rows_per_strip", &TiffFile::GetRowsPerStrip)
        .def("get_min_sample_value", &TiffFile::GetMinSampleValue)
        .def("get_max_sample_value", &TiffFile::GetMaxSampleValue)
        .def("get_planar_configuration", &TiffFile::GetPlanarConfiguration)
        .def("get_page_number", &TiffFile::GetPageNumber)
        .def("get_page_count", &TiffFile::GetPageCount)
        .def("get_tile_width", &TiffFile::GetTileWidth)
        .def("get_tile_length", &TiffFile::GetTileLength)
        .def("get_sample_format", &TiffFile::GetSampleFormat)
        .def("read_8", read_8)
        .def("read_16", read_16)
        .def("read_subfile_8", read_subfile_8)
        .def("read_subfile_16", read_subfile_16)
        .def("read_subfile_region_8", read_subfile_region_8)
        .def("read_subfile_region_16", read_subfile_region_16)
        .def("write_8", write_8)
        .def("write_16", write_16)
        .def("write_subfile_8", write_subfile_8)
        .def("write_subfile_16", write_subfile_16)
        .def("write_subfile_region_8", write_subfile_region_8)
        .def("write_subfile_region_16", write_subfile_region_16)
        .def("write_multiscale_subfile_8", write_multiscale_subfile_8)
        .def("write_multiscale_subfile_16", write_multiscale_subfile_16);
}

#endif /* __TIFFFILE_H__ */
