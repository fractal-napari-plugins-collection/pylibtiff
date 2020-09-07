#include "tiff_writer.h"


char TiffWriter::errorBuffer_[] = {};

void TiffWriter::ErrorHandler(
    const char* module, const char* format, va_list args
) {
    vsnprintf(errorBuffer_, 1024, format, args);
}


template <typename T>
void TiffWriter::WriteSubfileByScanline(
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
        if (TIFFWriteScanline(tiff, &arr_ptr[(img_row * image_width) * samples_per_pixel], img_row) < 0) {
            throw std::runtime_error(
                "Error while writing image row '" + std::to_string(img_row) + "'!\n" +
                std::string(errorBuffer_)
            );
        }
    }
}

template <typename T>
void TiffWriter::_WriteSubfileRegionByScanline(
    std::string in_file_path, std::string out_file_path,
    uint8 version, uint16 subfile_idx, T* arr_ptr,
    uint32 x1, uint32 y1, uint32 x2, uint32 y2
) {
    TIFFSetErrorHandler(ErrorHandler);

    TIFF* tiff_r = nullptr;
    if (version == 42) {
        tiff_r = TIFFOpen(in_file_path.c_str(), "r");
    } else {
        tiff_r = TIFFOpen(in_file_path.c_str(), "r8");
    }
    TIFFSetDirectory(tiff_r, subfile_idx);
    TIFF* tiff_w = nullptr;
    if (version == 42) {
        tiff_w = TIFFOpen((out_file_path).c_str(), "w");
    } else {
        tiff_w = TIFFOpen((out_file_path).c_str(), "w8");
    }

    uint32 image_width, image_length;
    uint16 samples_per_pixel, bits_per_sample, planar_config;
    if (!TIFFGetField(tiff_r, TIFFTAG_IMAGEWIDTH, &image_width))
        throw std::runtime_error("Missing field 'ImageWidth'!");
    if (!TIFFGetField(tiff_r, TIFFTAG_IMAGELENGTH, &image_length))
        throw std::runtime_error("Missing field 'ImageLength'!");
    if (!TIFFGetField(tiff_r, TIFFTAG_BITSPERSAMPLE, &bits_per_sample))
        bits_per_sample = 1;  // default
    if (!TIFFGetField(tiff_r, TIFFTAG_SAMPLESPERPIXEL, &samples_per_pixel))
        samples_per_pixel = 1;  // default
    if (!TIFFGetField(tiff_r, TIFFTAG_PLANARCONFIG, &planar_config))
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

    TIFFSetField(tiff_w, TIFFTAG_IMAGEWIDTH, image_width);
    TIFFSetField(tiff_w, TIFFTAG_IMAGELENGTH, image_length);
    TIFFSetField(tiff_w, TIFFTAG_BITSPERSAMPLE, bits_per_sample);
    TIFFSetField(tiff_w, TIFFTAG_SAMPLESPERPIXEL, samples_per_pixel);
    TIFFSetField(tiff_w, TIFFTAG_PLANARCONFIG, planar_config);

    TIFFSetField(tiff_w, TIFFTAG_SUBFILETYPE, 0);
    TIFFSetField(tiff_w, TIFFTAG_PHOTOMETRIC, 1);
    TIFFSetField(tiff_w, TIFFTAG_ROWSPERSTRIP, 4294967295);

    uint32 arr_width = x2 - x1;

    T* buffer = (T*) _TIFFmalloc(TIFFScanlineSize(tiff_r));
    for (uint32 img_row = 0; img_row < image_length; img_row++) {
        if (TIFFReadScanline(tiff_r, buffer, img_row) < 0) {
            throw std::runtime_error(
                "Error while reading image row '" + std::to_string(img_row) + "'!\n" +
                std::string(errorBuffer_)
            );
        }

        if (y1 <= img_row && img_row < y2) {
            std::memcpy(
                &buffer[x1 * samples_per_pixel],
                &arr_ptr[
                    ((img_row - y1) * arr_width) *
                    samples_per_pixel
                ],
                arr_width * samples_per_pixel * (bits_per_sample / 8)
            );
        }

        if (TIFFWriteScanline(tiff_w, buffer, img_row) < 0) {
            throw std::runtime_error(
                "Error while writing image row '" + std::to_string(img_row) + "'!\n" +
                std::string(errorBuffer_)
            );
        }
    }
    _TIFFfree(buffer);
    TIFFClose(tiff_r);
    TIFFClose(tiff_w);
}

