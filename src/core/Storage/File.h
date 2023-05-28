// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Common.h"

namespace chronicle {

class File {
protected:
    explicit File(const std::filesystem::path& path)
        : _path(path)
    {
    }

public:
    explicit File() = default;

    [[nodiscard]] std::vector<uint8_t> readAllBytes() const;
    [[nodiscard]] std::string readAllText() const;

private:
    std::filesystem::path _path {};
};

} // namespace chronicle