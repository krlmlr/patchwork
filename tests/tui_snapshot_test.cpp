#include <catch2/catch_test_macros.hpp>
#include "tui/display.hpp"
#include "simplified_game_state.hpp"
#include "game_setups.hpp"

using patchwork::SimplifiedGameState;
using patchwork::make_setup;
using patchwork::tui::DisplayConfig;
using patchwork::tui::LogState;
using patchwork::tui::NdjsonState;
using patchwork::tui::append_log;
using patchwork::tui::append_ndjson;
using patchwork::tui::render_frame_to_string;

// Helper: build a DisplayConfig for a given width (height always 24).
static DisplayConfig make_cfg(int width, bool color = false) {
    DisplayConfig cfg;
    cfg.width = width;
    cfg.height = 24;
    cfg.color_enabled = color;
    return cfg;
}

// Helper: build a default state + log + ndjson with a few log entries.
static void setup_scene(SimplifiedGameState& state, LogState& log,
                         NdjsonState& ndjson) {
    state = SimplifiedGameState{};
    log = LogState{};
    ndjson = NdjsonState{};
    append_log(log, "P1 bought [0]");
    append_log(log, "P2 advanced");
    append_ndjson(ndjson, R"({"event":"move","ply":1,"player":0,"move_type":"buy_patch","patch_index":0,"position":3,"buttons":2})");
}

// ── Snapshot tests: layout content ───────────────────────────────────────

TEST_CASE("Snapshot 80-col: frame contains circle prefix and quilt markers",
          "[tui_snapshot]") {
    SimplifiedGameState state;
    LogState log;
    NdjsonState ndjson;
    setup_scene(state, log, ndjson);
    auto setup = make_setup(0);

    std::string frame = render_frame_to_string(state, setup, log, ndjson, make_cfg(80));

    REQUIRE(!frame.empty());
    // Check the circle label is present.
    REQUIRE(frame.find("Circle:") != std::string::npos);
    // Check the ^ marker is present.
    REQUIRE(frame.find("^") != std::string::npos);
    // Check quilt grid markers.
    REQUIRE(frame.find("?????????") != std::string::npos);
    // Check NDJSON header.
    REQUIRE(frame.find("ndjson log") != std::string::npos);
    // Check event log entry.
    REQUIRE(frame.find("P1 bought [0]") != std::string::npos);
}

TEST_CASE("Snapshot 120-col: same content, wider frame", "[tui_snapshot]") {
    SimplifiedGameState state;
    LogState log;
    NdjsonState ndjson;
    setup_scene(state, log, ndjson);
    auto setup = make_setup(0);

    std::string frame80  = render_frame_to_string(state, setup, log, ndjson, make_cfg(80));
    std::string frame120 = render_frame_to_string(state, setup, log, ndjson, make_cfg(120));

    // Both should have circle and quilt content.
    REQUIRE(frame120.find("Circle:") != std::string::npos);
    REQUIRE(frame120.find("?????????") != std::string::npos);
    // 120-col frame should be wider (more chars per line).
    REQUIRE(frame120.size() > frame80.size());
}

TEST_CASE("Snapshot 160-col: wide layout shows circle and quilts", "[tui_snapshot]") {
    SimplifiedGameState state;
    LogState log;
    NdjsonState ndjson;
    setup_scene(state, log, ndjson);
    auto setup = make_setup(0);

    std::string frame = render_frame_to_string(state, setup, log, ndjson, make_cfg(160));

    REQUIRE(frame.find("Circle:") != std::string::npos);
    REQUIRE(frame.find("?????????") != std::string::npos);
    REQUIRE(frame.find("ndjson log") != std::string::npos);
}

TEST_CASE("Snapshot 159-col: narrow layout (not wide)", "[tui_snapshot]") {
    SimplifiedGameState state;
    LogState log;
    NdjsonState ndjson;
    setup_scene(state, log, ndjson);
    auto setup = make_setup(0);
    // Both narrow and wide should have circles and quilts.
    std::string frame159 = render_frame_to_string(state, setup, log, ndjson, make_cfg(159));
    std::string frame160 = render_frame_to_string(state, setup, log, ndjson, make_cfg(160));
    REQUIRE(frame159.find("Circle:") != std::string::npos);
    REQUIRE(frame160.find("Circle:") != std::string::npos);
    // Different layouts should produce different output.
    REQUIRE(frame159 != frame160);
}

// ── Snapshot tests: color output ──────────────────────────────────────────

TEST_CASE("Snapshot color: output contains ANSI codes when enabled",
          "[tui_snapshot]") {
    SimplifiedGameState state;
    LogState log;
    NdjsonState ndjson;
    setup_scene(state, log, ndjson);
    // Add NDJSON line that will trigger concept colors.
    append_ndjson(ndjson, R"({"event":"move","ply":2,"player":1,"move_type":"advance","position":5,"buttons":5})");
    auto setup = make_setup(0);

    std::string with_color    = render_frame_to_string(state, setup, log, ndjson, make_cfg(80, true));
    std::string without_color = render_frame_to_string(state, setup, log, ndjson, make_cfg(80, false));

    // With color, ESC codes should appear.
    REQUIRE(with_color.find('\033') != std::string::npos);
    // Without color, no ESC codes.
    REQUIRE(without_color.find('\033') == std::string::npos);
}
