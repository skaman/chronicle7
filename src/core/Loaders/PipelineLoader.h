// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Renderer/Renderer.h"

namespace chronicle {

class PipelineLoader {
public:
    [[nodiscard]] static PipelineRef load(const PipelineInfo& pipelineInfo, const char* debugName);
};

} // namespace chronicle