// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "Buffer.h"
#include "CommandEncoder.h"
#include "Sampler.h"

namespace chronicle::graphics
{

class Device
{
  public:
    virtual ~Device() = default;

    virtual [[nodiscard]] std::shared_ptr<CommandEncoder> createCommandEncoder(
        const CommandEncoderCreateInfo &commandEncoderCreateInfo = {}) const = 0;

    virtual [[nodiscard]] std::shared_ptr<Buffer> createBuffer(const BufferCreateInfo &bufferCreateInfo) const = 0;
    virtual [[nodiscard]] std::shared_ptr<Sampler> createSampler(const SamplerCreateInfo &samplerCreateInfo) const = 0;
};

} // namespace chronicle::graphics