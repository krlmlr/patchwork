// Snapshot tests for the TUI renderer.
//
// Each test renders a frame to a string and compares it against a golden file
// stored in tests/snapshots/.  When the rendered output differs from the
// stored snapshot the test fails and prints a unified diff to aid review.
//
// Updating snapshots
// ------------------
// Set the environment variable UPDATE_SNAPSHOTS=1 before running the test
// executable to overwrite all snapshot files with the current output:
//
//   UPDATE_SNAPSHOTS=1 ./build/tests/test_tui_snapshot
//
// The snapshot directory is resolved relative to the SNAPSHOT_DIR environment
// variable (set by meson) or, as a fallback, the directory of the test binary.

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <random>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

#include "tui/display.hpp"
#include "tui/history.hpp"
#include "simplified_game_state.hpp"
#include "game_setups.hpp"
#include "generated/game_setups.hpp"
#include "generated/patches.hpp"
#include "move_application.hpp"
#include "move_generation.hpp"
#include "random_agent.hpp"
#include "terminal_and_scoring.hpp"

namespace fs = std::filesystem;
using patchwork::SimplifiedGameState;
using patchwork::GameSetup;
using patchwork::BuyPatch;
using patchwork::Advance;
using patchwork::kGameSetups;
using patchwork::kNumGameSetups;
using patchwork::kPatches;
using patchwork::random_move;
using patchwork::apply_move;
using patchwork::legal_moves;
using patchwork::is_terminal;
using patchwork::tui::DisplayConfig;
using patchwork::tui::LogState;
using patchwork::tui::NdjsonState;
using patchwork::tui::History;
using patchwork::tui::RngState;
using patchwork::tui::append_log;
using patchwork::tui::append_ndjson;
using patchwork::tui::render_frame_to_string;

// ── Helpers ──────────────────────────────────────────────────────────────────

static DisplayConfig make_cfg(int width, bool color = false) {
    DisplayConfig cfg;
    cfg.width  = width;
    cfg.height = 24;
    cfg.color_enabled = color;
    return cfg;
}

static DisplayConfig make_cfg_h(int width, int height, bool color = false) {
    DisplayConfig cfg;
    cfg.width  = width;
    cfg.height = height;
    cfg.color_enabled = color;
    return cfg;
}

// Format a log entry for a buy move using the patch's single-char name.
// This matches what tui_main.cpp produces and avoids width variation.
static std::string log_entry_buy(int player_1idx, char patch_name) {
    char buf[60];
    std::snprintf(buf, sizeof(buf), "P%d bought [%c]", player_1idx, patch_name);
    return buf;
}
static std::string log_entry_cpu_buy(int player_1idx, char patch_name) {
    char buf[60];
    std::snprintf(buf, sizeof(buf), "P%d (cpu) bought [%c]", player_1idx, patch_name);
    return buf;
}
static std::string log_entry_cpu_advance(int player_1idx) {
    char buf[50];
    std::snprintf(buf, sizeof(buf), "P%d (cpu) advanced", player_1idx);
    return buf;
}
static std::string log_entry_advance(int player_1idx) {
    char buf[40];
    std::snprintf(buf, sizeof(buf), "P%d advanced", player_1idx);
    return buf;
}

static void setup_scene(SimplifiedGameState& state, LogState& log,
                        NdjsonState& ndjson) {
    state  = SimplifiedGameState{};
    log    = LogState{};
    ndjson = NdjsonState{};
    // Use the patch-name format (patch_index=0 has name '2').
    append_log(log, log_entry_buy(1, '2'));
    append_log(log, log_entry_advance(2));
    append_ndjson(ndjson,
        R"({"event":"move","ply":1,"player":0,"move_type":"buy_patch",)"
        R"("patch_index":0,"position":3,"buttons":2})");
}

// Return the directory where snapshot files live.
static fs::path snapshot_dir() {
    const char* env = std::getenv("SNAPSHOT_DIR");
    if (env && *env) return fs::path(env);
    // Fallback: next to the source tree (works when run from repo root).
    return fs::path("tests") / "snapshots";
}

