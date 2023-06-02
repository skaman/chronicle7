// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "Chronicle.h"

#include "Common/Log.h"
#include "Graphics/System.h"
#include "Platform/Platform.h"

namespace chronicle
{

void Chronicle::init()
{
    CHRLOG_INFO("{}", 12);

    platform::Platform::init();
    graphics::System::init({.enableDebug = true});
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