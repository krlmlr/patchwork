#ifndef PATCHWORK_GAME_LOGGER_HPP
#define PATCHWORK_GAME_LOGGER_HPP

#include <ostream>

#include "game_setup.hpp"
#include "move.hpp"
#include "simplified_game_state.hpp"

namespace patchwork {

/// Write a game-start NDJSON line: {"event":"game_start", ...}
void log_game_start(std::ostream& out, long long seed, int setup_id,
                    const SimplifiedGameState& state, const GameSetup& setup);

/// Write a move NDJSON line: {"event":"move", ...}
void log_move(std::ostream& out, int ply, int player, const Move& move,
              const SimplifiedGameState& new_state, const GameSetup& setup);

/// Write a game-end NDJSON line: {"event":"game_end", ...}
void log_game_end(std::ostream& out, const SimplifiedGameState& state);

}  // namespace patchwork

#endif  // PATCHWORK_GAME_LOGGER_HPP
