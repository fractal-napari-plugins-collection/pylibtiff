#include "tiff_file.h"


TiffFile::TiffFile(const std::string& file_path, uint8 version) :
    file_path_(file_path), version_(version), subfile_count_(0)
{
    if (!file_exists(file_path_))
        return;

    TIFF* tiff = TIFFOpen(file_path_.c_str(), "r");
    if (tiff == nullptr)
        throw std::runtime_error("Could not open file '" + std::string(file_path_) + "'!");

    std::fstream fs(file_path_.c_str(), std::ios::in | std::ios::binary);
    char byte_order;
    fs.read(&byte_order, 1);
    if (byte_order == 'I') {  // little-endian byte order
        fs.seekg(2);
    } else {                  // big-endian byte order
        fs.seekg(3);
    }
    fs.read(reinterpret_cast<char*>(&version_), 1);
    fs.close();

    if (version_ != 42 && version_ != 43) {
        throw std::runtime_error("Found unsupported TIFF version: " + std::to_string(version_) + "!");
    }

    subfile_count_ = 0;
    do {
        TiffTags tiff_tags;

        // Baseline
        if (!TIFFGetField(tiff, TIFFTAG_SUBFILETYPE, &tiff_tags.new_subfile_type))
            tiff_tags.new_subfile_type = 0;  // default
        if (!TIFFGetField(tiff, TIFFTAG_IMAGEWIDTH, &tiff_tags.image_width))
            throw std::runtime_error("Missing field 'ImageWidth'!");
        if (!TIFFGetField(tiff, TIFFTAG_IMAGELENGTH, &tiff_tags.image_length))
            throw std::runtime_error("Missing field 'ImageLength'!");
        if (!TIFFGetField(tiff, TIFFTAG_BITSPERSAMPLE, &tiff_tags.bits_per_sample))
            tiff_tags.bits_per_sample = 1;  // default
        if (!TIFFGetField(tiff, TIFFTAG_COMPRESSION, &tiff_tags.compression))
            tiff_tags.compression = 1;  // default
        if (!TIFFGetField(tiff, TIFFTAG_PHOTOMETRIC, &tiff_tags.photometric))
            throw std::runtime_error("Missing field 'PhotometricInterpretation'!");
        if (!TIFFGetField(tiff, TIFFTAG_SAMPLESPERPIXEL, &tiff_tags.samples_per_pixel))
            tiff_tags.samples_per_pixel = 1;  // default
        if (!TIFFGetField(tiff, TIFFTAG_ROWSPERSTRIP, &tiff_tags.rows_per_strip))
            tiff_tags.rows_per_strip = 4294967295;  // default: 2**32 - 1
        if (!TIFFGetField(tiff, TIFFTAG_MINSAMPLEVALUE, &tiff_tags.min_sample_value))
            tiff_tags.min_sample_value = 0;  // default
        if (!TIFFGetField(tiff, TIFFTAG_MAXSAMPLEVALUE, &tiff_tags.max_sample_value))
            tiff_tags.max_sample_value = (1 << tiff_tags.bits_per_sample) - 1;  // default
        if (!TIFFGetField(tiff, TIFFTAG_PLANARCONFIG, &tiff_tags.planar_config))
            tiff_tags.planar_config = 1;  // default
        // Extension
        if(
            !TIFFGetField(
                tiff, TIFFTAG_PAGENUMBER,
                &tiff_tags.page_number.page_number, &tiff_tags.page_number.page_count
            )
        ) {
            tiff_tags.page_number.page_number = 0;
            tiff_tags.page_number.page_count = 0;
        }
        if(!TIFFGetField(tiff, TIFFTAG_TILEWIDTH, &tiff_tags.tile_width)) {
            if (TIFFIsTiled(tiff))
                throw std::runtime_error("Missing field 'TileWidth'!");
            tiff_tags.tile_width = 0;
        }
        if (!TIFFGetField(tiff, TIFFTAG_TILELENGTH, &tiff_tags.tile_length)) {
            if (TIFFIsTiled(tiff))
                throw std::runtime_error("Missing field 'TileLength'!");
            tiff_tags.tile_length = 0;
        }
        if (!TIFFGetField(tiff, TIFFTAG_SAMPLEFORMAT, &tiff_tags.sample_format))
            tiff_tags.sample_format = 1;  // default

        subfile_tags_[subfile_count_] = tiff_tags;

        subfile_count_ += 1;
    } while (TIFFReadDirectory(tiff) > 0);

    TIFFClose(tiff);
}

