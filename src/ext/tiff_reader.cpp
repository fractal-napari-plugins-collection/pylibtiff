#include "tiff_reader.h"


char TiffReader::errorBuffer_[] = {};

void TiffReader::ErrorHandler(
    const char* module, const char* format, va_list args
) {
    vsnprintf(errorBuffer_, 1024, format, args);
}

template <typename T>
void TiffReader::ReadSubfileByScanline(
    TIFF* tiff, uint16 subfile_idx, T* arr_ptr
) {
    TIFFSetErrorHandler(ErrorHandler);

    TIFFSetDirectory(tiff, subfile_idx);

    uint32 image_width, image_length;
    uint16 samples_per_pixel, planar_config;
    if (!TIFFGetField(tiff, TIFFTAG_IMAGEWIDTH, &image_width))
        throw std::runtime_error("Missing field 'ImageWidth'!");
    if (!TIFFGetField(tiff, TIFFTAG_IMAGELENGTH, &image_length))
        throw std::runtime_error("Missing field 'ImageLength'!");
    if (!TIFFGetField(tiff, TIFFTAG_SAMPLESPERPIXEL, &samples_per_pixel))
        samples_per_pixel = 1;  // default
    if (!TIFFGetField(tiff, TIFFTAG_PLANARCONFIG, &planar_config))
        planar_config = 1;  // default

    if (planar_config != PLANARCONFIG_CONTIG)
        throw std::runtime_error(
            "Found unsupported planar configuration '" + std::to_string(planar_config) + "'!"
        );

    for (uint32 img_row = 0; img_row < image_length; img_row++) {
        if (TIFFReadScanline(tiff, &arr_ptr[(img_row * image_width) * samples_per_pixel], img_row) < 0) {
            throw std::runtime_error(
                "Error while reading image row '" + std::to_string(img_row) + "'!\n" +
                std::string(errorBuffer_)
            );
        }
    }
}

template <typename T>
void TiffReader::ReadSubfileRegionByScanline(
    TIFF* tiff, uint16 subfile_idx, T* arr_ptr,
    uint32 x1, uint32 y1, uint32 x2, uint32 y2
) {
    TIFFSetErrorHandler(ErrorHandler);

    TIFFSetDirectory(tiff, subfile_idx);

    uint32 image_width, image_length;
    uint16 samples_per_pixel, bits_per_sample, planar_config;
    if (!TIFFGetField(tiff, TIFFTAG_IMAGEWIDTH, &image_width))
        throw std::runtime_error("Missing field 'ImageWidth'!");
    if (!TIFFGetField(tiff, TIFFTAG_IMAGELENGTH, &image_length))
        throw std::runtime_error("Missing field 'ImageLength'!");
    if (!TIFFGetField(tiff, TIFFTAG_BITSPERSAMPLE, &bits_per_sample))
        bits_per_sample = 1;  // default
    if (!TIFFGetField(tiff, TIFFTAG_SAMPLESPERPIXEL, &samples_per_pixel))
        samples_per_pixel = 1;  // default
    if (!TIFFGetField(tiff, TIFFTAG_PLANARCONFIG, &planar_config))
        planar_config = 1;  // default

    if (planar_config != PLANARCONFIG_CONTIG)
        throw std::runtime_error(
            "Found unsupported planar configuration '" + std::to_string(planar_config) + "'!"
        );

    if (y1 < 0 || image_length < y1)
        throw std::runtime_error("y1 out of range!");
    if (y2 < 0 || image_length < y2)
        throw std::runtime_error("y2 out of range!");
    if (x1 < 0 || image_width < x1)
        throw std::runtime_error("x1 out of range!");
    if (x2 < 0 || image_width < x2)
        throw std::runtime_error("x2 out of range!");

    if (x1 >= x2 || y1 >= y2)
        throw std::runtime_error("Invalid crop dimensions defined!");

    uint32 arr_width = x2 - x1;

    uint32 img_row, arr_row;
    T* buffer = (T*) _TIFFmalloc(TIFFScanlineSize(tiff));  // may include padding
    for (img_row = y1, arr_row = 0; img_row < y2; img_row++, arr_row++) {
        if (TIFFReadScanline(tiff, buffer, img_row) < 0) {
            throw std::runtime_error(
                "Error while reading image row '" + std::to_string(img_row) + "'!\n" +
                std::string(errorBuffer_)
            );
        }

        std::memcpy(
            &arr_ptr[
                (arr_row * arr_width) *
                samples_per_pixel
            ],
            &buffer[x1 * samples_per_pixel],
            arr_width * samples_per_pixel * (bits_per_sample / 8)
        );
    }
    _TIFFfree(buffer);
}