template <typename T>
void TiffWriter::WriteSubfileByTile(
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
            uint32 pixels_to_copy = min(tile_size, image_width - img_column);

            for (
                uint32 tile_row = 0;
                tile_row < (uint32) min(tile_size, image_length - img_row);
                tile_row++
            ) {
                std::memcpy(
                    &buffer[
                        (tile_row * tile_size) * samples_per_pixel
                    ],
                    &arr_ptr[
                        ((img_row + tile_row) * image_width + img_column) * samples_per_pixel
                    ],
                    pixels_to_copy * samples_per_pixel * (bits_per_sample / 8)
                );
            }

            if (TIFFWriteTile(tiff, buffer, img_column, img_row, 0, 0) < 0) {
                throw std::runtime_error(
                    "Error while writing image tile (" + std::to_string(img_column) + ", " + std::to_string(img_row) + ")!\n" +
                    std::string(errorBuffer_)
                );
            }
        }
    }
    _TIFFfree(buffer);
}

template <typename T>
void TiffWriter::_WriteSubfileRegionByTile(
    std::string in_file_path, std::string out_file_path,
    uint8 version, uint16 subfile_idx,
    T* arr_ptr,
    uint32 x1, uint32 y1, uint32 x2, uint32 y2
) {
    TIFFSetErrorHandler(ErrorHandler);

    TIFF* tiff_r = nullptr;
    if (version == 42) {
        tiff_r = TIFFOpen(in_file_path.c_str(), "r");
    } else {
        tiff_r = TIFFOpen(in_file_path.c_str(), "r8");
    }
    TIFFSetDirectory(tiff_r, subfile_idx);
    TIFF* tiff_w = nullptr;
    if (version == 42) {
        tiff_w = TIFFOpen((out_file_path).c_str(), "w");
    } else {
        tiff_w = TIFFOpen((out_file_path).c_str(), "w8");
    }

    uint32 image_width, image_length;
    uint16 samples_per_pixel, bits_per_sample;
    uint32 tile_width, tile_length, tile_size;
    if (!TIFFGetField(tiff_r, TIFFTAG_IMAGEWIDTH, &image_width))
        throw std::runtime_error("Missing field 'ImageWidth'!");
    if (!TIFFGetField(tiff_r, TIFFTAG_IMAGELENGTH, &image_length))
        throw std::runtime_error("Missing field 'ImageLength'!");
    if (!TIFFGetField(tiff_r, TIFFTAG_BITSPERSAMPLE, &bits_per_sample))
        bits_per_sample = 1;  // default
    if (!TIFFGetField(tiff_r, TIFFTAG_SAMPLESPERPIXEL, &samples_per_pixel))
        samples_per_pixel = 1;  // default
    if(!TIFFGetField(tiff_r, TIFFTAG_TILEWIDTH, &tile_width))
        throw std::runtime_error("Missing field 'TileWidth'!");
    if(!TIFFGetField(tiff_r, TIFFTAG_TILELENGTH, &tile_length))
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

    TIFFSetField(tiff_w, TIFFTAG_IMAGEWIDTH, image_width);
    TIFFSetField(tiff_w, TIFFTAG_IMAGELENGTH, image_length);
    TIFFSetField(tiff_w, TIFFTAG_BITSPERSAMPLE, bits_per_sample);
    TIFFSetField(tiff_w, TIFFTAG_SAMPLESPERPIXEL, samples_per_pixel);
    TIFFSetField(tiff_w, TIFFTAG_TILEWIDTH, tile_size);
    TIFFSetField(tiff_w, TIFFTAG_TILELENGTH, tile_size);

    TIFFSetField(tiff_w, TIFFTAG_SUBFILETYPE, 0);
    TIFFSetField(tiff_w, TIFFTAG_PHOTOMETRIC, 1);
    TIFFSetField(tiff_w, TIFFTAG_PLANARCONFIG, 1);
    TIFFSetField(tiff_w, TIFFTAG_SAMPLEFORMAT, 1);

    // round to tile boundaries
    uint32 img_y1_aligned = y1 - (y1 % tile_size);
    uint32 img_x1_aligned = x1 - (x1 % tile_size);
    uint32 img_y2_aligned = y2 + (tile_size - 1) - ((y2 + (tile_size - 1)) % tile_size);
    uint32 img_x2_aligned = x2 + (tile_size - 1) - ((x2 + (tile_size - 1)) % tile_size);

    uint32 arr_width = x2 - x1;

    T* buffer = (T*) _TIFFmalloc(TIFFTileSize(tiff_r));

    for (uint32 img_row = 0; img_row < image_length; img_row += tile_size) {
        for (uint32 img_column = 0; img_column < image_width; img_column += tile_size) {
            if (TIFFReadTile(tiff_r, buffer, img_column, img_row, 0, 0) < 0) {
                throw std::runtime_error(
                    "Error while reading image tile (" + std::to_string(img_column) + ", " + std::to_string(img_row) + ")!\n" +
                    std::string(errorBuffer_)
                );
            }

            if (
                (img_y1_aligned <= img_row && img_row < img_y2_aligned) &&
                (img_x1_aligned <= img_column && img_column < img_x2_aligned)
            ) {
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
                        &buffer[
                            (buffer_row * tile_size + max(0, x1 - img_column)) * samples_per_pixel
                        ],
                        &arr_ptr[
                            (arr_row * arr_width + max(0, img_column - x1)) * samples_per_pixel
                        ],
                        pixels_to_copy * samples_per_pixel * (bits_per_sample / 8)
                    );
                }
            }

            if (TIFFWriteTile(tiff_w, buffer, img_column, img_row, 0, 0) < 0) {
                throw std::runtime_error(
                    "Error while writing image tile (" + std::to_string(img_column) + ", " + std::to_string(img_row) + ")!\n" +
                    std::string(errorBuffer_)
                );
            }
        }
    }
    _TIFFfree(buffer);
    TIFFClose(tiff_r);
    TIFFClose(tiff_w);
}

