// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "Storage.h"

extern const uint8_t bfont_ttf[];
extern const size_t bfont_ttf_len;

extern const uint8_t MaterialPbr_hlsl[];
extern const size_t MaterialPbr_hlsl_len;

namespace chronicle {

std::vector<uint8_t> Storage::readBytes(const std::string& filename)
{
    CHRZONE_STORAGE;

    if (filename == ":/bfont.ttf") {
        return getBytesFromConstData(bfont_ttf, bfont_ttf_len);
    } else if (filename == ":/MaterialPbr.hlsl") {
        return getBytesFromConstData(MaterialPbr_hlsl, MaterialPbr_hlsl_len);
    }

    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
        throw StorageError(fmt::format("Failed to open file {}", filename));

    size_t fileSize = file.tellg();
    std::vector<uint8_t> buffer(fileSize);

    file.seekg(0);
    file.read((char*)buffer.data(), fileSize);
    file.close();

    return buffer;
}

std::string Storage::readString(const std::string& filename)
{
    auto bytes = readBytes(filename);
    return std::string((char*)bytes.data(), bytes.size());
}

std::vector<uint8_t> Storage::getBytesFromConstData(const uint8_t* data, size_t size)
{
    auto vec = std::vector<uint8_t>(size);
    std::memcpy(vec.data(), data, size);
    return vec;
}

} // namespace chronicle