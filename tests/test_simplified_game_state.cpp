#include <catch2/catch_test_macros.hpp>
#include "simplified_game_state.hpp"

using patchwork::SimplifiedPlayerState;
using patchwork::SimplifiedGameState;
using patchwork::BonusStatus;

// ── SimplifiedPlayerState ──────────────────────────────────────────────────

TEST_CASE("SimplifiedPlayerState fits in 32 bits", "[simplified_player_state]") {
    REQUIRE(sizeof(SimplifiedPlayerState) <= 4);
}

TEST_CASE("SimplifiedPlayerState default construction", "[simplified_player_state]") {
    SimplifiedPlayerState ps;
    REQUIRE(ps.free_spaces() == 81);
    REQUIRE(ps.position() == 0);
    REQUIRE(ps.buttons() == 5);
    REQUIRE(ps.income() == 0);
}

TEST_CASE("SimplifiedPlayerState free_spaces round-trip", "[simplified_player_state]") {
    SimplifiedPlayerState ps;
    for (int v = 0; v <= 81; ++v) {
        ps.set_free_spaces(v);
        REQUIRE(ps.free_spaces() == v);
    }
}

TEST_CASE("SimplifiedPlayerState position round-trip", "[simplified_player_state]") {
    SimplifiedPlayerState ps;
    for (int v = 0; v <= 53; ++v) {
        ps.set_position(v);
        REQUIRE(ps.position() == v);
    }
}

TEST_CASE("SimplifiedPlayerState buttons round-trip", "[simplified_player_state]") {
    SimplifiedPlayerState ps;
    for (int v = 0; v <= 127; ++v) {
        ps.set_buttons(v);
        REQUIRE(ps.buttons() == v);
    }
}

TEST_CASE("SimplifiedPlayerState income round-trip", "[simplified_player_state]") {
    SimplifiedPlayerState ps;
    for (int v = 0; v <= 31; ++v) {
        ps.set_income(v);
        REQUIRE(ps.income() == v);
    }
}

TEST_CASE("SimplifiedPlayerState fields do not interfere", "[simplified_player_state]") {
    SimplifiedPlayerState ps;
    ps.set_free_spaces(42);
    ps.set_position(53);
    ps.set_buttons(100);
    ps.set_income(20);

    REQUIRE(ps.free_spaces() == 42);
    REQUIRE(ps.position() == 53);
    REQUIRE(ps.buttons() == 100);
    REQUIRE(ps.income() == 20);
}

// ── SimplifiedGameState ───────────────────────────────────────────────────

TEST_CASE("SimplifiedGameState default construction", "[simplified_game_state]") {
    SimplifiedGameState gs;

    SECTION("both players default-constructed") {
        for (int p = 0; p < 2; ++p) {
            REQUIRE(gs.player(p).free_spaces() == 81);
            REQUIRE(gs.player(p).position() == 0);
            REQUIRE(gs.player(p).buttons() == 5);
            REQUIRE(gs.player(p).income() == 0);
        }
    }

    SECTION("all 33 patches available") {
        for (int i = 0; i < 33; ++i)
            REQUIRE(gs.patch_available(i) == true);
    }

    SECTION("circle marker is 0") {
        REQUIRE(gs.circle_marker() == 0);
    }

    SECTION("bonus is unclaimed") {
        REQUIRE(gs.bonus_status() == BonusStatus::kUnclaimed);
    }
}

TEST_CASE("SimplifiedGameState patch availability round-trip", "[simplified_game_state]") {
    SimplifiedGameState gs;

    for (int i = 0; i < 33; ++i) {
        gs.set_patch_available(i, false);
        REQUIRE(gs.patch_available(i) == false);
        gs.set_patch_available(i, true);
        REQUIRE(gs.patch_available(i) == true);
    }
}

TEST_CASE("SimplifiedGameState circle marker round-trip", "[simplified_game_state]") {
    SimplifiedGameState gs;
    for (int v = 0; v <= 32; ++v) {
        gs.set_circle_marker(v);
        REQUIRE(gs.circle_marker() == v);
    }
}

TEST_CASE("SimplifiedGameState shared state does not interfere with patch bits",
          "[simplified_game_state]") {
    SimplifiedGameState gs;

    gs.set_circle_marker(32);
    gs.set_bonus_status(BonusStatus::kPlayer1);

    for (int i = 0; i < 33; ++i)
        REQUIRE(gs.patch_available(i) == true);

    REQUIRE(gs.circle_marker() == 32);
    REQUIRE(gs.bonus_status() == BonusStatus::kPlayer1);
}