template <typename T, typename U>
void TiffWriter::WriteScaledSubfileByTile(
    TIFF* tiff, uint16 subfile_idx, T* arr_ptr, float sfactor
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

    U* buffer = (U*) _TIFFmalloc(TIFFTileSize(tiff));

    for (uint32 img_row = 0; img_row < image_length; img_row += tile_size) {
        for (uint32 img_column = 0; img_column < image_width; img_column += tile_size) {
            std::memset(buffer, 0, TIFFTileSize(tiff));
            
            for (uint32 tile_row=0; tile_row < static_cast<uint32>(min(tile_length, image_length - img_row)); tile_row++) {
                for (uint32 tile_column=0; tile_column < static_cast<uint32>(min(tile_width, image_width - img_column)); tile_column++) {
                    buffer[tile_row * tile_width + tile_column] = (U) (
                        float(arr_ptr[(img_row + tile_row) * image_width + img_column + tile_column]) * sfactor
                    );
                }
            }

            if (TIFFWriteTile(tiff, buffer, img_column, img_row, 0, 0) < 0) {
                throw std::runtime_error(
                    "Error while writing image tile (" + std::to_string(img_column) + ", " + std::to_string(img_row) + ")!\n" +
                    std::string(errorBuffer_)
                );
            }
        }
    }
    _TIFFfree(buffer);
}

