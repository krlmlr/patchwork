#ifndef PATCHWORK_AGENT_HPP
#define PATCHWORK_AGENT_HPP

#include <random>

#include "agent_strategy.hpp"
#include "biased_random_agent.hpp"
#include "game_setup.hpp"
#include "move.hpp"
#include "random_agent.hpp"
#include "simplified_game_state.hpp"

namespace patchwork {

/// Select a move for the active player using the named strategy.
///
/// `AgentStrategy::Random` delegates to `random_move` (uniform selection) and
/// ignores `advance_weight`; every other strategy delegates to
/// `biased_random_move` with the strategy's weight function and the given
/// `advance_weight`.
///
/// Precondition: `state` must not be terminal.
[[nodiscard]] inline Move select_move(const SimplifiedGameState& state, const GameSetup& setup,
                                      std::mt19937& rng, AgentStrategy strategy,
                                      double advance_weight) {
    if (strategy == AgentStrategy::Random) {
        return random_move(state, setup, rng);
    }
    return biased_random_move(state, setup, rng, make_weight_fn(strategy), advance_weight);
}

}  // namespace patchwork

#endif  // PATCHWORK_AGENT_HPP
