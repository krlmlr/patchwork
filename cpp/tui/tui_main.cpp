#include <unistd.h>

#include <cstdio>
#include <cstring>
#include <sstream>
#include <variant>

#include "../agent.hpp"
#include "../agent_strategy.hpp"
#include "../game_logger.hpp"
#include "../game_setups.hpp"
#include "../game_state.hpp"
#include "../generated/game_setups.hpp"
#include "../generated/patches.hpp"
#include "../move_application.hpp"
#include "../move_generation.hpp"
#include "../terminal_and_scoring.hpp"
#include "display.hpp"
#include "history.hpp"
#include "input.hpp"
#include "launch.hpp"

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

    // Advance-weight for the opponent agent's biased sampling (the TUI uses the
    // default; there is no launch prompt to tune it).
    constexpr double kAdvanceWeight = 1.0;

    // Show the opponent strategy name in the frame header.
    cfg.opponent_strategy = std::string(strategy_name(launch.strategy));

    // Build game.
    GameSetup setup = GameSetup(kGameSetups[static_cast<std::size_t>(
        launch.setup_index % static_cast<int>(kNumGameSetups))]);
    SimplifiedGameState initial_state{};
    // Player 0 is the human (its RNG stream is carried but unused); player 1 is
    // the opponent agent seeded with the launch seed.
    RngState initial_rng_p0(static_cast<std::mt19937::result_type>(launch.seed));
    RngState initial_rng_p1(static_cast<std::mt19937::result_type>(launch.seed));

    // Build history with initial state and both RNG streams.
    History history(initial_state, initial_rng_p0, initial_rng_p1);

    // Log state.
    LogState log;
    NdjsonState ndjson;

    // Log game-start to ndjson.
    {
        std::ostringstream oss;
        log_game_start(oss, launch.setup_index, initial_state, setup, "human",
                       strategy_name(launch.strategy), static_cast<long long>(launch.seed),
                       static_cast<long long>(launch.seed), kAdvanceWeight);
        append_ndjson(ndjson, oss.str());
    }

    // Enter raw mode for game loop.
    RawMode raw_mode;

    // Compute max ndjson lines = terminal height minus fixed frame rows.
    // Narrow: top(1)+circle(2)+detail_min(3)+stats-sep(1)+stats(1)+quilt-sep(1)+
    //         quilt-header(1)+quilt-rows(9)+ndjson-sep(1)+bottom(1) = 21 + ndjson
    // Target: cfg.height - 1 (fill terminal minus last line)
    // max_ndjson_narrow = (cfg.height - 1) - 21 = cfg.height - 22
    // Wide: top(1)+body(10)+ndjson-sep(1)+bottom(1) = 13 + ndjson
    // max_ndjson_wide   = (cfg.height - 1) - 13 = cfg.height - 14
    auto max_ndjson = [&]() -> int {
        if (cfg.width >= 160) return std::max(0, cfg.height - 14);
        return std::max(0, cfg.height - 22);
    };

    // Start with the frame filling the entire terminal.
    ndjson.height = max_ndjson();

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
            // Restore event-log to the snapshot stored in the history entry.
            log.entries = history.current_log_entries();
            continue;
        }
        if (std::holds_alternative<RedoCmd>(cmd)) {
            history.redo();
            // Restore event-log to the snapshot stored in the history entry.
            log.entries = history.current_log_entries();
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

        // Build move.  BuyPatchCmd.index is the sequential position (0-based)
        // within the legal BuyPatch moves; translate to the actual patch_id.
        Move move = Advance{};
        if (std::holds_alternative<BuyPatchCmd>(cmd)) {
            int seq = std::get<BuyPatchCmd>(cmd).index;
            auto legal = legal_moves(state, setup);
            int bc = 0;
            for (auto& m : legal) {
                if (std::holds_alternative<BuyPatch>(m)) {
                    if (bc == seq) {
                        move = m;
                        break;
                    }
                    ++bc;
                }
            }
        }

        // Snapshot both RNG streams from the current history entry. Player 0
        // (human) never consumes its stream; player 1 (opponent) does.
        RngState rng_p0 = history.current_rng_p0();
        RngState rng_p1 = history.current_rng_p1();

        // Apply player move.
        SimplifiedGameState new_state = apply_move(state, move, setup);

        // Log move.  Patch names (single char) give consistent-width entries.
        {
            std::ostringstream oss;
            log_move(oss, ply, state.active_player(), move, new_state, setup);
            std::string line = oss.str();
            if (!line.empty() && line.back() == '\n') line.pop_back();
            append_ndjson(ndjson, line);
            // Human-readable event log uses patch name char to avoid width variation.
            if (std::holds_alternative<BuyPatch>(move)) {
                int pid = std::get<BuyPatch>(move).patch_index;
                char pname = kPatches[static_cast<std::size_t>(pid)].name;
                char buf[60];
                std::snprintf(buf, sizeof(buf), "P%d bought [%c]", state.active_player() + 1,
                              pname);
                append_log(log, buf);
            } else {
                char buf[40];
                std::snprintf(buf, sizeof(buf), "P%d advanced", state.active_player() + 1);
                append_log(log, buf);
            }
        }
        ++ply;

        // Let CPU take all its consecutive turns (may be multiple if human advanced
        // far ahead on the time track).  The opponent's RNG stream (rng_p1) was
        // snapshotted from the history entry above, so redo reproduces the
        // identical CPU moves deterministically.
        if (!is_terminal(new_state)) {
            while (!is_terminal(new_state) && new_state.active_player() != 0) {
                Move opp_move =
                    select_move(new_state, setup, rng_p1, launch.strategy, kAdvanceWeight);
                SimplifiedGameState after_opp = apply_move(new_state, opp_move, setup);
                {
                    std::ostringstream oss;
                    log_move(oss, ply, new_state.active_player(), opp_move, after_opp, setup);
                    std::string line = oss.str();
                    if (!line.empty() && line.back() == '\n') line.pop_back();
                    append_ndjson(ndjson, line);
                    if (std::holds_alternative<BuyPatch>(opp_move)) {
                        int pid = std::get<BuyPatch>(opp_move).patch_index;
                        char pname = kPatches[static_cast<std::size_t>(pid)].name;
                        char buf[60];
                        std::snprintf(buf, sizeof(buf), "P%d (cpu) bought [%c]",
                                      new_state.active_player() + 1, pname);
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
        }

        // Push new state + both RNG snapshots + log snapshot to history.
        history.push(new_state, rng_p0, rng_p1, log.entries);
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
    int w = winner(final_state);
    bool bonus0 = (final_state.bonus_status() == BonusStatus::kPlayer0);
    bool bonus1 = (final_state.bonus_status() == BonusStatus::kPlayer1);
    std::printf("\n  Game over!\n");
    std::printf("  P1 score: %d%s\n", s0, bonus0 ? " (+7 bonus)" : "");
    std::printf("  P2 score: %d%s\n", s1, bonus1 ? " (+7 bonus)" : "");
    if (w == 0)
        std::printf("  Winner: P1\n\n");
    else
        std::printf("  Winner: P2\n\n");

    return 0;
}
