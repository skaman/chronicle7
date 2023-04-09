#pragma once

#include "pch.h"

#include "Common.h"

namespace chronicle {

template <class T> class FenceI {
public:
    static FenceRef create(const Renderer* renderer) { return T::create(renderer); }

private:
    FenceI() = default;
    friend T;
};

} // namespace chronicle