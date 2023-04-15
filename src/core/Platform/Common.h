// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

namespace chronicle {

template <class T> class PlatformI;

#ifdef GLFW_PLATFORM
class GLFWPlatform;

using Platform = PlatformI<GLFWPlatform>;
#endif

} // namespace chronicle