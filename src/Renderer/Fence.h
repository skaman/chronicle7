#pragma once

#include "pch.h"

namespace chronicle {

class Renderer;
template <class T> class FenceI;

#ifdef VULKAN_RENDERER
class VulkanFence;
using Fence = FenceI<VulkanFence>;
#endif

using FenceRef = std::shared_ptr<Fence>;

template <class T> class FenceI {
public:
    static FenceRef create(const Renderer* renderer) { return T::createImpl(renderer); }
};

} // namespace chronicle