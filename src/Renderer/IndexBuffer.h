#pragma once

#include "pch.h"

#include "Common.h"

namespace chronicle {

template <class T> class IndexBufferI {
public:
    void set(void* src, size_t size) { static_cast<T*>(this)->set(src, size); }

    static IndexBufferRef create(const Renderer* renderer) { return T::create(renderer); }

private:
    IndexBufferI() = default;
    friend T;
};

} // namespace chronicle