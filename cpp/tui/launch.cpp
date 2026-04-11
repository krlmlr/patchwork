#include "launch.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

namespace patchwork::tui {

LaunchConfig run_launch_screen() {
    std::printf("\n");
    std::printf("  ╔══════════════════════════════╗\n");
    std::printf("  ║   PATCHWORK — new game       ║\n");
    std::printf("  ╚══════════════════════════════╝\n\n");

    LaunchConfig cfg;

    // Setup index.
    for (;;) {
        std::printf("  Setup index (0–99, default 0): ");
        std::fflush(stdout);
        std::string line;
        if (!std::getline(std::cin, line)) break;
        if (line.empty()) {
            cfg.setup_index = 0;
            break;
        }
        try {
            int v = std::stoi(line);
            if (v >= 0 && v <= 99) {
                cfg.setup_index = v;
                break;
            }
        } catch (...) {
        }
        std::printf("  Invalid input — enter a number 0–99.\n");
    }

    // Seed.
    for (;;) {
        std::printf("  Random seed (default 42): ");
        std::fflush(stdout);
        std::string line;
        if (!std::getline(std::cin, line)) break;
        if (line.empty()) {
            cfg.seed = 42;
            break;
        }
        try {
            unsigned long long v = std::stoull(line);
            cfg.seed = static_cast<std::uint64_t>(v);
            break;
        } catch (...) {
        }
        std::printf("  Invalid input — enter a non-negative integer.\n");
    }

    std::printf("\n  Starting game: setup %d, seed %llu\n\n", cfg.setup_index,
                static_cast<unsigned long long>(cfg.seed));
    return cfg;
}

}  // namespace patchwork::tui
