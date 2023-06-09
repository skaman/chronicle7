// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Common/Common.h"
#include "Data/FrameBufferInfo.h"

namespace chronicle {

/// @brief Object used to handle a frame buffer.
/// @tparam T Type with implementation.
template <class T> class BaseFrameBuffer {
public:
    /// @brief Get the frame buffer handle ID
    /// @return Frame buffer ID
    [[nodiscard]] FrameBufferId frameBufferId() const { return CRTP_CONST_THIS->frameBufferId(); }

    /// @brief Factory for create a new frame buffer.
    /// @param frameBufferInfo Informations used to create the frame buffer.
    /// @param name Frame buffer name.
    /// @return The frame buffer.
    [[nodiscard]] static FrameBufferRef create(const FrameBufferInfo& frameBufferInfo, const std::string& name)
    {
        return T::create(frameBufferInfo, name);
    }

private:
    BaseFrameBuffer() = default;
    friend T;
};

} // namespace chronicle