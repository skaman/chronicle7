#include <MainWindow.h>

int main(int argc, char** argv)
{
    spdlog::set_level(spdlog::level::debug);
    try {
        chronicle::editor::MainWindow window {};
        window.run();
    } catch (const std::exception& e) {
        CHRLOG_ERROR("Unhandled exception: {}", e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}