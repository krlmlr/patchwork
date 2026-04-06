#include <catch2/catch_test_macros.hpp>
#include "tui/display.hpp"
#include "simplified_game_state.hpp"
#include "game_setup.hpp"

using patchwork::SimplifiedGameState;
using patchwork::tui::LogState;
using patchwork::tui::NdjsonState;
using patchwork::tui::append_log;
using patchwork::tui::ndjson_toggle_minimize;
using patchwork::tui::ndjson_maximize;
using patchwork::tui::ndjson_semi_maximize;
using patchwork::tui::ndjson_decr_lines;
using patchwork::tui::ndjson_incr_lines;

// ── append_log tests ────────────────────────────────────────────────────

TEST_CASE("append_log: entries are added", "[tui_display]") {
    LogState log;
    append_log(log, "hello");
    REQUIRE(log.entries.size() == 1);
    REQUIRE(log.entries[0] == "hello");
}

TEST_CASE("append_log: resets horizontal scroll offset", "[tui_display]") {
    LogState log;
    log.scroll_offset = 5;
    append_log(log, "new entry");
    REQUIRE(log.scroll_offset == 0);
}

TEST_CASE("append_log: buffer trims to 50 entries", "[tui_display]") {
    LogState log;
    for (int i = 0; i < 60; ++i)
        append_log(log, "entry " + std::to_string(i));
    REQUIRE(log.entries.size() == 50);
    // First 10 entries should be gone; entry "entry 10" should be first.
    REQUIRE(log.entries[0] == "entry 10");
    REQUIRE(log.entries[49] == "entry 59");
}

// ── NdjsonState pane height state machine ─────────────────────────────────

TEST_CASE("ndjson_toggle_minimize: minimises then restores", "[tui_display]") {
    NdjsonState s;
    s.height = 5;
    ndjson_toggle_minimize(s, 20);
    REQUIRE(s.height == 0);
    ndjson_toggle_minimize(s, 20);
    REQUIRE(s.height == 5);
}

TEST_CASE("ndjson_maximize: sets height to max", "[tui_display]") {
    NdjsonState s;
    s.height = 3;
    ndjson_maximize(s, 20);
    REQUIRE(s.height == 20);
}

TEST_CASE("ndjson_semi_maximize: sets height to half max", "[tui_display]") {
    NdjsonState s;
    ndjson_semi_maximize(s, 20);
    REQUIRE(s.height == 10);
}

TEST_CASE("ndjson_incr_lines: increments height", "[tui_display]") {
    NdjsonState s;
    s.height = 5;
    ndjson_incr_lines(s, 20);
    REQUIRE(s.height == 6);
}

TEST_CASE("ndjson_incr_lines: clamped at max", "[tui_display]") {
    NdjsonState s;
    s.height = 20;
    ndjson_incr_lines(s, 20);
    REQUIRE(s.height == 20);
}

TEST_CASE("ndjson_decr_lines: decrements height", "[tui_display]") {
    NdjsonState s;
    s.height = 5;
    ndjson_decr_lines(s);
    REQUIRE(s.height == 4);
}

TEST_CASE("ndjson_decr_lines: clamped at 0", "[tui_display]") {
    NdjsonState s;
    s.height = 0;
    ndjson_decr_lines(s);
    REQUIRE(s.height == 0);
}
