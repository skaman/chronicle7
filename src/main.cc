#include <spdlog/spdlog.h>

#include "Platform/App.h"

int main()
{
    // chronicle::HelloTriangleApplication app;
    chronicle::App app;

    try {
        app.Run("Test app");
    } catch (const std::exception& e) {
        spdlog::error(e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}