TiffFile::TiffTags TiffFile::GetSubfileTags(uint16 subfile_idx) {
    if(subfile_idx < 0 || subfile_count_ <= subfile_idx)
        throw std::out_of_range("Subfile index out of range!");
    return subfile_tags_[subfile_idx];
}

uint32 TiffFile::GetSubfileType(uint16 subfile_idx) {
    if(subfile_idx < 0 || subfile_count_ <= subfile_idx)
        throw std::out_of_range("Subfile index out of range!");
    return subfile_tags_[subfile_idx].new_subfile_type;
}
uint32 TiffFile::GetImageWidth(uint16 subfile_idx) {
    if(subfile_idx < 0 || subfile_count_ <= subfile_idx)
        throw std::out_of_range("Subfile index out of range!");
    return subfile_tags_[subfile_idx].image_width;
}
uint32 TiffFile::GetImageLength(uint16 subfile_idx) {
    if(subfile_idx < 0 || subfile_count_ <= subfile_idx)
        throw std::out_of_range("Subfile index out of range!");
    return subfile_tags_[subfile_idx].image_length;
}
uint16 TiffFile::GetBitsPerSample(uint16 subfile_idx) {
    if(subfile_idx < 0 || subfile_count_ <= subfile_idx)
        throw std::out_of_range("Subfile index out of range!");
    return subfile_tags_[subfile_idx].bits_per_sample;
}
uint16 TiffFile::GetCompression(uint16 subfile_idx) {
    if(subfile_idx < 0 || subfile_count_ <= subfile_idx)
        throw std::out_of_range("Subfile index out of range!");
    return subfile_tags_[subfile_idx].compression;
}
uint16 TiffFile::GetPhotometricInterpretation(uint16 subfile_idx) {
    if(subfile_idx < 0 || subfile_count_ <= subfile_idx)
        throw std::out_of_range("Subfile index out of range!");
    return subfile_tags_[subfile_idx].photometric;
}
uint16 TiffFile::GetSamplesPerPixel(uint16 subfile_idx) {
    if(subfile_idx < 0 || subfile_count_ <= subfile_idx)
        throw std::out_of_range("Subfile index out of range!");
    return subfile_tags_[subfile_idx].samples_per_pixel;
}
uint32 TiffFile::GetRowsPerStrip(uint16 subfile_idx) {
    if(subfile_idx < 0 || subfile_count_ <= subfile_idx)
        throw std::out_of_range("Subfile index out of range!");
    return subfile_tags_[subfile_idx].rows_per_strip;
}
uint16 TiffFile::GetMinSampleValue(uint16 subfile_idx) {
    if(subfile_idx < 0 || subfile_count_ <= subfile_idx)
        throw std::out_of_range("Subfile index out of range!");
    return subfile_tags_[subfile_idx].min_sample_value;
}
uint16 TiffFile::GetMaxSampleValue(uint16 subfile_idx) {
    if(subfile_idx < 0 || subfile_count_ <= subfile_idx)
        throw std::out_of_range("Subfile index out of range!");
    return subfile_tags_[subfile_idx].max_sample_value;
}
uint16 TiffFile::GetPlanarConfiguration(uint16 subfile_idx) {
    if(subfile_idx < 0 || subfile_count_ <= subfile_idx)
        throw std::out_of_range("Subfile index out of range!");
    return subfile_tags_[subfile_idx].planar_config;
}
uint16 TiffFile::GetPageNumber(uint16 subfile_idx) {
    if(subfile_idx < 0 || subfile_count_ <= subfile_idx)
        throw std::out_of_range("Subfile index out of range!");
    return subfile_tags_[subfile_idx].page_number.page_number;
}
uint16 TiffFile::GetPageCount(uint16 subfile_idx) {
    if(subfile_idx < 0 || subfile_count_ <= subfile_idx)
        throw std::out_of_range("Subfile index out of range!");
    return subfile_tags_[subfile_idx].page_number.page_count;
}
uint32 TiffFile::GetTileWidth(uint16 subfile_idx) {
    if(subfile_idx < 0 || subfile_count_ <= subfile_idx)
        throw std::out_of_range("Subfile index out of range!");
    return subfile_tags_[subfile_idx].tile_width;
}
uint32 TiffFile::GetTileLength(uint16 subfile_idx) {
    if(subfile_idx < 0 || subfile_count_ <= subfile_idx)
        throw std::out_of_range("Subfile index out of range!");
    return subfile_tags_[subfile_idx].tile_length;
}
uint16 TiffFile::GetSampleFormat(uint16 subfile_idx) {
    if(subfile_idx < 0 || subfile_count_ <= subfile_idx)
        throw std::out_of_range("Subfile index out of range!");
    return subfile_tags_[subfile_idx].sample_format;
}

