// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "PipelineLoader.h"

#include "Renderer/PipelineInfo.h"

namespace chronicle {

struct PipelineLoaderContext {
    static inline std::unordered_map<size_t, std::weak_ptr<Pipeline>> cache = {};
};

PipelineRef PipelineLoader::load(const PipelineInfo& pipelineInfo, const char* debugName)
{
    auto hash = std::hash<PipelineInfo>()(pipelineInfo);
    if (auto pipeline = PipelineLoaderContext::cache[hash].lock()) {
        return pipeline;
    }

    auto pipeline = Pipeline::create(pipelineInfo, debugName);
    PipelineLoaderContext::cache[hash] = pipeline;
    return pipeline;
}

} // namespace chronicle