template <typename T>
void TiffReader::ReadSubfileByTile(
    TIFF* tiff, uint16 subfile_idx, T* arr_ptr
) {
    TIFFSetErrorHandler(ErrorHandler);

    TIFFSetDirectory(tiff, subfile_idx);

    uint32 image_width, image_length;
    uint16 samples_per_pixel, bits_per_sample;
    uint32 tile_width, tile_length, tile_size;
    if (!TIFFGetField(tiff, TIFFTAG_IMAGEWIDTH, &image_width))
        throw std::runtime_error("Missing field 'ImageWidth'!");
    if (!TIFFGetField(tiff, TIFFTAG_IMAGELENGTH, &image_length))
        throw std::runtime_error("Missing field 'ImageLength'!");
    if (!TIFFGetField(tiff, TIFFTAG_BITSPERSAMPLE, &bits_per_sample))
        bits_per_sample = 1;  // default
    if (!TIFFGetField(tiff, TIFFTAG_SAMPLESPERPIXEL, &samples_per_pixel))
        samples_per_pixel = 1;  // default
    if(!TIFFGetField(tiff, TIFFTAG_TILEWIDTH, &tile_width))
        throw std::runtime_error("Missing field 'TileWidth'!");
    if(!TIFFGetField(tiff, TIFFTAG_TILELENGTH, &tile_length))
        throw std::runtime_error("Missing field 'TileLength'!");

    if (tile_width != tile_length)
        throw std::runtime_error("The fields 'TileLength' and 'TileWidth' must have the same value!");
    tile_size = tile_length;

    T* buffer = (T*) _TIFFmalloc(TIFFTileSize(tiff));

    for (uint32 img_row = 0; img_row < image_length; img_row += tile_size) {
        for (uint32 img_column = 0; img_column < image_width; img_column += tile_size) {
            if (TIFFReadTile(tiff, buffer, img_column, img_row, 0, 0) < 0) {
                throw std::runtime_error(
                    "Error while reading image tile (" + std::to_string(img_column) + ", " + std::to_string(img_row) + ")!\n" +
                    std::string(errorBuffer_)
                );
            }

            uint32 pixels_to_copy = min(tile_size, image_width - img_column);

            for (
                uint32 tile_row = 0;
                tile_row < (uint32) min(tile_size, image_length - img_row);
                tile_row++
            ) {
                std::memcpy(
                    &arr_ptr[
                        ((img_row + tile_row) * image_width + img_column) * samples_per_pixel
                    ],
                    &buffer[
                        (tile_row * tile_size) * samples_per_pixel
                    ],
                    pixels_to_copy * samples_per_pixel * (bits_per_sample / 8)
                );
            }
        }
    }
    _TIFFfree(buffer);
}

