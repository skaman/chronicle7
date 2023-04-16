// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "Platform/Common.h"

namespace chronicle {

struct GLFWContext {
    static inline GLFWwindow* window = nullptr;
    static inline std::string title = "Chronicle";
    static inline int width = 1024;
    static inline int height = 768;
    static inline entt::dispatcher dispatcher = {};
    static inline GLFWcursor* cursor = nullptr;
    static inline CursorType cursorType = CursorType::system;
};

} // namespace chronicle