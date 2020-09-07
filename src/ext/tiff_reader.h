#ifndef __TIFFREADER_H__
#define __TIFFREADER_H__

#include <cstring>
#include <iostream>
#include <string>

#include <tiffio.h>

#include "utils.h"


/**
 * Internal class for reading TIFF files.
 */
class TiffReader {
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
         * Reads a subfile by scanlines.
         * @tparam T Data type of a subfile component (i.e. pixel).
         * @param tiff TIFF handle from libtiff.
         * @param subfile_idx Index of the subfile.
         * @param arr_ptr image buffer where to write to.
         */
        template <typename T>
        static void ReadSubfileByScanline(TIFF* tiff, uint16 subfile_idx, T* arr_ptr);

        /**
         * Reads a region of a subfile by scanlines.
         * @tparam T Data type of a subfile component (i.e. pixel).
         * @param tiff TIFF handle from libtiff.
         * @param subfile_idx Index of the subfile.
         * @param arr_ptr image buffer where to write to.
         * @param x1 Upper left x-coordinate (incl).
         * @param y1 Upper left y-coordinate (incl).
         * @param x2 Lower right x-coordinate (excl).
         * @param y2 Lower right y-coordinate (excl).
         */
        template <typename T>
        static void ReadSubfileRegionByScanline(
            TIFF* tiff, uint16 subfile_idx, T* arr_ptr,
            uint32 x1, uint32 y1, uint32 x2, uint32 y2
        );

        /**
         * Reads a subfile by strips.
         * @note This operation is not yet supported!
         * @tparam T Data type of a subfile component (i.e. pixel).
         * @param tiff TIFF handle from libtiff.
         * @param subfile_idx Index of the subfile.
         * @param arr_ptr image buffer where to write to.
         */
        template <typename T>
        static void ReadSubfileByStrip(TIFF* tiff, uint16 subfile_idx, T* arr_ptr) {
            throw std::runtime_error("Reading a TIFF file by strips is not supported!");
        }

        /**
         * Reads a region of a subfile by strips.
         * @note This operation is not yet supported!
         * @tparam T Data type of a subfile component (i.e. pixel).
         * @param tiff TIFF handle from libtiff.
         * @param subfile_idx Index of the subfile.
         * @param arr_ptr image buffer where to write to.
         * @param x1 Upper left x-coordinate (incl).
         * @param y1 Upper left y-coordinate (incl).
         * @param x2 Lower right x-coordinate (excl).
         * @param y2 Lower right y-coordinate (excl).
         */
        template <typename T>
        static void ReadSubfileRegionByStrip(
            TIFF* tiff, uint16 subfile_idx, T* arr_ptr,
            uint32 x1, uint32 y1, uint32 x2, uint32 y2
        ) {
            throw std::runtime_error("Reading a TIFF file by strips is not supported!");
        }

        /**
         * Reads a subfile by tiles.
         * @tparam T Data type of a subfile component (i.e. pixel).
         * @param tiff TIFF handle from libtiff.
         * @param subfile_idx Index of the subfile.
         * @param arr_ptr image buffer where to write to.
         */
        template <typename T>
        static void ReadSubfileByTile(TIFF* tiff, uint16 subfile_idx, T* arr_ptr);

        /**
         * Reads a region of a subfile by tiles.
         * @tparam T Data type of a subfile component (i.e. pixel).
         * @param tiff TIFF handle from libtiff.
         * @param subfile_idx Index of the subfile.
         * @param arr_ptr image buffer where to write to.
         * @param x1 Upper left x-coordinate (incl).
         * @param y1 Upper left y-coordinate (incl).
         * @param x2 Lower right x-coordinate (excl).
         * @param y2 Lower right y-coordinate (excl).
         */
        template <typename T>
        static void ReadSubfileRegionByTile(
            TIFF* tiff, uint16 subfile_idx, T* arr_ptr,
            uint32 x1, uint32 y1, uint32 x2, uint32 y2
        );
};

#endif /* __TIFFREADER_H__ */
