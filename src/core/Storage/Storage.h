// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

namespace chronicle {

class StorageError : public std::runtime_error {
public:
    using runtime_error::runtime_error;

    explicit StorageError(const std::string& message)
        : runtime_error(message.c_str())
    {
    }
};

class Storage {
public:
    [[nodiscard]] static std::vector<uint8_t> readBytes(const std::string& filename);
    [[nodiscard]] static std::string readString(const std::string& filename);

private:
    [[nodiscard]] static std::vector<uint8_t> getBytesFromConstData(const uint8_t* data, size_t size);
};

} // namespace chronicle