// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "File.h"

namespace chronicle {

std::vector<uint8_t> File::readAllBytes() const
{
    CHRZONE_STORAGE;

    assert(!_path.empty());

    std::ifstream file(_path, std::ios::ate | std::ios::binary | std::ios::in);

    if (!file.is_open())
        throw StorageError(fmt::format("Failed to open file {}", _path.string()));

    size_t fileSize = file.tellg();
    std::vector<uint8_t> buffer(fileSize);

    file.seekg(0);
    file.read((char*)buffer.data(), fileSize);
    file.close();

    return buffer;
}

std::string File::readAllText() const
{
    auto bytes = readAllBytes();
    return std::string((char*)bytes.data(), bytes.size());
}

} // namespace chronicle