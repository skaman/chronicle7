// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "Common/Common.h"
#include "Graphics/CommandEncoder.h"

#include <vulkan/vulkan.hpp>

namespace chronicle::graphics::internal::vulkan
{

class VulkanDevice;

class VulkanCommandEncoder : public CommandEncoder, private NonCopyable<VulkanCommandEncoder>
{
  public:
    explicit VulkanCommandEncoder(std::shared_ptr<VulkanDevice> device,
                                  const CommandEncoderCreateInfo &commandEncoderCreateInfo);
    ~VulkanCommandEncoder();

    void copyBufferToBuffer(const std::shared_ptr<Buffer> &source, uint64_t sourceOffset,
                            const std::shared_ptr<Buffer> &destination, uint64_t destinationOffset,
                            uint64_t size) override;

  private:
    std::shared_ptr<VulkanDevice> _device{};
    std::string _name{};

    vk::CommandBuffer _commandBuffer{};
};

} // namespace chronicle::graphics::internal::vulkan