#include <catch2/catch_test_macros.hpp>

#include "player_state.hpp"

using patchwork::PlayerState;

TEST_CASE("PlayerState default construction", "[player_state]") {
    PlayerState ps;

    SECTION("board is empty") {
        for (int r = 0; r < 9; ++r)
            for (int c = 0; c < 9; ++c) REQUIRE(ps.cell(r, c) == false);
    }

    SECTION("position is 0") { REQUIRE(ps.position() == 0); }

    SECTION("buttons is 5 (starting value)") { REQUIRE(ps.buttons() == 5); }

    SECTION("income is 0") { REQUIRE(ps.income() == 0); }
}

TEST_CASE("PlayerState cell set/get round-trip", "[player_state]") {
    PlayerState ps;

    SECTION("set single cell") {
        ps.set_cell(3, 4, true);
        REQUIRE(ps.cell(3, 4) == true);
        // All other cells remain false
        for (int r = 0; r < 9; ++r)
            for (int c = 0; c < 9; ++c)
                if (r != 3 || c != 4) REQUIRE(ps.cell(r, c) == false);
    }

    SECTION("set and clear cell") {
        ps.set_cell(0, 0, true);
        REQUIRE(ps.cell(0, 0) == true);
        ps.set_cell(0, 0, false);
        REQUIRE(ps.cell(0, 0) == false);
    }

    SECTION("cell in upper word (bit >= 64)") {
        // bit index 8*9+0 = 72, still in word0; 8*9+1 = 73 etc
        // bit index 7*9+2 = 65 is in word1
        ps.set_cell(7, 2, true);
        REQUIRE(ps.cell(7, 2) == true);
        // cell (8,8) → bit 80 — in word1
        ps.set_cell(8, 8, true);
        REQUIRE(ps.cell(8, 8) == true);
    }
}

TEST_CASE("PlayerState position round-trip full range", "[player_state]") {
    PlayerState ps;
    for (int v = 0; v <= 53; ++v) {
        ps.set_position(v);
        REQUIRE(ps.position() == v);
    }
}

TEST_CASE("PlayerState buttons round-trip full range", "[player_state]") {
    PlayerState ps;
    for (int v = 0; v <= 127; ++v) {
        ps.set_buttons(v);
        REQUIRE(ps.buttons() == v);
    }
}

TEST_CASE("PlayerState income round-trip full range", "[player_state]") {
    PlayerState ps;
    for (int v = 0; v <= 31; ++v) {
        ps.set_income(v);
        REQUIRE(ps.income() == v);
    }
}

TEST_CASE("PlayerState sizeof assertion", "[player_state]") {
    STATIC_REQUIRE(sizeof(PlayerState) <= 16);
}

TEST_CASE("PlayerState fields do not interfere", "[player_state]") {
    PlayerState ps;

    // Set all scalar fields to non-zero
    ps.set_position(53);
    ps.set_buttons(127);
    ps.set_income(31);

    // Set some board cells
    ps.set_cell(0, 0, true);
    ps.set_cell(8, 8, true);

    // Verify everything reads back correctly
    REQUIRE(ps.position() == 53);
    REQUIRE(ps.buttons() == 127);
    REQUIRE(ps.income() == 31);
    REQUIRE(ps.cell(0, 0) == true);
    REQUIRE(ps.cell(8, 8) == true);
    REQUIRE(ps.cell(4, 4) == false);
}
