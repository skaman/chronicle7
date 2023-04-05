#include "pch.h"

#include "Platform/App.h"

int main()
{
    chronicle::App app;

    try {
        app.Run("Test app");
    } catch (const std::exception& e) {
        CHRLOG_ERROR("Unhandled exception: {}", e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}