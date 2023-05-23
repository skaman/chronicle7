// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "MainWindow.h"

#include "Resources.h"

#include <Platform/Platform.h>
#include <Renderer/Renderer.h>
#include <Storage/Storage.h>

namespace chronicle::editor {

MainWindow::MainWindow()
{
    Storage::init();
    Platform::init();
    Renderer::init();

    registerResources();
}

MainWindow::~MainWindow()
{
    Renderer::waitIdle();

    Renderer::deinit();
    Platform::deinit();
}

void MainWindow::run()
{
    double delta;

    while (Platform::poll(delta)) {
        if (!Renderer::beginFrame())
            continue;
        Renderer::beginRenderPass();

        DockArea::draw([]() {
            /*
            if (ImGui::BeginMenuBar()) {
                if (ImGui::BeginMenu("Options")) {
                    // Disabling fullscreen would allow the window to be moved to the front of other windows,
                    // which we can't undo at the moment without finer window depth/z control.
                    ImGui::Separator();

                    if (ImGui::MenuItem("Flag: NoSplit", "", true)) { }
                    ImGui::EndMenu();
                }

                ImGui::EndMenuBar();
            }
            */

            ImGui::ShowDemoWindow();
        });

        Renderer::endRenderPass();
        Renderer::endFrame();
    }
}

void MainWindow::registerResources()
{
    Storage::registerFileData("WindowClose.png", WindowClose_png, WindowClose_png_len);
    Storage::registerFileData("WindowRestore.png", WindowRestore_png, WindowRestore_png_len);
    Storage::registerFileData("WindowMaximize.png", WindowMaximize_png, WindowMaximize_png_len);
    Storage::registerFileData("WindowMinimize.png", WindowMinimize_png, WindowMinimize_png_len);
}

} // namespace chronicle::editor