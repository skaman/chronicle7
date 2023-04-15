// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Common.h"
#include "ImageInfo.h"

namespace chronicle {

template <class T> class ImageI {
public:
    void set(void* src, size_t size, uint32_t width, uint32_t height)
    {
        static_cast<T*>(this)->set(src, size, width, height);
    }

    [[nodiscard]] static ImageRef createTexture(const ImageInfo& imageInfo) { return T::createTexture(imageInfo); }

private:
    ImageI() = default;
    friend T;
};

} // namespace chronicle