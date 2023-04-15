// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Common.h"

namespace chronicle {

template <class T> class FenceI {
public:
    [[nodiscard]] static FenceRef create() { return T::create(); }

private:
    FenceI() = default;
    friend T;
};

} // namespace chronicle