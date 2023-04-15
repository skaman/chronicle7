// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

namespace chronicle {

class FileError : public std::runtime_error {
public:
    using runtime_error::runtime_error;

    explicit FileError(const std::string& message)
        : runtime_error(message.c_str())
    {
    }
};

class File {
public:
    static std::vector<char> readBytes(const std::string& filename);
};

} // namespace chronicle