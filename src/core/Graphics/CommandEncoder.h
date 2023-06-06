// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

namespace chronicle::graphics
{

struct CommandEncoderCreateInfo
{
    std::string name{};
};

class Buffer;

class CommandEncoder
{
  public:
    virtual ~CommandEncoder() = default;

    virtual void copyBufferToBuffer(const std::shared_ptr<Buffer> &source, uint64_t sourceOffset,
                                    const std::shared_ptr<Buffer> &destination, uint64_t destinationOffset,
                                    uint64_t size) = 0;
};

} // namespace chronicle::graphics