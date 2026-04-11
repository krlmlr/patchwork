#ifndef PATCHWORK_RANDOM_AGENT_HPP
#define PATCHWORK_RANDOM_AGENT_HPP

#include <random>

#include "game_setup.hpp"
#include "move.hpp"
#include "simplified_game_state.hpp"

namespace patchwork {

/// Select a legal move uniformly at random using `rng`.
/// Precondition: `state` must not be terminal.
[[nodiscard]] Move random_move(const SimplifiedGameState& state, const GameSetup& setup,
                               std::mt19937& rng);

}  // namespace patchwork

#endif  // PATCHWORK_RANDOM_AGENT_HPP
