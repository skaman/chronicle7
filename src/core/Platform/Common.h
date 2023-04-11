#pragma once

namespace chronicle {

template <class T> class PlatformI;

#ifdef GLFW_PLATFORM
class GLFWPlatform;

using Platform = PlatformI<GLFWPlatform>;
#endif

} // namespace chronicle