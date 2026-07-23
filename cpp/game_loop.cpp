#include "game_loop.hpp"

#include <random>

#include "game_logger.hpp"
#include "move.hpp"
#include "move_application.hpp"
#include "move_generation.hpp"
#include "random_agent.hpp"
#include "terminal_and_scoring.hpp"

namespace patchwork {

GameOutcome play_game(const GameSetup& setup, int setup_id, long long seed,
                      std::ostream* full_log) {
    std::mt19937 rng(static_cast<unsigned>(seed));
    SimplifiedGameState state;

    if (full_log != nullptr) {
        log_game_start(*full_log, seed, setup_id, state, setup);
    }

    int ply = 0;
    while (!is_terminal(state)) {
        int player = state.active_player();
        Move mv = random_move(state, setup, rng);
        state = apply_move(state, mv, setup);
        if (full_log != nullptr) {
            log_move(*full_log, ply, player, mv, state, setup);
        }
        ++ply;
    }

    if (full_log != nullptr) {
        log_game_end(*full_log, state);
    }

    return GameOutcome{state, ply};
}

}  // namespace patchwork