template <typename T>
py::array_t<T> TiffFile::ReadSubfile(uint16 subfile_idx) {
    if(subfile_idx < 0 || subfile_count_ <= subfile_idx)
        throw std::out_of_range("Subfile index out of range!");

    TIFF* tiff = nullptr;
    if (version_ == 42) {
        tiff = TIFFOpen(file_path_.c_str(), "r");
    } else {
        tiff = TIFFOpen(file_path_.c_str(), "r8");
    }

    if (tiff == nullptr) {
        throw std::runtime_error("Could not open file '" + std::string(file_path_) + "'!");
    }

    py::array_t<T> image = py::array(
        py::buffer_info(
            nullptr,                                                     // Pointer to data (nullptr -> ask NumPy to allocate!)
            sizeof(T),                                                   // Size of one item
            py::format_descriptor<T>::value,                             // Buffer format
            2,                                                           // How many dimensions? 
            { GetImageLength(subfile_idx), GetImageWidth(subfile_idx) }, // Number of elements for each dimension
            { sizeof(T) * GetImageWidth(subfile_idx), sizeof(T) }        // Strides for each dimension
        )
    );
    T* image_ptr = static_cast<T*>(image.request().ptr);

    if (TIFFIsTiled(tiff)) {
        TiffReader::ReadSubfileByTile<T>(
            tiff, subfile_idx, image_ptr
        );
    } else {
        TiffReader::ReadSubfileByScanline<T>(
            tiff, subfile_idx, image_ptr
        );
    }

    TIFFClose(tiff);

    return image;
}

template <typename T>
py::array_t<T> TiffFile::ReadSubfileRegion(
    uint16 subfile_idx, uint32 x1, uint32 y1, uint32 x2, uint32 y2
) {
    if(subfile_idx < 0 || subfile_count_ <= subfile_idx)
        throw std::out_of_range("Subfile index out of range!");

    TIFF* tiff = nullptr;
    if (version_ == 42) {
        tiff = TIFFOpen(file_path_.c_str(), "r");
    } else {
        tiff = TIFFOpen(file_path_.c_str(), "r8");
    }

    if (tiff == nullptr) {
        throw std::runtime_error("Could not open file '" + std::string(file_path_) + "'!");
    }

    uint32 region_length = y2 - y1;
    uint32 region_width = x2 - x1;

    DEBUG_PRINTF("crop_length/crop_width: %d, %d\n", region_length, region_width);

    py::array_t<T> region = py::array(
        py::buffer_info(
            nullptr,                                // Pointer to data (nullptr -> ask NumPy to allocate!)
            sizeof(T),                              // Size of one item
            py::format_descriptor<T>::value,        // Buffer format
            2,                                      // How many dimensions? 
            { region_length, region_width },        // Number of elements for each dimension
            { sizeof(T) * region_width, sizeof(T) } // Strides for each dimension
        )
    );
    T* region_ptr = static_cast<T*>(region.request().ptr);

    if (TIFFIsTiled(tiff)) {
        TiffReader::ReadSubfileRegionByTile<T>(
            tiff, subfile_idx, region_ptr, x1, y1, x2, y2
        );
    } else {
        TiffReader::ReadSubfileRegionByScanline<T>(
            tiff, subfile_idx, region_ptr, x1, y1, x2, y2
        );
    }

    TIFFClose(tiff);

    return region;
}

