// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "Storage.h"

extern const uint8_t bfont_ttf[];
extern const size_t bfont_ttf_len;

extern const uint8_t MaterialPbr_hlsl[];
extern const size_t MaterialPbr_hlsl_len;

extern const uint8_t MaterialPbr_glsl[];
extern const size_t MaterialPbr_glsl_len;

extern const uint8_t test_hlslh[];
extern const size_t test_hlslh_len;

namespace chronicle {

struct StorageContext {
    inline static std::unordered_map<std::string, std::vector<uint8_t>> embeddedAssets {};
};

void Storage::init()
{
    CHRZONE_STORAGE;

    registerFileData("bfont.ttf", bfont_ttf, bfont_ttf_len);
    registerFileData("MaterialPbr.hlsl", MaterialPbr_hlsl, MaterialPbr_hlsl_len);
    registerFileData("MaterialPbr.glsl", MaterialPbr_glsl, MaterialPbr_glsl_len);
    registerFileData("test.hlslh", test_hlslh, test_hlslh_len);
}

bool Storage::exists(const std::string& filename)
{
    CHRZONE_STORAGE;

    if (StorageContext::embeddedAssets.contains(filename))
        return true;

    return std::filesystem::exists(filename);
}

std::vector<uint8_t> Storage::readBytes(const std::string& filename)
{
    CHRZONE_STORAGE;

    assert(!filename.empty());

    if (StorageContext::embeddedAssets.contains(filename)) {
        return StorageContext::embeddedAssets[filename];
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

void Storage::registerFileData(const std::string& filename, const uint8_t* data, size_t size)
{
    CHRZONE_STORAGE;

    assert(!filename.empty());
    assert(data);
    assert(size > 0);

    auto key = fmt::format(":/{}", filename);

    assert(!StorageContext::embeddedAssets.contains(key));

    StorageContext::embeddedAssets[key] = getBytesFromConstData(data, size);
}

std::vector<uint8_t> Storage::getBytesFromConstData(const uint8_t* data, size_t size)
{
    auto vec = std::vector<uint8_t>(size);
    std::memcpy(vec.data(), data, size);
    return vec;
}

} // namespace chronicle