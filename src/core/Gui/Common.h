// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

namespace chronicle {

struct GuiContex {
    static inline vk::DescriptorPool descriptorPool;
    static inline vk::PipelineCache pipelineCache;
};

} // namespace chronicle