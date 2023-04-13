#pragma once

#include "pch.h"

namespace chronicle {

struct GuiContex {
    static inline vk::DescriptorPool descriptorPool;
    static inline vk::PipelineCache pipelineCache;
};

} // namespace chronicle