#ifndef PATCHWORK_GAME_LOOP_HPP
#define PATCHWORK_GAME_LOOP_HPP

#include <ostream>

#include "game_setup.hpp"
#include "simplified_game_state.hpp"

namespace patchwork {

/// Result of playing a single game to termination.
struct GameOutcome {
    SimplifiedGameState state;  ///< terminal state (scores/winner derivable)
    int plies;                  ///< number of moves applied
};

/// Play a full game between two uniform-random agents on `setup`, seeded by
/// `seed`. When `full_log` is non-null, the complete NDJSON stream
/// (`game_start`, one `move` line per ply, `game_end`) is written to it,
/// byte-for-byte identical to the standalone play driver. `setup_id` is used
/// only for the `game_start` record. Returns the terminal state and ply count.
GameOutcome play_game(const GameSetup& setup, int setup_id, long long seed,
                      std::ostream* full_log = nullptr);

}  // namespace patchwork

#endif  // PATCHWORK_GAME_LOOP_HPP
