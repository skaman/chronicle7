#pragma once

#include "pch.h"

template <class T> struct Offset2D {
    Offset2D(T x = {}, T y = {})
        : x(x)
        , y(y)
    {
    }

    T x;
    T y;
};

using OffsetInt2D = Offset2D<int32_t>;
using OffsetFloat2D = Offset2D<float>;

template <class T> struct Extent2D {
    Extent2D(T width = {}, T height = {})
        : width(width)
        , height(height)
    {
    }

    T width;
    T height;
};

using ExtentInt2D = Extent2D<int32_t>;
using ExtentFloat2D = Extent2D<float>;

template <class T> struct Rect2D {
    Rect2D(Offset2D<T> offset = {}, Extent2D<T> extent = {})
        : offset(offset)
        , extent(extent)
    {
    }

    Offset2D<T> offset;
    Extent2D<T> extent;
};

using RectInt2D = Rect2D<int32_t>;
using RectFloat2D = Rect2D<float>;

enum class ShaderStage { Fragment, Vertex };
enum class Format { Undefined, B8G8R8A8Unorm, R32G32Sfloat, R32G32B32Sfloat };
enum class VertexInputRate { Vertex, Instance };
// enum class DescriptorType { UniformBuffer };