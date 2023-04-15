// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Common.h"

namespace chronicle {

/// @brief Object used to record command which can be sebsequently submitted to GPU for execution.
/// @tparam T Type with implementation.
template <class T> class CommandBufferI {
public:
    void drawIndexed(uint32_t indexCount, uint32_t instanceCount) const
    {
        static_cast<const T*>(this)->drawIndexed(indexCount, instanceCount);
    }

    void bindPipeline(const PipelineRef& pipeline) { static_cast<T*>(this)->bindPipeline(pipeline); }
    void bindVertexBuffer(const VertexBufferRef& vertexBuffer) const
    {
        static_cast<const T*>(this)->bindVertexBuffer(vertexBuffer);
    }
    void bindIndexBuffer(const IndexBufferRef& indexBuffer) const
    {
        static_cast<const T*>(this)->bindIndexBuffer(indexBuffer);
    }
    void bindDescriptorSet(const DescriptorSetRef& descriptorSet, uint32_t index) const
    {
        static_cast<const T*>(this)->bindDescriptorSet(descriptorSet, index);
    }

private:
    CommandBufferI() = default;
    friend T;
};

} // namespace chronicle