template <typename T>
void TiffFile::WriteSubfile(py::array_t<T> image, TiffTags tiff_tags, bool tiled) {
    TIFF* tiff = nullptr;
    if (GetSubfileCount() > 0) {
        if (version_ == 42) {
            tiff = TIFFOpen(file_path_.c_str(), "r");
        } else {
            tiff = TIFFOpen(file_path_.c_str(), "r8");
        }

        if (tiff == nullptr) {
            throw std::runtime_error("Could not open file '" + std::string(file_path_) + "'!");
        }

        TIFFSetDirectory(tiff, 0);
        if (TIFFIsTiled(tiff) != tiled)
            throw std::runtime_error("Cannot mix scanline- and tile-based images within the same TIFF file!");

        TIFFClose(tiff);
    }

    if (version_ == 42) {
        tiff = TIFFOpen(file_path_.c_str(), "a");
    } else {
        tiff = TIFFOpen(file_path_.c_str(), "a8");
    }

    if (tiff == nullptr) {
        throw std::runtime_error("Could not open file '" + std::string(file_path_) + "'!");
    }

    // Baseline
    TIFFSetField(tiff, TIFFTAG_SUBFILETYPE, tiff_tags.new_subfile_type);
    TIFFSetField(tiff, TIFFTAG_IMAGEWIDTH, tiff_tags.image_width);
    TIFFSetField(tiff, TIFFTAG_IMAGELENGTH, tiff_tags.image_length);
    TIFFSetField(tiff, TIFFTAG_BITSPERSAMPLE, tiff_tags.bits_per_sample);
    TIFFSetField(tiff, TIFFTAG_COMPRESSION, tiff_tags.compression);
    TIFFSetField(tiff, TIFFTAG_PHOTOMETRIC, tiff_tags.photometric);
    TIFFSetField(tiff, TIFFTAG_SAMPLESPERPIXEL, tiff_tags.samples_per_pixel);
    TIFFSetField(tiff, TIFFTAG_ROWSPERSTRIP, tiff_tags.rows_per_strip);
    TIFFSetField(tiff, TIFFTAG_MINSAMPLEVALUE, tiff_tags.min_sample_value);
    TIFFSetField(tiff, TIFFTAG_MAXSAMPLEVALUE, tiff_tags.max_sample_value);
    TIFFSetField(tiff, TIFFTAG_PLANARCONFIG, tiff_tags.planar_config);
    // Extension
    TIFFSetField(
        tiff, TIFFTAG_PAGENUMBER,
        tiff_tags.page_number.page_number, tiff_tags.page_number.page_count
    );

    if (tiled) {
        TIFFSetField(tiff, TIFFTAG_TILEWIDTH, tiff_tags.tile_width);  // sets tif->tif_flags |= TIFF_ISTILED
        TIFFSetField(tiff, TIFFTAG_TILELENGTH, tiff_tags.tile_length);  // sets tif->tif_flags |= TIFF_ISTILED
    }
    TIFFSetField(tiff, TIFFTAG_SAMPLEFORMAT, tiff_tags.sample_format);

    uint16 subfile_idx = GetSubfileCount();
    subfile_tags_[subfile_idx] = tiff_tags;
    subfile_count_ += 1;

    T* image_ptr = static_cast<T*>(make_c_style(image).request().ptr);

    if (tiled) {
        TiffWriter::WriteSubfileByTile<T>(
            tiff, subfile_idx, image_ptr
        );
    } else {
        TiffWriter::WriteSubfileByScanline<T>(
            tiff, subfile_idx, image_ptr
        );
    }

    TIFFClose(tiff);
}

