// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

namespace chronicle::editor {

class DockPanel {
public:
    static void draw(std::function<void()> contentCallback);
};

} // namespace chronicle::editor