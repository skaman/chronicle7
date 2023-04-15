// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Common.h"
#include "TextureInfo.h"

namespace chronicle {

template <class T> class TextureI {
public:
    void set(void* src, size_t size, uint32_t width, uint32_t height)
    {
        static_cast<T*>(this)->set(src, size, width, height);
    }

    [[nodiscard]] static TextureRef create(const TextureInfo& textureInfo) { return T::create(textureInfo); }

private:
    TextureI() = default;
    friend T;
};

} // namespace chronicle