template <typename T>
void TiffWriter::WriteDownsampledSubfileByTile(
    TIFF* in_tiff, TIFF* out_tiff, uint16 in_subfile_idx, uint16 out_subfile_idx
) {
    TIFFSetErrorHandler(ErrorHandler);

    TIFFSetDirectory(in_tiff, in_subfile_idx);
    TIFFSetDirectory(out_tiff, out_subfile_idx);

    uint32 image_width, image_length;
    uint16 samples_per_pixel, bits_per_sample;
    uint32 tile_width, tile_length;
    if (!TIFFGetField(in_tiff, TIFFTAG_IMAGEWIDTH, &image_width))
        throw std::runtime_error("Missing field 'ImageWidth'!");
    if (!TIFFGetField(in_tiff, TIFFTAG_IMAGELENGTH, &image_length))
        throw std::runtime_error("Missing field 'ImageLength'!");
    if (!TIFFGetField(in_tiff, TIFFTAG_BITSPERSAMPLE, &bits_per_sample))
        bits_per_sample = 1;  // default
    if (!TIFFGetField(in_tiff, TIFFTAG_SAMPLESPERPIXEL, &samples_per_pixel))
        samples_per_pixel = 1;  // default
    if(!TIFFGetField(in_tiff, TIFFTAG_TILEWIDTH, &tile_width))
        throw std::runtime_error("Missing field 'TileWidth'!");
    if(!TIFFGetField(in_tiff, TIFFTAG_TILELENGTH, &tile_length))
        throw std::runtime_error("Missing field 'TileLength'!");

    if (tile_width != tile_length)
        throw std::runtime_error("The fields 'TileLength' and 'TileWidth' must have the same value!");

    T* in_buffer = (T*) _TIFFmalloc(TIFFTileSize(in_tiff));
    T* out_buffer = (T*) _TIFFmalloc(TIFFTileSize(out_tiff));

    for (uint32 img_row=0; img_row < image_length; img_row += tile_length * 2) {
        for (uint32 img_column=0; img_column < image_width; img_column += tile_width * 2) {
            for (uint32 row_delta = 0; row_delta < 2 * tile_length && img_row + row_delta < image_length; row_delta += tile_length) {
                for (uint32 column_delta = 0; column_delta < 2 * tile_width && img_column + column_delta < image_width; column_delta += tile_width) {
                    std::memset(in_buffer, 255, TIFFTileSize(in_tiff));
                    TIFFReadTile(in_tiff, in_buffer, img_column + column_delta, img_row + row_delta, 0, 0);
                    for (uint32 y = 0; y < tile_length && img_row + row_delta + y < image_length; y += 2) {
                        for (uint32 x = 0; x < tile_width && img_column + column_delta + x < image_width; x += 2) {
                            uint32 i1 = in_buffer[y * tile_width + x];
                            uint32 i2 = in_buffer[y * tile_width + x + 1];
                            uint32 i3 = in_buffer[(y + 1) * tile_width + x];
                            uint32 i4 = in_buffer[(y + 1) * tile_width + x + 1];
                            out_buffer[
                                ((y + row_delta) / 2) * tile_width + ((x + column_delta) / 2)
                            ] = (
                                (i1 + i2 + i3 + i4) / 4
                            );
                        }
                    }
                }
            }
            if (TIFFWriteTile(out_tiff, out_buffer, img_column / 2, img_row / 2, 0, 0) < 0) {
                throw std::runtime_error(
                    "Error while writing image tile (" + std::to_string(img_column) + "," + std::to_string(img_row) + ")!\n" +
                    std::string(errorBuffer_)
                );
            }
        }
    }

    _TIFFfree(out_buffer);
    _TIFFfree(in_buffer);
}


// explicit instantiation of templates
template void TiffWriter::WriteSubfileByScanline<uint8>(TIFF*, uint16, uint8*);
template void TiffWriter::WriteSubfileByScanline<uint16>(TIFF*, uint16, uint16*);
template void TiffWriter::_WriteSubfileRegionByScanline<uint8>(std::string, std::string, uint8, uint16, uint8*, uint32, uint32, uint32, uint32);
template void TiffWriter::_WriteSubfileRegionByScanline<uint16>(std::string, std::string, uint8, uint16, uint16*, uint32, uint32, uint32, uint32);
template void TiffWriter::WriteSubfileByTile<uint8>(TIFF*, uint16, uint8*);
template void TiffWriter::WriteSubfileByTile<uint16>(TIFF*, uint16, uint16*);
template void TiffWriter::_WriteSubfileRegionByTile<uint8>(std::string, std::string, uint8, uint16, uint8*, uint32, uint32, uint32, uint32);
template void TiffWriter::_WriteSubfileRegionByTile<uint16>(std::string, std::string, uint8, uint16, uint16*, uint32, uint32, uint32, uint32);
template void TiffWriter::WriteScaledSubfileByTile<uint8, uint8>(TIFF*, uint16, uint8*, float sfactor);
template void TiffWriter::WriteScaledSubfileByTile<uint8, uint16>(TIFF*, uint16, uint8*, float sfactor);
template void TiffWriter::WriteScaledSubfileByTile<uint16, uint8>(TIFF*, uint16, uint16*, float sfactor);
template void TiffWriter::WriteScaledSubfileByTile<uint16, uint16>(TIFF*, uint16, uint16*, float sfactor);
template void TiffWriter::WriteDownsampledSubfileByTile<uint8>(TIFF*, TIFF*, uint16, uint16);
template void TiffWriter::WriteDownsampledSubfileByTile<uint16>(TIFF*, TIFF*, uint16, uint16);
