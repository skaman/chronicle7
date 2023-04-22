// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Common.h"
#include "PipelineInfo.h"

namespace chronicle {

/// @brief Object used to handle a graphic pipeline.
/// @tparam T Type with implementation.
template <class T> class PipelineI {
public:
    /// @brief Factory for create a new pipeline.
    /// @param pipelineInfo Informations used to create the pipeline.
    /// @param debugName Debug name.
    /// @return The pipeline.
    [[nodiscard]] static PipelineRef create(const PipelineInfo& pipelineInfo, const char* debugName)
    {
        return T::create(pipelineInfo, debugName);
    }

private:
    PipelineI() = default;
    friend T;
};

} // namespace chronicle