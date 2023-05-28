// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

namespace chronicle {

struct ShaderInfo {
    std::string filename = {};
    std::vector<std::string> macroDefinitions = {};
};

} // namespace chronicle

template <> struct std::hash<chronicle::ShaderInfo> {
    std::size_t operator()(const chronicle::ShaderInfo& data) const noexcept
    {
        std::size_t h = 0;
        std::hash_combine(h, data.filename);
        for (const auto& macroDefinition : data.macroDefinitions) {
            std::hash_combine(h, macroDefinition);
        }
        return h;
    }
};