template <typename T>
void TiffFile::WriteSubfileRegion(
    py::array_t<T> image, uint16 subfile_idx,
    uint32 x1, uint32 y1, uint32 x2, uint32 y2
) {
    if(subfile_idx < 0 || subfile_count_ <= subfile_idx)
        throw std::out_of_range("Subfile index out of range!");

    TIFF* tiff = nullptr;
    bool is_tiled = false;
    if (version_ == 42) {
        tiff = TIFFOpen(file_path_.c_str(), "r");
        is_tiled = TIFFIsTiled(tiff);
        TIFFClose(tiff);
        tiff = TIFFOpen(file_path_.c_str(), "a");
    } else {
        tiff = TIFFOpen(file_path_.c_str(), "r8");
        is_tiled = TIFFIsTiled(tiff);
        TIFFClose(tiff);
        tiff = TIFFOpen(file_path_.c_str(), "a8");
    }

    if (tiff == nullptr) {
        throw std::runtime_error("Could not open file '" + std::string(file_path_) + "'!");
    }

    T* image_ptr = static_cast<T*>(make_c_style(image).request().ptr);

    if (is_tiled) {
        TiffWriter::WriteSubfileRegionByTile<T>(
            tiff, subfile_idx, image_ptr,
            x1, y1, x2, y2
        );
    } else {
        TiffWriter::WriteSubfileRegionByScanline<T>(
            tiff, subfile_idx, image_ptr,
            x1, y1, x2, y2
        );
    }

    TIFFClose(tiff);
}

