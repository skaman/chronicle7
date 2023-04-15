// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Common.h"

namespace chronicle {

template <class T> class SemaphoreI {
public:
    static SemaphoreRef create() { return T::create(); }

private:
    SemaphoreI() = default;
    friend T;
};

} // namespace chronicle