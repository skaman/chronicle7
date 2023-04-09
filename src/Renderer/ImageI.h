#pragma once

#include "pch.h"

#include "Common.h"
#include "ImageInfo.h"

namespace chronicle {

template <class T> class ImageI {
public:
    static ImageRef createTexture(const Renderer* renderer, const ImageInfo& imageInfo)
    {
        return T::createTexture(renderer, imageInfo);
    }

private:
    ImageI() = default;
    friend T;
};

} // namespace chronicle