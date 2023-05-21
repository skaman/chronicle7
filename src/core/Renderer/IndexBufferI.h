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
    /// @brief Get the index buffer handle ID
    /// @return Index buffer ID
    [[nodiscard]] IndexBufferId indexBufferId() const { return static_cast<const T*>(this)->indexBufferId(); }

    /// @brief Factory for create a new index buffer.
    /// @param data Index buffer data.
    /// @param name Index buffer name.
    /// @return The index buffer.
    [[nodiscard]] static IndexBufferRef create(const std::vector<uint8_t>& data, const std::string& name)
    {
        return T::create(data, name);
    }

    /// @brief Factory for create a new index buffer.
    /// @param src Pointer to memory data location.
    /// @param size Size of the data to set into the index buffer.
    /// @param name Index buffer name.
    /// @return The index buffer.
    [[nodiscard]] static IndexBufferRef create(const uint8_t* src, size_t size, const std::string& name)
    {
        return T::create(src, size, name);
    }

private:
    IndexBufferI() = default;
    friend T;
};

} // namespace chronicle