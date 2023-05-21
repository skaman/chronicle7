// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Widgets/DockArea.h"

namespace chronicle::editor {

class MainWindow {
public:
    explicit MainWindow();
    ~MainWindow();

    void run();
    void registerResources();
};

} // namespace chronicle::editor