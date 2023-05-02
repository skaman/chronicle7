// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Renderer/Renderer.h"

namespace chronicle {

class ShaderLoader {
public:
    [[nodiscard]] static ShaderRef load(const std::string& filename);
};

} // namespace chronicle