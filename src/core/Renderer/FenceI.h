#pragma once

#include "pch.h"

#include "Common.h"

namespace chronicle {

template <class T> class FenceI {
public:
    static FenceRef create() { return T::create(); }

private:
    FenceI() = default;
    friend T;
};

} // namespace chronicle