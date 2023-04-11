#pragma once

#include "pch.h"

#include "RenderPassInfo.h"

#include "Common.h"

namespace chronicle {

template <class T> class RenderPassI {
public:
    static RenderPassRef create(const RenderPassInfo& renderPassInfo) { return T::create(renderPassInfo); }

private:
    RenderPassI() = default;
    friend T;
};

} // namespace chronicle