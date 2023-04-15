// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Common.h"

namespace chronicle {

template <class T> class PlatformI {
public:
    static void init() { T::init(); }
    static void deinit() { T::deinit(); }
    static bool poll() { return T::poll(); }

    static void setWindowTitle(const std::string_view& title) { T::setWindowTitle(title); }
    static void setWindowSize(uint32_t width, uint32_t height) { T::setWindowSize(width, height); }

    [[nodiscard]] static std::pair<uint32_t, uint32_t> windowSize() { return T::windowSize(); }
    [[nodiscard]] static std::pair<uint32_t, uint32_t> frameBufferSize() { return T::frameBufferSize(); }

    [[nodiscard]] static void* hwnd() { return T::hwnd(); }
    [[nodiscard]] static void* hinstace() { return T::hinstace(); }

private:
    PlatformI() = default;
    friend T;
};

} // namespace chronicle