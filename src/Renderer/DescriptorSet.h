#pragma once

#include "pch.h"

#ifdef VULKAN_RENDERER
#include "Vulkan/VulkanDescriptorSet.h"
#endif

namespace chronicle {

class Renderer;

class DescriptorSet {
public:
    explicit DescriptorSet(const Renderer* renderer);

    template <class T> inline void addUniform(entt::hashed_string::hash_type id, ShaderStage stage)
    {
        _descriptorSet.addUniform<T>(id, stage);
    }
    template <class T> inline void setUniform(entt::hashed_string::hash_type id, const T& data)
    {
        _descriptorSet.setUniform<T>(id, data);
    }

    inline void build() { _descriptorSet.build(); }

#ifdef VULKAN_RENDERER
    [[nodiscard]] inline const VulkanDescriptorSet& native() const { return _descriptorSet; };
#endif

private:
#ifdef VULKAN_RENDERER
    VulkanDescriptorSet _descriptorSet;
#endif
};

} // namespace chronicle