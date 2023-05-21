// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Common.h"

namespace chronicle {

/// @brief Object used to handle a vertex buffer.
/// @tparam T Type with implementation.
template <class T> class VertexBufferI {
public:
    /// @brief Get the vertex buffer handle ID
    /// @return Vertex buffer ID
    [[nodiscard]] VertexBufferId vertexBufferId() const { return CRTP_CONST_THIS->vertexBufferId(); }

    /// @brief Factory for create a new vertex buffer.
    /// @param data Vertex buffer data.
    /// @param name Vertex buffer name.
    /// @return The vertex buffer.
    [[nodiscard]] static VertexBufferRef create(const std::vector<uint8_t>& data, const std::string& name)
    {
        return T::create(data, name);
    }

    /// @brief Factory for create a new vertex buffer.
    /// @param src Pointer to memory data location.
    /// @param size Size of the data to set into the vertex buffer.
    /// @param name Vertex buffer name.
    /// @return The vertex buffer.
    [[nodiscard]] static VertexBufferRef create(const uint8_t* src, size_t size, const std::string& name)
    {
        return T::create(src, size, name);
    }

private:
    VertexBufferI() = default;
    friend T;
};

} // namespace chronicle