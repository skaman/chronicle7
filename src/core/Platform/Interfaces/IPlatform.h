// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "Platform/Window.h"

namespace chronicle::platform
{

template <class T> class IPlatform
{
  public:
    static void init()
    {
        return T::init();
    }

    static void deinit()
    {
        return T::deinit();
    }

    static void poll()
    {
        return T::poll();
    }

    static Window &defaultWindow()
    {
        return T::defaultWindow();
    }

  private:
    IPlatform() = default;
    friend T;
};

} // namespace chronicle::platform