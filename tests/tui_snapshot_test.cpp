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
#include <sstream>
#include <string>
#include <vector>

#include "tui/display.hpp"
#include "simplified_game_state.hpp"
#include "game_setups.hpp"

namespace fs = std::filesystem;
using patchwork::SimplifiedGameState;
using patchwork::make_setup;
using patchwork::tui::DisplayConfig;
using patchwork::tui::LogState;
using patchwork::tui::NdjsonState;
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

static void setup_scene(SimplifiedGameState& state, LogState& log,
                        NdjsonState& ndjson) {
    state  = SimplifiedGameState{};
    log    = LogState{};
    ndjson = NdjsonState{};
    append_log(log, "P1 bought [0]");
    append_log(log, "P2 advanced");
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

// ── Snapshot test cases ───────────────────────────────────────────────────────

TEST_CASE("Snapshot: 80-col narrow layout", "[tui_snapshot]") {
    SimplifiedGameState state;
    LogState log;
    NdjsonState ndjson;
    setup_scene(state, log, ndjson);
    auto setup = make_setup(0);

    std::string actual = render_frame_to_string(state, setup, log, ndjson,
                                                make_cfg(80));
    check_snapshot("frame_080", actual);
}

TEST_CASE("Snapshot: 120-col narrow layout (wider log pane)", "[tui_snapshot]") {
    SimplifiedGameState state;
    LogState log;
    NdjsonState ndjson;
    setup_scene(state, log, ndjson);
    auto setup = make_setup(0);

    std::string actual = render_frame_to_string(state, setup, log, ndjson,
                                                make_cfg(120));
    check_snapshot("frame_120", actual);
}

TEST_CASE("Snapshot: 160-col wide layout (two-column)", "[tui_snapshot]") {
    SimplifiedGameState state;
    LogState log;
    NdjsonState ndjson;
    setup_scene(state, log, ndjson);
    auto setup = make_setup(0);

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
    auto setup = make_setup(0);

    std::string actual = render_frame_to_string(state, setup, log, ndjson,
                                                make_cfg(80, /*color=*/true));
    check_snapshot("frame_080_color", actual);
}
