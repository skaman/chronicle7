// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Common.h"

namespace chronicle {

/// @brief Object used to handle an index buffer.
/// @tparam T Type with implementation.
template <class T> class IndexBufferI {
public:
    /// @brief Set data into the index buffer. This method can be called multiple times if there's need to update the
    ///        data into the index buffer.
    ///        A new buffer is allocated into the GPU and data is copied. There's no need to keep data in memory once is
    ///        set into the index buffer.
    /// @param src Pointer to memory data location.
    /// @param size Size of the data to set into the index buffer.
    /// @param debugName Debug name.
    void set(void* src, size_t size, const char* debugName = nullptr)
    {
        static_cast<T*>(this)->set(src, size, debugName);
    }

    /// @brief Factory for create a new index buffer.
    /// @return The index buffer.
    [[nodiscard]] static IndexBufferRef create() { return T::create(); }

private:
    IndexBufferI() = default;
    friend T;
};

} // namespace chronicle