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

TEST_CASE("History initial entry stores state and both RNG streams", "[tui_history]") {
    RngState rng_p0(1);
    RngState rng_p1(2);
    SimplifiedGameState s0 = make_state(5);
    History h(s0, rng_p0, rng_p1);

    REQUIRE(h.current_state().player(0).buttons() == 5);
    REQUIRE(h.current_rng_p0() == RngState(1));
    REQUIRE(h.current_rng_p1() == RngState(2));
    REQUIRE(!h.can_undo());
    REQUIRE(!h.can_redo());
}

TEST_CASE("History push adds entry", "[tui_history]") {
    RngState rng(42);
    History h(make_state(5), rng, rng);

    h.push(make_state(10), rng, rng);
    REQUIRE(h.current_state().player(0).buttons() == 10);
    REQUIRE(h.can_undo());
    REQUIRE(!h.can_redo());
}

TEST_CASE("History push stores both RNG states", "[tui_history]") {
    RngState rng_a(1);
    RngState rng_b(2);
    History h(make_state(5), rng_a, rng_a);

    h.push(make_state(10), rng_a, rng_b);
    REQUIRE(h.current_rng_p0() == RngState(1));
    REQUIRE(h.current_rng_p1() == RngState(2));
}

TEST_CASE("History undo moves cursor back", "[tui_history]") {
    RngState rng(42);
    History h(make_state(5), rng, rng);
    h.push(make_state(10), rng, rng);

    h.undo();
    REQUIRE(h.current_state().player(0).buttons() == 5);
    REQUIRE(!h.can_undo());
    REQUIRE(h.can_redo());
}

TEST_CASE("History redo moves cursor forward", "[tui_history]") {
    RngState rng(42);
    History h(make_state(5), rng, rng);
    h.push(make_state(10), rng, rng);
    h.undo();
    h.redo();
    REQUIRE(h.current_state().player(0).buttons() == 10);
    REQUIRE(h.can_undo());
    REQUIRE(!h.can_redo());
}

TEST_CASE("History push-after-undo truncates redo branch", "[tui_history]") {
    RngState rng(42);
    History h(make_state(5), rng, rng);
    h.push(make_state(10), rng, rng);
    h.push(make_state(15), rng, rng);

    h.undo();
    h.undo();
    REQUIRE(h.current_state().player(0).buttons() == 5);

    h.push(make_state(99), rng, rng);
    REQUIRE(h.current_state().player(0).buttons() == 99);
    REQUIRE(!h.can_redo());
}

TEST_CASE("History undo at boundary is no-op", "[tui_history]") {
    RngState rng(42);
    History h(make_state(5), rng, rng);
    h.undo();  // no-op
    REQUIRE(h.current_state().player(0).buttons() == 5);
}

TEST_CASE("History redo at boundary is no-op", "[tui_history]") {
    RngState rng(42);
    History h(make_state(5), rng, rng);
    h.redo();  // no-op
    REQUIRE(h.current_state().player(0).buttons() == 5);
}

TEST_CASE("History undo/redo restores both per-player RNG states independently",
          "[tui_history]") {
    // Distinct RNG states so we can tell them apart.
    RngState rng_p0_init(1);
    RngState rng_p1_init(2);
    RngState rng_p0_next(3);
    RngState rng_p1_next(4);

    History h(make_state(5), rng_p0_init, rng_p1_init);
    h.push(make_state(10), rng_p0_next, rng_p1_next);

    h.undo();
    // After undo, both streams are restored to their initial states.
    REQUIRE(h.current_rng_p0() == RngState(1));
    REQUIRE(h.current_rng_p1() == RngState(2));

    h.redo();
    // After redo, both streams are restored to the pushed states.
    REQUIRE(h.current_rng_p0() == RngState(3));
    REQUIRE(h.current_rng_p1() == RngState(4));
}

TEST_CASE("History deterministic redo: opponent move after redo is reproducible",
          "[tui_history]") {
    // Two RNG engines seeded identically will produce identical draws. The
    // history stores the opponent's stream (rng_p1) so that after redo the same
    // draw is produced.
    RngState rng_p1(7);
    RngState reference(7);

    History h(make_state(5), RngState(0), rng_p1);
    h.push(make_state(10), RngState(0), rng_p1);
    h.undo();
    h.redo();

    // Draw from the restored stream and from an identically-seeded reference:
    // they must match, proving the redo reseeds deterministically.
    RngState restored = h.current_rng_p1();
    REQUIRE(restored() == reference());
}
