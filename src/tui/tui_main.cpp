#include "display.hpp"
#include "history.hpp"
#include "input.hpp"
#include "launch.hpp"

#include "../game_logger.hpp"
#include "../game_setups.hpp"
#include "../game_state.hpp"
#include "../move_application.hpp"
#include "../move_generation.hpp"
#include "../random_agent.hpp"
#include "../terminal_and_scoring.hpp"

#include <cstdio>
#include <cstring>
#include <sstream>
#include <unistd.h>
#include <variant>

using namespace patchwork;
using namespace patchwork::tui;

int main(int argc, char** argv) {
    // Parse --no-color flag.
    bool no_color = false;
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--no-color") == 0) no_color = true;
    }

    // Only run in a TTY.
    if (!isatty(STDIN_FILENO)) {
        std::fprintf(stderr, "patchwork-tui requires an interactive terminal.\n");
        return 1;
    }

    // Initialise display (also enforces min terminal size).
    DisplayConfig cfg = init_display(no_color, argc, argv);

    // Launch screen (cooked mode).
    LaunchConfig launch = run_launch_screen();

    // Build game.
    GameSetup setup = make_setup(launch.setup_index);
    SimplifiedGameState initial_state{};
    RngState rng(static_cast<std::mt19937::result_type>(launch.seed));

    // Build history with initial state and RNG.
    History history(initial_state, rng);

    // Log state.
    LogState log;
    NdjsonState ndjson;

    // Log game-start to ndjson.
    {
        std::ostringstream oss;
        log_game_start(oss, static_cast<long long>(launch.seed),
                       launch.setup_index, initial_state);
        append_ndjson(ndjson, oss.str());
    }

    // Enter raw mode for game loop.
    RawMode raw_mode;

    // Compute max ndjson lines = terminal height - fixed frame rows.
    auto max_ndjson = [&]() -> int {
        return std::max(0, cfg.height - 18);
    };

    int ply = 0;
    std::string last_error;

    for (;;) {
        SimplifiedGameState state = history.current_state();
        render_frame(state, setup, log, ndjson, cfg, last_error);
        last_error.clear();

        if (is_terminal(state)) break;

        Command cmd = read_command();

        // Handle non-game commands first.
        if (std::holds_alternative<QuitCmd>(cmd)) break;

        if (std::holds_alternative<UndoCmd>(cmd)) {
            history.undo();
            append_log(log, "Undo");
            continue;
        }
        if (std::holds_alternative<RedoCmd>(cmd)) {
            history.redo();
            append_log(log, "Redo");
            continue;
        }
        if (std::holds_alternative<ScrollLogLeft>(cmd)) {
            if (log.scroll_offset > 0) --log.scroll_offset;
            continue;
        }
        if (std::holds_alternative<ScrollLogRight>(cmd)) {
            ++log.scroll_offset;
            continue;
        }
        if (std::holds_alternative<ToggleLogWrap>(cmd)) {
            log.wrap_mode = !log.wrap_mode;
            continue;
        }
        if (std::holds_alternative<NdjsonToggleMinimize>(cmd)) {
            ndjson_toggle_minimize(ndjson, max_ndjson());
            continue;
        }
        if (std::holds_alternative<NdjsonMaximize>(cmd)) {
            ndjson_maximize(ndjson, max_ndjson());
            continue;
        }
        if (std::holds_alternative<NdjsonSemiMaximize>(cmd)) {
            ndjson_semi_maximize(ndjson, max_ndjson());
            continue;
        }
        if (std::holds_alternative<NdjsonDecrLines>(cmd)) {
            ndjson_decr_lines(ndjson);
            continue;
        }
        if (std::holds_alternative<NdjsonIncrLines>(cmd)) {
            ndjson_incr_lines(ndjson, max_ndjson());
            continue;
        }

        // Legality check — silently ignore illegal commands.
        if (!is_legal(cmd, state, setup)) {
            continue;
        }

        // Build move.
        Move move = Advance{};
        if (std::holds_alternative<BuyPatchCmd>(cmd)) {
            move = BuyPatch{std::get<BuyPatchCmd>(cmd).index};
        }

        // Snapshot RNG before opponent move.
        RngState rng_before = history.current_rng();

        // Apply player move.
        SimplifiedGameState new_state = apply_move(state, move, setup);

        // Log move.
        {
            std::ostringstream oss;
            log_move(oss, ply, state.active_player(), move, new_state);
            std::string line = oss.str();
            if (!line.empty() && line.back() == '\n') line.pop_back();
            append_ndjson(ndjson, line);
            // Human-readable event log.
            if (std::holds_alternative<BuyPatch>(move)) {
                int idx = std::get<BuyPatch>(move).patch_index;
                char buf[60];
                std::snprintf(buf, sizeof(buf), "P%d bought [%d]",
                              state.active_player() + 1, idx);
                append_log(log, buf);
            } else {
                char buf[40];
                std::snprintf(buf, sizeof(buf), "P%d advanced",
                              state.active_player() + 1);
                append_log(log, buf);
            }
        }
        ++ply;

        // If game not terminal, let opponent move (random agent).
        if (!is_terminal(new_state)) {
            // Restore RNG from before this player's move for determinism on redo.
            rng = rng_before;
            Move opp_move = random_move(new_state, setup, rng);
            SimplifiedGameState after_opp = apply_move(new_state, opp_move, setup);
            {
                std::ostringstream oss;
                log_move(oss, ply, new_state.active_player(), opp_move, after_opp);
                std::string line = oss.str();
                if (!line.empty() && line.back() == '\n') line.pop_back();
                append_ndjson(ndjson, line);
                if (std::holds_alternative<BuyPatch>(opp_move)) {
                    int idx = std::get<BuyPatch>(opp_move).patch_index;
                    char buf[60];
                    std::snprintf(buf, sizeof(buf), "P%d (cpu) bought [%d]",
                                  new_state.active_player() + 1, idx);
                    append_log(log, buf);
                } else {
                    char buf[40];
                    std::snprintf(buf, sizeof(buf), "P%d (cpu) advanced",
                                  new_state.active_player() + 1);
                    append_log(log, buf);
                }
            }
            ++ply;
            new_state = after_opp;
        }

        // Push new state + RNG snapshot (before opponent move) to history.
        history.push(new_state, rng);
    }

    // Result summary.
    SimplifiedGameState final_state = history.current_state();
    {
        std::ostringstream oss;
        log_game_end(oss, final_state);
        std::string line = oss.str();
        if (!line.empty() && line.back() == '\n') line.pop_back();
        append_ndjson(ndjson, line);
    }

    // Re-render final state.
    render_frame(final_state, setup, log, ndjson, cfg);

    // Print result summary below frame.
    int s0 = score(final_state, 0);
    int s1 = score(final_state, 1);
    int w  = winner(final_state);
    bool bonus0 = (final_state.bonus_status() == BonusStatus::kPlayer0);
    bool bonus1 = (final_state.bonus_status() == BonusStatus::kPlayer1);
    std::printf("\n  Game over!\n");
    std::printf("  P1 score: %d%s\n", s0, bonus0 ? " (+7 bonus)" : "");
    std::printf("  P2 score: %d%s\n", s1, bonus1 ? " (+7 bonus)" : "");
    if (w == 0) std::printf("  Winner: P1\n\n");
    else        std::printf("  Winner: P2\n\n");

    return 0;
}