// Read an entire file into a string; returns empty string if the file does
// not exist yet (treated as "no snapshot stored yet").
static std::string read_file(const fs::path& p) {
    std::ifstream f(p, std::ios::binary);
    if (!f) return {};
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

// Write content to a file, creating parent directories as needed.
static void write_file(const fs::path& p, const std::string& content) {
    fs::create_directories(p.parent_path());
    std::ofstream f(p, std::ios::binary | std::ios::trunc);
    f << content;
}

// Produce a simple line-by-line unified diff for display in test failure output.
static std::string unified_diff(const std::string& expected,
                                const std::string& actual) {
    // Split both strings into lines.
    auto split = [](const std::string& s) {
        std::vector<std::string> lines;
        std::istringstream ss(s);
        std::string line;
        while (std::getline(ss, line)) lines.push_back(line);
        return lines;
    };

    auto exp_lines = split(expected);
    auto act_lines = split(actual);

    std::ostringstream out;
    std::size_t max_lines = std::max(exp_lines.size(), act_lines.size());
    bool any_diff = false;

    for (std::size_t i = 0; i < max_lines; ++i) {
        const std::string& e = (i < exp_lines.size()) ? exp_lines[i] : "<missing>";
        const std::string& a = (i < act_lines.size()) ? act_lines[i] : "<missing>";
        if (e != a) {
            out << "line " << (i + 1) << ":\n";
            out << "  expected: " << e << "\n";
            out << "  actual  : " << a << "\n";
            any_diff = true;
        }
    }
    if (!any_diff) out << "(no line differences — byte-level difference only)\n";
    return out.str();
}

// Core snapshot assertion: compare `actual` against `snapshot_name`.txt.
// If UPDATE_SNAPSHOTS=1, overwrite the file and always pass.
static void check_snapshot(const std::string& snapshot_name,
                            const std::string& actual) {
    fs::path snap_file = snapshot_dir() / (snapshot_name + ".txt");
    const char* update_env = std::getenv("UPDATE_SNAPSHOTS");
    bool update = (update_env != nullptr && std::string(update_env) == "1");

    if (update) {
        write_file(snap_file, actual);
        SUCCEED("Snapshot updated: " + snap_file.string());
        return;
    }

    std::string stored = read_file(snap_file);
    if (stored.empty()) {
        // No snapshot yet — write it and fail so CI notices.
        write_file(snap_file, actual);
        FAIL("No snapshot existed for '" + snapshot_name +
             "'. Created: " + snap_file.string() +
             ". Re-run the test (or commit the new snapshot file).");
    }

    if (stored != actual) {
        std::string diff = unified_diff(stored, actual);
        FAIL("Snapshot mismatch for '" + snapshot_name + "'.\n"
             "To update: set UPDATE_SNAPSHOTS=1 and re-run.\n\n"
             "Diff (expected vs actual):\n" + diff);
    }
}

// Zero-padded integer: 0 → "00", 5 → "05", 35 → "35"
static std::string zpad2(int n) {
    std::ostringstream ss;
    ss << std::setfill('0') << std::setw(2) << n;
    return ss.str();
}

// ── Snapshot test cases ───────────────────────────────────────────────────────

TEST_CASE("Snapshot: 80-col narrow layout", "[tui_snapshot]") {
    SimplifiedGameState state;
    LogState log;
    NdjsonState ndjson;
    setup_scene(state, log, ndjson);
    auto setup = GameSetup(kGameSetups[0]);

    std::string actual = render_frame_to_string(state, setup, log, ndjson,
                                                make_cfg(80));
    check_snapshot("frame_080", actual);
}

TEST_CASE("Snapshot: 120-col narrow layout (wider log pane)", "[tui_snapshot]") {
    SimplifiedGameState state;
    LogState log;
    NdjsonState ndjson;
    setup_scene(state, log, ndjson);
    auto setup = GameSetup(kGameSetups[0]);

    std::string actual = render_frame_to_string(state, setup, log, ndjson,
                                                make_cfg(120));
    check_snapshot("frame_120", actual);
}

TEST_CASE("Snapshot: 160-col wide layout (two-column)", "[tui_snapshot]") {
    SimplifiedGameState state;
    LogState log;
    NdjsonState ndjson;
    setup_scene(state, log, ndjson);
    auto setup = GameSetup(kGameSetups[0]);

    std::string actual = render_frame_to_string(state, setup, log, ndjson,
                                                make_cfg(160));
    check_snapshot("frame_160", actual);
}

TEST_CASE("Snapshot: 80-col with ANSI color codes", "[tui_snapshot]") {
    SimplifiedGameState state;
    LogState log;
    NdjsonState ndjson;
    setup_scene(state, log, ndjson);
    append_ndjson(ndjson,
        R"({"event":"move","ply":2,"player":1,"move_type":"advance",)"
        R"("position":5,"buttons":5})");
    auto setup = GameSetup(kGameSetups[0]);

    std::string actual = render_frame_to_string(state, setup, log, ndjson,
                                                make_cfg(80, /*color=*/true));
    check_snapshot("frame_080_color", actual);
}

TEST_CASE("Snapshot: 80-col, 40-row taller terminal with 12 log entries",
          "[tui_snapshot]") {
    SimplifiedGameState state{};
    LogState log{};
    NdjsonState ndjson{};
    ndjson.height = 5;

    auto setup = GameSetup(kGameSetups[0]);
    auto cfg   = make_cfg_h(80, 40);

    // 12 log entries with single-char patch names to verify visual alignment.
    // All "[X]" tokens are exactly 3 bytes wide, ensuring column alignment.
    const char* entries[] = {
        "P1 bought [2]", "P2 advanced",   "P1 advanced",
        "P2 bought [v]", "P1 bought [3]", "P2 advanced",
        "P1 advanced",   "P2 bought [j]", "P1 bought [t]",
        "P2 advanced",   "P1 advanced",   "P2 bought [4]",
    };
    for (auto* e : entries) append_log(log, e);
    append_ndjson(ndjson,
        R"({"event":"move","ply":12,"player":1,"move_type":"advance"})");

    std::string actual = render_frame_to_string(state, setup, log, ndjson, cfg);
    check_snapshot("frame_080_h40", actual);
}

// ── Full game sequence snapshot test ──────────────────────────────────────────
//
// Drives a complete simplified game with seed=42 / setup=0 using the random
// agent for ALL moves.  Snapshots are stored in tests/snapshots/game_seq/.
// The test also exercises undo/redo after the first 5 rounds.

TEST_CASE("Snapshot: full game sequence with undo/redo (seed=42, setup=0)",
          "[tui_snapshot]") {
    auto setup = GameSetup(kGameSetups[0]);
    SimplifiedGameState state{};
    RngState agent_rng(42);  // random-agent RNG; separate from the game state
    LogState log{};
    NdjsonState ndjson{};
    ndjson.height = 3;  // fixed for reproducible snapshots
    History history(state, agent_rng);

    // Display: 80 cols, 40 rows — taller terminal so more detail lines are shown.
    auto cfg = make_cfg_h(80, 40);

    int step = 0;
    auto snap = [&](const std::string& label) {
        check_snapshot("game_seq/step_" + zpad2(step) + "_" + label,
                       render_frame_to_string(state, setup, log, ndjson, cfg));
        ++step;
    };

    // Helper: apply one ply via random_move (both players use random agent).
    // Returns true if a move was made, false if game was already terminal.
    auto do_ply = [&]() -> bool {
        if (is_terminal(state)) return false;
        // Snapshot the agent_rng BEFORE making the move (for deterministic redo).
        RngState agent_rng_snap = agent_rng;
        auto move = random_move(state, setup, agent_rng);
        int player_1idx = state.active_player() + 1;
        if (std::holds_alternative<BuyPatch>(move)) {
            int pid = std::get<BuyPatch>(move).patch_index;
            char pname = kPatches[static_cast<std::size_t>(pid)].name;
            append_log(log, log_entry_buy(player_1idx, pname));
        } else {
            append_log(log, log_entry_advance(player_1idx));
        }
        state = apply_move(state, move, setup);
        history.push(state, agent_rng_snap, log.entries);
        return true;
    };

    snap("initial");            // step 00

    // Play 10 plies (5 P1 moves + 5 P2 moves in a typical alternating game).
    for (int i = 0; i < 10; ++i) {
        if (!do_ply()) break;
        snap("ply" + zpad2(i));  // steps 01–10
    }

    // Undo twice.
    history.undo();
    state     = history.current_state();
    log.entries = history.current_log_entries();
    agent_rng       = history.current_rng();
    snap("undo1");              // step 11

    history.undo();
    state     = history.current_state();
    log.entries = history.current_log_entries();
    agent_rng       = history.current_rng();
    snap("undo2");              // step 12

    // Redo once.
    history.redo();
    state     = history.current_state();
    log.entries = history.current_log_entries();
    agent_rng       = history.current_rng();
    snap("redo1");              // step 13

    // Continue to end of game.  The guard of 50 (> max game plies ~35) prevents
    // an infinite loop if the terminal condition is never reached.
    static constexpr int kMaxRemainingPlies = 50;
    int remaining = 0;
    while (!is_terminal(state) && remaining < kMaxRemainingPlies) {
        do_ply();
        snap("cont" + zpad2(remaining));  // steps 14+
        ++remaining;
    }
}

// ── CPU multi-turn snapshot test ─────────────────────────────────────────────
//
// Exercises the TUI game-loop rule: after P1 (human) buys a patch with a large
// time cost, P2 (CPU) may need to take several consecutive turns before P1
// regains the move.  This test replicates the TUI's inner CPU loop:
//
//   while (!is_terminal && active_player == 1) { cpu_move; }
//
// With setup=0 / seed=42, P1 buying "L" (time=6) from pos 0 → pos 6 forces P2
// to run at least 6 consecutive turns before catching up.  The snapshot should
// show 6+ "P2 (cpu)" entries in the event log after the single human move.

TEST_CASE("Snapshot: CPU takes multiple consecutive turns after P1 buys far-time patch",
          "[tui_snapshot]") {
    auto setup = GameSetup(kGameSetups[0]);
    SimplifiedGameState state{};
    // seed=42 — same as the full game sequence test for reproducibility.
    RngState cpu_rng(42);
    LogState log{};
    NdjsonState ndjson{};
    ndjson.height = 3;
    auto cfg = make_cfg_h(80, 40);

    // P1 (human) buys "L" (position 3 in the circle, time cost 6).
    // Find "L" in the legal moves for the initial state.
    auto initial_moves = legal_moves(state, setup);
    patchwork::Move human_move = Advance{};
    for (auto& m : initial_moves) {
        if (std::holds_alternative<BuyPatch>(m)) {
            int pid = std::get<BuyPatch>(m).patch_index;
            if (kPatches[static_cast<std::size_t>(pid)].name == 'L') {
                human_move = m;
                break;
            }
        }
    }
    // Verify we found it (sanity check — if setup changes this will trip).
    REQUIRE(std::holds_alternative<BuyPatch>(human_move));
    {
        int pid = std::get<BuyPatch>(human_move).patch_index;
        char pname = kPatches[static_cast<std::size_t>(pid)].name;
        REQUIRE(pname == 'L');
        append_log(log, log_entry_buy(1, pname));
    }
    state = apply_move(state, human_move, setup);

    // Verify that P2 (player index 1) is now active.
    REQUIRE(state.active_player() == 1);

    // Replicate the TUI CPU loop: P2 takes consecutive turns until
    // it's P1's turn again or the game ends.
    int cpu_turns = 0;
    while (!is_terminal(state) && state.active_player() != 0) {
        int cpu_1idx = state.active_player() + 1;  // 2 for P2
        auto opp_move = random_move(state, setup, cpu_rng);
        if (std::holds_alternative<BuyPatch>(opp_move)) {
            int pid = std::get<BuyPatch>(opp_move).patch_index;
            char pname = kPatches[static_cast<std::size_t>(pid)].name;
            append_log(log, log_entry_cpu_buy(cpu_1idx, pname));
        } else {
            append_log(log, log_entry_cpu_advance(cpu_1idx));
        }
        state = apply_move(state, opp_move, setup);
        ++cpu_turns;
    }

    // P2 must have taken at least 2 turns.  With seed=42, P2 buys patch [3]
    // (time=2, pos 0→2) and then advances to pos 7 (P1's pos+1=7) — exactly 2
    // CPU turns.  This would be only 1 if the old single-shot loop were used.
    REQUIRE(cpu_turns >= 2);

    // After the CPU loop, P1 should be active again (or game over).
    if (!is_terminal(state)) {
        REQUIRE(state.active_player() == 0);
    }

    std::string actual = render_frame_to_string(state, setup, log, ndjson, cfg);
    check_snapshot("cpu_multi_turn", actual);
}
