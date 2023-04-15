// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "RenderPassInfo.h"

#include "Common.h"

namespace chronicle {

template <class T> class RenderPassI {
public:
    [[nodiscard]] static RenderPassRef create(const RenderPassInfo& renderPassInfo)
    {
        return T::create(renderPassInfo);
    }

private:
    RenderPassI() = default;
    friend T;
};

} // namespace chronicle