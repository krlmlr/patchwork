#ifndef PATCHWORK_GAME_LOGGER_HPP
#define PATCHWORK_GAME_LOGGER_HPP

#include <ostream>
#include <string_view>

#include "game_setup.hpp"
#include "move.hpp"
#include "simplified_game_state.hpp"

namespace patchwork {

/// Write a game-start NDJSON line: {"event":"game_start", ...}
///
/// Records the setup, the initial per-player summaries, the 33-character
/// `circle`, and the per-player agent strategies and RNG seeds plus the
/// advance-weight used. There is no single `seed` field: with two independent
/// RNG streams the per-player `seed_p0` / `seed_p1` fields replace it.
void log_game_start(std::ostream& out, int setup_id, const SimplifiedGameState& state,
                    const GameSetup& setup, std::string_view agent_p0, std::string_view agent_p1,
                    long long seed_p0, long long seed_p1, double advance_weight);

/// Write a move NDJSON line: {"event":"move", ...}
void log_move(std::ostream& out, int ply, int player, const Move& move,
              const SimplifiedGameState& new_state, const GameSetup& setup);

/// Write a game-end NDJSON line: {"event":"game_end", ...}
void log_game_end(std::ostream& out, const SimplifiedGameState& state);

}  // namespace patchwork

#endif  // PATCHWORK_GAME_LOGGER_HPP
