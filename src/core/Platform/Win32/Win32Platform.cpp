// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "Win32Platform.h"

#include <entt/entt.hpp>

namespace chronicle::platform::internal::win32
{

void Win32Platform::init()
{
    _window = Win32Window::create({});
}

void Win32Platform::deinit()
{
    _window.reset();
}

void Win32Platform::poll()
{
    WaitForInputIdle(GetCurrentProcess(), 16);

    MSG msg{.message = WM_NULL};

    while (PeekMessageW(&msg, nullptr, 0U, 0U, PM_REMOVE) != 0)
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    _window->publishEvents();
}

} // namespace chronicle::internal::win32