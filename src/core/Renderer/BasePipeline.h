// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Common/Common.h"
#include "Data/PipelineInfo.h"

namespace chronicle {

/// @brief Object used to handle a graphic pipeline.
/// @tparam T Type with implementation.
template <class T> class BasePipeline {
public:
    /// @brief Get the pipeline handle ID
    /// @return Pipeline ID
    [[nodiscard]] PipelineId pipelineId() const { return CRTP_CONST_THIS->pipelineId(); }

    /// @brief Get the pipeline layout handle ID
    /// @return Pipeline layout ID
    [[nodiscard]] PipelineLayoutId pipelineLayoutId() const { return CRTP_CONST_THIS->pipelineLayoutId(); }

    /// @brief Factory for create a new pipeline.
    /// @param pipelineInfo Informations used to create the pipeline.
    /// @param name Pipeline name.
    /// @return The pipeline.
    [[nodiscard]] static PipelineRef create(const PipelineInfo& pipelineInfo, const std::string& name)
    {
        return T::create(pipelineInfo, name);
    }

private:
    BasePipeline() = default;
    friend T;
};

} // namespace chronicle