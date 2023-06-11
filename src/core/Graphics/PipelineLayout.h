// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "BindGroupLayout.h"
#include "Common.h"

namespace chronicle::graphics
{

/// @brief Description for pipeline layout creation.
struct PipelineLayoutDescriptor
{
    /// @brief Pipeline layout name.
    std::string name{};

    /// @brief A list of BindGroupLayouts the pipeline will use.
    std::vector<std::shared_ptr<BindGroupLayout>> bindGroupLayouts{};
};

/// @brief A PipelineLayout defines the mapping between resources of all BindGroup objects set up during command
///        encoding in setBindGroup(), and the shaders of the pipeline set by RenderCommandsMixin::setPipeline or
///        ComputePassEncoder::setPipeline.
class PipelineLayout
{
  public:
    /// @brief Constructor.
    /// @param pipelineLayoutDescriptor Pipeline layout descriptor.
    explicit PipelineLayout(const PipelineLayoutDescriptor &pipelineLayoutDescriptor)
        : _pipelineLayoutDescriptor(pipelineLayoutDescriptor)
    {
    }

    /// @brief Destructor.
    virtual ~PipelineLayout() = default;

    /// @brief Returns the descriptor used to create the pipeline layout.
    /// @return Pipeline layout descriptor.
    const PipelineLayoutDescriptor &descriptor() const
    {
        return _pipelineLayoutDescriptor;
    }

  private:
    PipelineLayoutDescriptor _pipelineLayoutDescriptor; ///< Pipeline layout descriptor.
};

} // namespace chronicle::graphics