template <typename T>
void TiffReader::ReadSubfileRegionByTile(
    TIFF* tiff, uint16 subfile_idx, T* arr_ptr,
    uint32 x1, uint32 y1, uint32 x2, uint32 y2
) {
    TIFFSetErrorHandler(ErrorHandler);

    TIFFSetDirectory(tiff, subfile_idx);

    uint32 image_width, image_length;
    uint16 samples_per_pixel, bits_per_sample;
    uint32 tile_width, tile_length, tile_size;
    if (!TIFFGetField(tiff, TIFFTAG_IMAGEWIDTH, &image_width))
        throw std::runtime_error("Missing field 'ImageWidth'!");
    if (!TIFFGetField(tiff, TIFFTAG_IMAGELENGTH, &image_length))
        throw std::runtime_error("Missing field 'ImageLength'!");
    if (!TIFFGetField(tiff, TIFFTAG_BITSPERSAMPLE, &bits_per_sample))
        bits_per_sample = 1;  // default
    if (!TIFFGetField(tiff, TIFFTAG_SAMPLESPERPIXEL, &samples_per_pixel))
        samples_per_pixel = 1;  // default
    if(!TIFFGetField(tiff, TIFFTAG_TILEWIDTH, &tile_width))
        throw std::runtime_error("Missing field 'TileWidth'!");
    if(!TIFFGetField(tiff, TIFFTAG_TILELENGTH, &tile_length))
        throw std::runtime_error("Missing field 'TileLength'!");

    if (tile_width != tile_length)
        throw std::runtime_error("The fields 'TileLength' and 'TileWidth' must have the same value!");
    tile_size = tile_length;

    if (y1 < 0 || image_length < y1)
        throw std::runtime_error("y1 out of range!");
    if (y2 < 0 || image_length < y2)
        throw std::runtime_error("y2 out of range!");
    if (x1 < 0 || image_width < x1)
        throw std::runtime_error("x1 out of range!");
    if (x2 < 0 || image_width < x2)
        throw std::runtime_error("x2 out of range!");

    if (x1 >= x2 || y1 >= y2)
        throw std::runtime_error("Invalid crop dimensions defined!");

    // round to tile boundaries
    uint32 img_y1_aligned = y1 - (y1 % tile_size);
    uint32 img_x1_aligned = x1 - (x1 % tile_size);
    uint32 img_y2_aligned = y2 + (tile_size - 1) - ((y2 + (tile_size - 1)) % tile_size);
    uint32 img_x2_aligned = x2 + (tile_size - 1) - ((x2 + (tile_size - 1)) % tile_size);

    uint32 arr_width = x2 - x1;

    T* buffer = (T*) _TIFFmalloc(TIFFTileSize(tiff));

    for (uint32 img_row = img_y1_aligned; img_row < img_y2_aligned; img_row += tile_size) {
        for (uint32 img_column = img_x1_aligned; img_column < img_x2_aligned; img_column += tile_size) {
            if (TIFFReadTile(tiff, buffer, img_column, img_row, 0, 0) < 0) {
                throw std::runtime_error(
                    "Error while reading image tile (" + std::to_string(img_column) + ", " + std::to_string(img_row) + ")!\n" +
                    std::string(errorBuffer_)
                );
            }

            uint32 pixels_to_copy = min(
                tile_size, min((img_column + tile_size) - x1, x2 - img_column)
            );

            uint32 buffer_row, arr_row;
            for (
                buffer_row = max(0, y1 - img_row), arr_row = max(0, img_row - y1);
                buffer_row < (uint32) min(tile_size, y2 - img_row);
                buffer_row++, arr_row++
            ) {
                std::memcpy(
                    &arr_ptr[
                        (arr_row * arr_width + max(0, img_column - x1)) * samples_per_pixel
                    ],
                    &buffer[
                        (buffer_row * tile_size + max(0, x1 - img_column)) * samples_per_pixel
                    ],
                    pixels_to_copy * samples_per_pixel * (bits_per_sample / 8)
                );
            }
        }
    }
    _TIFFfree(buffer);
}


// explicit instantiation of templates
template void TiffReader::ReadSubfileByScanline<uint8>(TIFF*, uint16, uint8*);
template void TiffReader::ReadSubfileByScanline<uint16>(TIFF*, uint16, uint16*);
template void TiffReader::ReadSubfileRegionByScanline<uint8>(TIFF*, uint16, uint8*, uint32, uint32, uint32, uint32);
template void TiffReader::ReadSubfileRegionByScanline<uint16>(TIFF*, uint16, uint16*, uint32, uint32, uint32, uint32);
template void TiffReader::ReadSubfileByTile<uint8>(TIFF*, uint16, uint8*);
template void TiffReader::ReadSubfileByTile<uint16>(TIFF*, uint16, uint16*);
template void TiffReader::ReadSubfileRegionByTile<uint8>(TIFF*, uint16, uint8*, uint32, uint32, uint32, uint32);
template void TiffReader::ReadSubfileRegionByTile<uint16>(TIFF*, uint16, uint16*, uint32, uint32, uint32, uint32);
