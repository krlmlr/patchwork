#pragma once
#include <cstdint>

#include "../agent_strategy.hpp"

namespace patchwork::tui {

/// Configuration returned by the launch screen.
struct LaunchConfig {
    int setup_index{0};
    std::uint64_t seed{42};
    AgentStrategy strategy{AgentStrategy::Random};
};

/// Show the launch screen (cooked-mode prompts), validate input, and return
/// the configuration. Loops on invalid input.
LaunchConfig run_launch_screen();

}  // namespace patchwork::tui
