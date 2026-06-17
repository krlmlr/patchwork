#include <catch2/catch_test_macros.hpp>
#include "tui/history.hpp"
#include "simplified_game_state.hpp"

using patchwork::SimplifiedGameState;
using patchwork::tui::History;
using patchwork::tui::RngState;

// Helper: build a state with distinct button value so we can tell them apart.
static SimplifiedGameState make_state(int buttons_p0) {
    SimplifiedGameState s;
    s.player(0).set_buttons(buttons_p0);
    return s;
}

TEST_CASE("History initial entry", "[tui_history]") {
    RngState rng(42);
    SimplifiedGameState s0 = make_state(5);
    History h(s0, rng);

    REQUIRE(h.current_state().player(0).buttons() == 5);
    REQUIRE(!h.can_undo());
    REQUIRE(!h.can_redo());
}

TEST_CASE("History push adds entry", "[tui_history]") {
    RngState rng(42);
    History h(make_state(5), rng);

    h.push(make_state(10), rng);
    REQUIRE(h.current_state().player(0).buttons() == 10);
    REQUIRE(h.can_undo());
    REQUIRE(!h.can_redo());
}

TEST_CASE("History undo moves cursor back", "[tui_history]") {
    RngState rng(42);
    History h(make_state(5), rng);
    h.push(make_state(10), rng);

    h.undo();
    REQUIRE(h.current_state().player(0).buttons() == 5);
    REQUIRE(!h.can_undo());
    REQUIRE(h.can_redo());
}

TEST_CASE("History redo moves cursor forward", "[tui_history]") {
    RngState rng(42);
    History h(make_state(5), rng);
    h.push(make_state(10), rng);
    h.undo();
    h.redo();
    REQUIRE(h.current_state().player(0).buttons() == 10);
    REQUIRE(h.can_undo());
    REQUIRE(!h.can_redo());
}

TEST_CASE("History push-after-undo truncates redo branch", "[tui_history]") {
    RngState rng(42);
    History h(make_state(5), rng);
    h.push(make_state(10), rng);
    h.push(make_state(15), rng);

    h.undo();
    h.undo();
    REQUIRE(h.current_state().player(0).buttons() == 5);

    h.push(make_state(99), rng);
    REQUIRE(h.current_state().player(0).buttons() == 99);
    REQUIRE(!h.can_redo());
}

TEST_CASE("History undo at boundary is no-op", "[tui_history]") {
    RngState rng(42);
    History h(make_state(5), rng);
    h.undo();  // no-op
    REQUIRE(h.current_state().player(0).buttons() == 5);
}

TEST_CASE("History redo at boundary is no-op", "[tui_history]") {
    RngState rng(42);
    History h(make_state(5), rng);
    h.redo();  // no-op
    REQUIRE(h.current_state().player(0).buttons() == 5);
}

TEST_CASE("History deterministic redo: current_rng() returns saved RNG state",
          "[tui_history]") {
    // Build two RNG states that will produce different output.
    RngState rng0(1);
    RngState rng1(2);

    History h(make_state(5), rng0);
    h.push(make_state(10), rng1);
    h.undo();

    // After undo, current_rng() should equal rng0.
    REQUIRE(h.current_rng() == RngState(1));

    h.redo();
    // After redo, current_rng() should equal rng1.
    REQUIRE(h.current_rng() == RngState(2));
}
