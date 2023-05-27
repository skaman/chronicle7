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