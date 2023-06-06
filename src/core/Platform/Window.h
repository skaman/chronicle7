// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#ifdef CHR_PLATFORM_WIN32
#include "Win32/Win32Window.h"
#endif // CHR_PLATFORM_WIN32

namespace chronicle::platform
{

#ifdef CHR_PLATFORM_WIN32
using Window = IWindow<internal::win32::Win32Window>;
#endif // CHR_PLATFORM_WIN32

} // namespace chronicle::platform