template <typename T>
void TiffFile::WriteMultiscaleSubfile(py::array_t<T> image, TiffTags tiff_tags) {
    if (GetSubfileCount() > 0) {
        throw std::runtime_error(
            "Cannot append a multi-scale subfiles to an existing TIFF file!"
        );
    }

    TIFF* out_tiff = nullptr;
    if (version_ == 42) {
        out_tiff = TIFFOpen(file_path_.c_str(), "w");
    } else {
        out_tiff = TIFFOpen(file_path_.c_str(), "w8");
    }

    if (out_tiff == nullptr) {
        throw std::runtime_error("Could not open file '" + std::string(file_path_) + "'!");
    }

    if (
        (tiff_tags.tile_width == 0) || (tiff_tags.tile_length == 0)
    )
        throw std::runtime_error(
            "A multi-scale image is written by tiles!\n"
            "Field 'TileLength' or 'TileWidth' is missing."
        );

    if (tiff_tags.bits_per_sample != 8) {
        printf(
            "WARNING: Can only write 8-bit multi-scale TIFF subfiles!\n"
            "Changed the bits per sample tag from '%d' to '8'.",
            tiff_tags.bits_per_sample
        );
        tiff_tags.bits_per_sample = 8;
    }

    // Baseline
    TIFFSetField(out_tiff, TIFFTAG_SUBFILETYPE, tiff_tags.new_subfile_type);
    TIFFSetField(out_tiff, TIFFTAG_IMAGEWIDTH, tiff_tags.image_width);
    TIFFSetField(out_tiff, TIFFTAG_IMAGELENGTH, tiff_tags.image_length);
    TIFFSetField(out_tiff, TIFFTAG_BITSPERSAMPLE, tiff_tags.bits_per_sample);
    TIFFSetField(out_tiff, TIFFTAG_COMPRESSION, tiff_tags.compression);
    TIFFSetField(out_tiff, TIFFTAG_PHOTOMETRIC, tiff_tags.photometric);
    TIFFSetField(out_tiff, TIFFTAG_SAMPLESPERPIXEL, tiff_tags.samples_per_pixel);
    TIFFSetField(out_tiff, TIFFTAG_ROWSPERSTRIP, tiff_tags.rows_per_strip);
    TIFFSetField(out_tiff, TIFFTAG_MINSAMPLEVALUE, tiff_tags.min_sample_value);
    TIFFSetField(out_tiff, TIFFTAG_MAXSAMPLEVALUE, tiff_tags.max_sample_value);
    TIFFSetField(out_tiff, TIFFTAG_PLANARCONFIG, tiff_tags.planar_config);
    // Extension
    TIFFSetField(out_tiff, TIFFTAG_TILEWIDTH, tiff_tags.tile_width);
    TIFFSetField(out_tiff, TIFFTAG_TILELENGTH, tiff_tags.tile_length);
    TIFFSetField(out_tiff, TIFFTAG_SAMPLEFORMAT, tiff_tags.sample_format);

    uint16 subfile_idx = GetSubfileCount();
    subfile_tags_[subfile_idx] = tiff_tags;
    subfile_count_ += 1;

    image = make_c_style(image);
    T* image_ptr = static_cast<T*>(image.request().ptr);

    uint32 max_value = 0;
    for (uint32 row = 0; row < tiff_tags.image_length; row++) {
        for (uint32 column = 0; column < tiff_tags.image_width; column++) {
            max_value = max(
                max_value, image_ptr[row * tiff_tags.image_width + column]
            );
        }
    }
    float scaling_factor = 255.f / max_value;

    TiffWriter::WriteScaledSubfileByTile<T, uint8>(
        out_tiff, subfile_idx, image_ptr, scaling_factor
    );
    TIFFWriteDirectory(out_tiff);

    const uint32 kPageCount = std::ceil(
        std::log2(
            float(max(tiff_tags.image_length, tiff_tags.image_width)) /
            tiff_tags.tile_length
        )
    ) + 1;

    tiff_tags.new_subfile_type = FILETYPE_REDUCEDIMAGE;

    for (uint32 page: range(0, kPageCount)) {
        // the TIFF handle must be renewed within each loop to reflect the
        // newly inserted subfile.
        TIFF* in_tiff = nullptr;
        if (version_ == 42) {
            in_tiff = TIFFOpen(file_path_.c_str(), "r");
        } else {
            in_tiff = TIFFOpen(file_path_.c_str(), "r8");
        }

        // Baseline
        TIFFSetField(out_tiff, TIFFTAG_SUBFILETYPE, tiff_tags.new_subfile_type);
        TIFFSetField(out_tiff, TIFFTAG_IMAGEWIDTH, tiff_tags.image_width >> (page + 1));
        TIFFSetField(out_tiff, TIFFTAG_IMAGELENGTH, tiff_tags.image_length >> (page + 1));
        TIFFSetField(out_tiff, TIFFTAG_BITSPERSAMPLE, tiff_tags.bits_per_sample);
        TIFFSetField(out_tiff, TIFFTAG_COMPRESSION, tiff_tags.compression);
        TIFFSetField(out_tiff, TIFFTAG_PHOTOMETRIC, tiff_tags.photometric);
        TIFFSetField(out_tiff, TIFFTAG_SAMPLESPERPIXEL, tiff_tags.samples_per_pixel);
        TIFFSetField(out_tiff, TIFFTAG_ROWSPERSTRIP, tiff_tags.rows_per_strip);
        TIFFSetField(out_tiff, TIFFTAG_MINSAMPLEVALUE, tiff_tags.min_sample_value);
        TIFFSetField(out_tiff, TIFFTAG_MAXSAMPLEVALUE, tiff_tags.max_sample_value);
        TIFFSetField(out_tiff, TIFFTAG_PLANARCONFIG, tiff_tags.planar_config);
        // Extension
        TIFFSetField(out_tiff, TIFFTAG_TILEWIDTH, tiff_tags.tile_width);
        TIFFSetField(out_tiff, TIFFTAG_TILELENGTH, tiff_tags.tile_length);
        TIFFSetField(out_tiff, TIFFTAG_SAMPLEFORMAT, tiff_tags.sample_format);

        TIFFCheckpointDirectory(out_tiff);

        subfile_tags_[GetSubfileCount()] = tiff_tags;
        subfile_count_ += 1;

        TiffWriter::WriteDownsampledSubfileByTile<uint8>(
            in_tiff, out_tiff, page, page + 1
        );

        TIFFWriteDirectory(out_tiff);

        TIFFClose(in_tiff);
    }

    TIFFClose(out_tiff);
}
