// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

namespace chronicle {

struct GLFWContext {
    static inline GLFWwindow* window = nullptr;
    static inline std::string title = "Chronicle";
    static inline int width = 1024;
    static inline int height = 768;
};

} // namespace chronicle