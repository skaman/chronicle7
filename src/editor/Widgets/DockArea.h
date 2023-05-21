// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

namespace chronicle::editor {

class DockArea {
public:
    static void draw(std::function<void()> contentCallback);

private:
    static void drawTitleBar();
};

} // namespace chronicle::editor