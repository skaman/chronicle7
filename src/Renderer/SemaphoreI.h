#pragma once

#include "pch.h"

#include "Common.h"

namespace chronicle {

template <class T> class SemaphoreI {
public:
    static SemaphoreRef create(const Renderer* renderer) { return T::create(renderer); }

private:
    SemaphoreI() = default;
    friend T;
};

} // namespace chronicle