// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Common/Common.h"
#include "Data/RenderPassInfo.h"

namespace chronicle {

/// @brief Object used to handle a render pass.
/// @tparam T Type with implementation.
template <class T> class BaseRenderPass {
public:
    /// @brief Get the render pass handle ID
    /// @return Render pass ID
    [[nodiscard]] RenderPassId renderPassId() const { return CRTP_CONST_THIS->renderPassId(); }

    /// @brief Get the hash of the configuration used to create the render pass.
    /// @return Render pass hash.
    [[nodiscard]] size_t hash() const { return CRTP_CONST_THIS->hash(); };

    /// @brief Get the format used by the color attachment.
    /// @return Format.
    [[nodiscard]] Format format() const { return CRTP_CONST_THIS->format(); };

    /// @brief Get the multi sampling used by the color attachment.
    /// @return Multi sampling.
    [[nodiscard]] MSAA msaa() const { return CRTP_CONST_THIS->msaa(); };

    /// @brief Factory for create a new render pass.
    /// @param renderPassInfo Informations used to create the render pass.
    /// @param name Render pass name.
    /// @return The render pass.
    [[nodiscard]] static RenderPassRef create(const RenderPassInfo& renderPassInfo, const std::string& name)
    {
        return T::create(renderPassInfo, name);
    }

private:
    BaseRenderPass() = default;
    friend T;
};

} // namespace chronicle