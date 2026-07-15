#include <catch2/catch_test_macros.hpp>
#include "move_generation.hpp"
#include "move_application.hpp"
#include "game_setups.hpp"
#include "generated/patches.hpp"
#include "simplified_game_state.hpp"

using namespace patchwork;

TEST_CASE("legal_moves returns empty for terminal state", "[move_generation]") {
    auto setup = make_setup(0);
    SimplifiedGameState state;
    state.player(0).set_position(53);
    state.player(1).set_position(53);
    REQUIRE(legal_moves(state, setup).empty());
}

TEST_CASE("legal_moves always includes Advance for non-terminal state", "[move_generation]") {
    auto setup = make_setup(0);
    SimplifiedGameState state;
    auto moves = legal_moves(state, setup);
    bool has_advance = false;
    for (const auto& m : moves) {
        if (std::holds_alternative<Advance>(m)) has_advance = true;
    }
    REQUIRE(has_advance);
}

TEST_CASE("legal_moves includes up to 3 BuyPatch moves when affordable", "[move_generation]") {
    auto setup = make_setup(0);
    SimplifiedGameState state;
    state.player(0).set_buttons(100);
    auto moves = legal_moves(state, setup);
    int buy_count = 0;
    for (const auto& m : moves) {
        if (std::holds_alternative<BuyPatch>(m)) ++buy_count;
    }
    REQUIRE(buy_count <= 3);
    REQUIRE(buy_count >= 1);
}

TEST_CASE("legal_moves excludes unaffordable patches", "[move_generation]") {
    auto setup = make_setup(0);
    SimplifiedGameState state;
    state.player(0).set_buttons(0);
    auto moves = legal_moves(state, setup);
    // Only Advance should remain
    for (const auto& m : moves) {
        REQUIRE(std::holds_alternative<Advance>(m));
    }
    REQUIRE(moves.size() == 1);
}

TEST_CASE("legal_moves stops at 3 available patches", "[move_generation]") {
    auto setup = make_setup(0);
    SimplifiedGameState state;
    state.player(0).set_buttons(127);
    // Mark all patches unavailable
    for (int i = 0; i < 33; ++i) state.set_patch_available(i, false);
    // Make first 5 available from marker 0
    for (int i = 0; i < 5; ++i) {
        int patch_id = static_cast<int>(setup.circle()[static_cast<std::size_t>(i)]);
        state.set_patch_available(patch_id, true);
    }
    auto moves = legal_moves(state, setup);
    int buy_count = 0;
    for (const auto& m : moves) {
        if (std::holds_alternative<BuyPatch>(m)) ++buy_count;
    }
    REQUIRE(buy_count == 3);
}

TEST_CASE("legal_moves includes fewer than 3 when fewer available", "[move_generation]") {
    auto setup = make_setup(0);
    SimplifiedGameState state;
    state.player(0).set_buttons(127);
    for (int i = 0; i < 33; ++i) state.set_patch_available(i, false);
    int patch_id = static_cast<int>(setup.circle()[0]);
    state.set_patch_available(patch_id, true);
    auto moves = legal_moves(state, setup);
    int buy_count = 0;
    for (const auto& m : moves) {
        if (std::holds_alternative<BuyPatch>(m)) ++buy_count;
    }
    REQUIRE(buy_count == 1);
}

TEST_CASE("legal_moves: Advance present when no patches affordable", "[move_generation]") {
    auto setup = make_setup(0);
    SimplifiedGameState state;
    state.player(0).set_buttons(0);
    auto moves = legal_moves(state, setup);
    REQUIRE(moves.size() == 1);
    REQUIRE(std::holds_alternative<Advance>(moves[0]));
}

TEST_CASE("legal_moves excludes patches that do not fit on the board", "[move_generation]") {
    auto setup = make_setup(0);
    SimplifiedGameState state;
    state.player(0).set_buttons(127);  // affordable regardless of cost

    // Leave room for only a single cell: any patch larger than 1 cell must be
    // excluded even though it is affordable.
    state.player(0).set_free_spaces(1);
    auto moves = legal_moves(state, setup);
    for (const auto& m : moves) {
        if (std::holds_alternative<BuyPatch>(m)) {
            int id = std::get<BuyPatch>(m).patch_index;
            REQUIRE(kPatches[static_cast<std::size_t>(id)].num_cells <= 1);
        }
    }

    // With zero free spaces no patch fits, so only Advance is legal.
    state.player(0).set_free_spaces(0);
    auto none = legal_moves(state, setup);
    REQUIRE(none.size() == 1);
    REQUIRE(std::holds_alternative<Advance>(none[0]));
}

TEST_CASE("applying only legal moves never underflows free spaces", "[move_generation]") {
    // Regression: random play used to buy patches that did not fit, driving
    // free_spaces below zero. Every move returned by legal_moves must keep
    // free_spaces within [0, 81] after application.
    auto setup = make_setup(0);
    SimplifiedGameState state;
    state.player(0).set_buttons(50);
    state.player(0).set_free_spaces(3);  // tight board
    for (const auto& m : legal_moves(state, setup)) {
        auto next = apply_move(state, m, setup);
        REQUIRE(next.player(0).free_spaces() >= 0);
        REQUIRE(next.player(0).free_spaces() <= 81);
    }
}
