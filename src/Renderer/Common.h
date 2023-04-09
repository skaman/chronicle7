#pragma once

#include "pch.h"

namespace chronicle {

template <class T> struct Offset2D {
    Offset2D(T x = {}, T y = {})
        : x(x)
        , y(y)
    {
    }

    T x;
    T y;
};

using OffsetInt2D = Offset2D<int32_t>;
using OffsetFloat2D = Offset2D<float>;

template <class T> struct Extent2D {
    Extent2D(T width = {}, T height = {})
        : width(width)
        , height(height)
    {
    }

    T width;
    T height;
};

using ExtentInt2D = Extent2D<int32_t>;
using ExtentFloat2D = Extent2D<float>;

template <class T> struct Rect2D {
    Rect2D(Offset2D<T> offset = {}, Extent2D<T> extent = {})
        : offset(offset)
        , extent(extent)
    {
    }

    Offset2D<T> offset;
    Extent2D<T> extent;
};

using RectInt2D = Rect2D<int32_t>;
using RectFloat2D = Rect2D<float>;

enum class ShaderStage { Fragment, Vertex };
enum class Format {
    Undefined,

    B8G8R8A8Unorm,
    R32G32Sfloat,
    R32G32B32Sfloat,

    D32Sfloat,
    D32SfloatS8Uint,
    D24UnormS8Uint
};
enum class VertexInputRate { Vertex, Instance };

template <class T> class CommandBufferI;
template <class T> class DescriptorSetI;
template <class T> class FenceI;
template <class T> class VertexBufferI;

#ifdef VULKAN_RENDERER
class VulkanCommandBuffer;
class VulkanDescriptorSet;
class VulkanFence;
class VulkanVertexBuffer;

using CommandBuffer = CommandBufferI<VulkanCommandBuffer>;
using DescriptorSet = DescriptorSetI<VulkanDescriptorSet>;
using Fence = FenceI<VulkanFence>;
using VertexBuffer = VertexBufferI<VulkanVertexBuffer>;
#endif

using CommandBufferRef = std::shared_ptr<CommandBuffer>;
using DescriptorSetRef = std::shared_ptr<DescriptorSet>;
using FenceRef = std::shared_ptr<Fence>;
using VertexBufferRef = std::shared_ptr<VertexBuffer>;

class IndexBuffer;
class Pipeline;
class RenderPass;
class Semaphore;
class Renderer;
class Image;

} // namespace chronicle