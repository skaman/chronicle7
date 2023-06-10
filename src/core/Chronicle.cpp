// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "Chronicle.h"

#include "Common/Log.h"
#include "Graphics/System.h"
#include "Platform/Platform.h"

using namespace magic_enum::bitwise_operators;

namespace chronicle
{

void Chronicle::init()
{
    platform::Platform::init();
    graphics::System::init({.enableDebug = true});

    auto device = graphics::System::requestDevice({.hwnd = platform::Platform::defaultWindow().hwnd()});
    auto commandBuffer = device->createCommandEncoder();
    auto buffer = device->createBuffer(graphics::BufferDescriptor(
        "Buffer 1", graphics::BufferUsageFlags::eMapWrite | graphics::BufferUsageFlags::eCopySrc, 1024));
    auto buffer2 = device->createBuffer(graphics::BufferDescriptor(
        "Buffer 2", graphics::BufferUsageFlags::eCopyDst | graphics::BufferUsageFlags::eIndex, 1024));

    platform::Platform::defaultWindow().sink<platform::WindowCloseEvent>().connect<&Chronicle::onWindowClose>();
}

void Chronicle::deinit()
{
    platform::Platform::defaultWindow().sink<platform::WindowCloseEvent>().disconnect<&Chronicle::onWindowClose>();
    graphics::System::deinit();
    platform::Platform::deinit();
}

void Chronicle::run()
{
    while (_isRunning)
    {
        platform::Platform::poll();
    }
}

void Chronicle::onWindowClose([[maybe_unused]] const platform::WindowCloseEvent &evn)
{
    _isRunning = false;
}

} // namespace chronicle