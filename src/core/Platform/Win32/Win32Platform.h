// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "Common/Common.h"
#include "Platform/Interfaces/IPlatform.h"

namespace chronicle::platform::internal::win32
{

class Win32Platform : public IPlatform<Win32Platform>, private NonCopyable<Win32Platform>
{
  public:
    static void init();
    static void deinit();
    static void poll();

    static Window &defaultWindow()
    {
        return *_window.get();
    }

  private:
    static inline std::shared_ptr<Win32Window> _window{};
};

} // namespace chronicle::platform::internal::win32