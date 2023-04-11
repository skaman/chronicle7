#pragma once

#include "pch.h"

#include "Common.h"

namespace chronicle {

template <class T> class VertexBufferI {
public:
    void set(void* src, size_t size) { static_cast<T*>(this)->set(src, size); }

    static VertexBufferRef create() { return T::create(); }

private:
    VertexBufferI() = default;
    friend T;
};

} // namespace chronicle