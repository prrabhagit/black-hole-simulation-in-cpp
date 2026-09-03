#include "Application.hpp"
#include <cstdio>
#include <exception>

int main() {
    try {
        bh::app::Application app;
        app.run();
    } catch (const std::exception& e) {
        std::fprintf(stderr, "Fatal error: %s\n", e.what());
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
