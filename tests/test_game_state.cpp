#include <catch2/catch_test_macros.hpp>

#include "game_state.hpp"

using patchwork::BonusStatus;
using patchwork::GameState;

TEST_CASE("GameState default construction", "[game_state]") {
    GameState gs;

    SECTION("both players default-constructed") {
        for (int p = 0; p < 2; ++p) {
            REQUIRE(gs.player(p).position() == 0);
            REQUIRE(gs.player(p).buttons() == 5);
            REQUIRE(gs.player(p).income() == 0);
            for (int r = 0; r < 9; ++r)
                for (int c = 0; c < 9; ++c) REQUIRE(gs.player(p).cell(r, c) == false);
        }
    }

    SECTION("all 33 patches available") {
        for (int i = 0; i < 33; ++i) REQUIRE(gs.patch_available(i) == true);
    }

    SECTION("circle marker is 0") { REQUIRE(gs.circle_marker() == 0); }

    SECTION("bonus is unclaimed") { REQUIRE(gs.bonus_status() == BonusStatus::kUnclaimed); }
}

TEST_CASE("GameState patch availability round-trip", "[game_state]") {
    GameState gs;

    SECTION("mark patch 0 unavailable") {
        gs.set_patch_available(0, false);
        REQUIRE(gs.patch_available(0) == false);
        for (int i = 1; i < 33; ++i) REQUIRE(gs.patch_available(i) == true);
    }

    SECTION("mark patch 32 unavailable") {
        gs.set_patch_available(32, false);
        REQUIRE(gs.patch_available(32) == false);
        for (int i = 0; i < 32; ++i) REQUIRE(gs.patch_available(i) == true);
    }

    SECTION("re-enable patch") {
        gs.set_patch_available(15, false);
        REQUIRE(gs.patch_available(15) == false);
        gs.set_patch_available(15, true);
        REQUIRE(gs.patch_available(15) == true);
    }
}

TEST_CASE("GameState circle marker range", "[game_state]") {
    GameState gs;
    for (int v = 0; v <= 32; ++v) {
        gs.set_circle_marker(v);
        REQUIRE(gs.circle_marker() == v);
    }
}

TEST_CASE("GameState bonus status transitions", "[game_state]") {
    GameState gs;

    REQUIRE(gs.bonus_status() == BonusStatus::kUnclaimed);

    gs.set_bonus_status(BonusStatus::kPlayer0);
    REQUIRE(gs.bonus_status() == BonusStatus::kPlayer0);

    gs.set_bonus_status(BonusStatus::kPlayer1);
    REQUIRE(gs.bonus_status() == BonusStatus::kPlayer1);

    gs.set_bonus_status(BonusStatus::kUnclaimed);
    REQUIRE(gs.bonus_status() == BonusStatus::kUnclaimed);
}

TEST_CASE("GameState shared state does not interfere with patch bits", "[game_state]") {
    GameState gs;

    // Set circle marker and bonus
    gs.set_circle_marker(32);
    gs.set_bonus_status(BonusStatus::kPlayer1);

    // All patches should still be available
    for (int i = 0; i < 33; ++i) REQUIRE(gs.patch_available(i) == true);

    // Circle and bonus still correct
    REQUIRE(gs.circle_marker() == 32);
    REQUIRE(gs.bonus_status() == BonusStatus::kPlayer1);
}
