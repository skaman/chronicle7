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