#pragma once

#include "pch.h"

#ifdef VULKAN_RENDERER
#include "Vulkan/VulkanDescriptorSet.h"
#endif

namespace chronicle {

class Renderer;

class DescriptorSet {
public:
    template <class T> inline void addUniform(entt::hashed_string::hash_type id, ShaderStage stage)
    {
        _descriptorSet.addUniform<T>(id, stage);
    }

    inline void addSampler(ShaderStage stage, const std::shared_ptr<Image> image)
    {
        _descriptorSet.addSampler(stage, image);
    }

    template <class T> inline void setUniform(entt::hashed_string::hash_type id, const T& data)
    {
        _descriptorSet.setUniform<T>(id, data);
    }

    inline void build() { _descriptorSet.build(); }

    static DescriptorSet create(const Renderer* renderer)
    {
        return DescriptorSet(VulkanDescriptorSet::create(renderer));
    }

private:
#ifdef VULKAN_RENDERER
    VulkanDescriptorSet _descriptorSet;

    explicit DescriptorSet(VulkanDescriptorSet descriptorSet)
        : _descriptorSet(std::move(descriptorSet))
    {
    }

    [[nodiscard]] inline const VulkanDescriptorSet& native() const { return _descriptorSet; };

    friend class VulkanCommandBuffer;
#endif
};

} // namespace chronicle