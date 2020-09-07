#ifndef __TIFFWRITER_H__
#define __TIFFWRITER_H__

#include <cstring>
#include <iostream>
#include <string>

#include <tiffio.h>

#include "utils.h"


/**
 * Internal class for writing TIFF files.
 */
class TiffWriter {
    private:
        static char errorBuffer_[1024]; /**< Buffer containing diagnostic messages from libtiff. */
        /**
         * Error handler routine for libtiff.
         * @param module Module in which an error is detected.
         * @param format printf(3S) format string.
         * @param args arguments for the format string.
         */
        static void ErrorHandler(const char* module, const char* format, va_list args);

    public:
        /**
         * Writes a subfile by scanlines.
         * @tparam T Data type of the image buffer.
         * @param tiff TIFF handle from libtiff.
         * @param subfile_idx Index of the subfile.
         * @param arr_ptr image buffer where to read from.
         */
        template <typename T>
        static void WriteSubfileByScanline(TIFF* tiff, uint16 subfile_idx, T* arr_ptr);

        /**
         * Writes a region of a subfile by scanlines.
         * @note This operation is not supported by libtiff!
         * @tparam T Data type of the region buffer.
         * @param tiff TIFF handle from libtiff.
         * @param subfile_idx Index of the subfile.
         * @param arr_ptr region buffer where to read from.
         * @param x1 Upper left x-coordinate (incl).
         * @param y1 Upper left y-coordinate (incl).
         * @param x2 Lower right x-coordinate (excl).
         * @param y2 Lower right y-coordinate (excl).
         */
        template <typename T>
        static void WriteSubfileRegionByScanline(
            TIFF* tiff, uint16 subfile_idx, T* arr_ptr,
            uint32 x1, uint32 y1, uint32 x2, uint32 y2
        ) {
            throw std::runtime_error("Altering the content of a TIFF file is not supported!");
        }

        /**
         * Overwrites a region of a subfile by scanlines
         * and stores the resulting image into a new TIFF file.
         * @note This operation is for testing purpose only!
         * @tparam T Data type of the region buffer.
         * @param in_file_path Path to the input TIFF file.
         * @param out_file_path Path to the output TIFF file.
         * @param version Version of the TIFF files (default = 42, BigTIFF = 43).
         * @param subfile_idx Index of the subfile.
         * @param arr_ptr region buffer where to read from.
         * @param x1 Upper left x-coordinate (incl).
         * @param y1 Upper left y-coordinate (incl).
         * @param x2 Lower right x-coordinate (excl).
         * @param y2 Lower right y-coordinate (excl).
         */
        template <typename T>
        static void _WriteSubfileRegionByScanline(
            std::string in_file_path, std::string out_file_path,
            uint8 version, uint16 subfile_idx, T* arr_ptr,
            uint32 x1, uint32 y1, uint32 x2, uint32 y2
        );

        /**
         * Writes a subfile by strips.
         * @note This operation is not yet supported!
         * @tparam T Data type of the image buffer.
         * @param tiff TIFF handle from libtiff.
         * @param subfile_idx Index of the subfile.
         * @param arr_ptr image buffer where to read from.
         */
        template <typename T>
        static void WriteSubfileByStrip(TIFF* tiff, uint16 subfile_idx, T* arr_ptr) {
            throw std::runtime_error("Writing a TIFF file by strips is not supported!");
        }

        /**
         * Writes a region of a subfile by strips.
         * @note This operation is not supported by libtiff!
         * @tparam T Data type of the region buffer.
         * @param tiff TIFF handle from libtiff.
         * @param subfile_idx Index of the subfile.
         * @param arr_ptr region buffer where to read from.
         * @param x1 Upper left x-coordinate (incl).
         * @param y1 Upper left y-coordinate (incl).
         * @param x2 Lower right x-coordinate (excl).
         * @param y2 Lower right y-coordinate (excl).
         */
        template <typename T>
        static void WriteSubfileRegionByStrip(
            TIFF* tiff, uint16 subfile_idx, T* arr_ptr,
            uint32 x1, uint32 y1, uint32 x2, uint32 y2
        ) {
            throw std::runtime_error("Writing a TIFF file by strips is not supported!");
        }

        /**
         * Writes a subfile by tiles.
         * @tparam T Data type of the image buffer.
         * @param tiff TIFF handle from libtiff.
         * @param subfile_idx Index of the subfile.
         * @param arr_ptr image buffer where to read from.
         */
        template <typename T>
        static void WriteSubfileByTile(TIFF* tiff, uint16 subfile_idx, T* arr_ptr);

        /**
         * Writes a region of a subfile by tiles.
         * @note This operation is not supported by libtiff!
         * @tparam T Data type of the region buffer.
         * @param tiff TIFF handle from libtiff.
         * @param subfile_idx Index of the subfile.
         * @param arr_ptr region buffer where to read from.
         * @param x1 Upper left x-coordinate (incl).
         * @param y1 Upper left y-coordinate (incl).
         * @param x2 Lower right x-coordinate (excl).
         * @param y2 Lower right y-coordinate (excl).
         */
        template <typename T>
        static void WriteSubfileRegionByTile(
            TIFF* tiff, uint16 subfile_idx, T* arr_ptr,
            uint32 x1, uint32 y1, uint32 x2, uint32 y2
        ) {
            throw std::runtime_error("Altering the content of a TIFF file is not supported!");
        }

        /**
         * Overwrites a region of a subfile by tiles
         * and stores the resulting image into a new TIFF file.
         * @note This operation is for testing purpose only!
         * @tparam T Data type of the region buffer.
         * @param in_file_path Path to the input TIFF file.
         * @param out_file_path Path to the output TIFF file.
         * @param version Version of the TIFF files (default = 42, BigTIFF = 43).
         * @param subfile_idx Index of the subfile.
         * @param arr_ptr region buffer where to read from.
         * @param x1 Upper left x-coordinate (incl).
         * @param y1 Upper left y-coordinate (incl).
         * @param x2 Lower right x-coordinate (excl).
         * @param y2 Lower right y-coordinate (excl).
         */
        template <typename T>
        static void _WriteSubfileRegionByTile(
            std::string in_file_path, std::string out_file_path,
            uint8 version, uint16 subfile_idx, T* arr_ptr,
            uint32 x1, uint32 y1, uint32 x2, uint32 y2
        );

        /**
         * Writes a new scaled subfile by tiles.
         * @note This is a slow pixel operation.
         * @tparam T Data type of the image component (i.e. pixel).
         * @tparam U Data type of a subfile component (i.e. pixel).
         * @param tiff TIFF handle from libtiff.
         * @param subfile_idx Index of the subfile.
         * @param arr_ptr image buffer where to read from.
         * @param sfactor Scaling factor.
         */
        template <typename T, typename U>
        static void WriteScaledSubfileByTile(
            TIFF* tiff, uint16 subfile_idx, T* arr_ptr, float sfactor
        );

        /**
         * Writes a downsampled subfile by tiles.
         * The subfile in in_tiff is downsampled to half and stored in out_file.
         * @tparam T Data type of the image buffer.
         * @param in_tiff TIFF handle from libtiff.
         * @param out_tiff TIFF handle from libtiff.
         * @param in_subfile_idx Index of the subfile to read from.
         * @param out_subfile_idx Index of the subfile to write to.
         */
        template <typename T>
        static void WriteDownsampledSubfileByTile(
            TIFF* in_tiff, TIFF* out_tiff,
            uint16 in_subfile_idx, uint16 out_subfile_idx
        );
};

#endif /* __TIFFWRITER_H__ */
