#pragma once

#include "pch.h"

#include "Common.h"
#include "PipelineInfo.h"

namespace chronicle {

template <class T> class PipelineI {
public:
    static PipelineRef create(const Renderer* renderer, const PipelineInfo& pipelineInfo)
    {
        return T::create(renderer, pipelineInfo);
    }

private:
    PipelineI() = default;
    friend T;
};

} // namespace chronicle