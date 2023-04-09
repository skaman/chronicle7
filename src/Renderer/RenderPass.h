#pragma once

#include "pch.h"

#include "RenderPassInfo.h"

#include "Common.h"

namespace chronicle {

template <class T> class RenderPassI {
public:
    static RenderPassRef create(const Renderer* renderer, const RenderPassInfo& renderPassInfo)
    {
        return T::create(renderer, renderPassInfo);
    }

private:
    RenderPassI() = default;
    friend T;
};

} // namespace chronicle