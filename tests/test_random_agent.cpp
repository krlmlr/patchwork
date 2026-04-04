#include <catch2/catch_test_macros.hpp>
#include "random_agent.hpp"
#include "move_generation.hpp"
#include "game_setups.hpp"

#include <map>
#include <variant>

using namespace patchwork;

TEST_CASE("random_move returns a legal move", "[random_agent]") {
    auto setup = make_setup(0);
    SimplifiedGameState state;
    std::mt19937 rng(123);
    Move mv = random_move(state, setup, rng);
    auto legal = legal_moves(state, setup);
    bool found = false;
    for (const auto& m : legal) {
        if (m == mv) { found = true; break; }
    }
    REQUIRE(found);
}

TEST_CASE("random_move: same seed produces same move", "[random_agent]") {
    auto setup = make_setup(0);
    SimplifiedGameState state;
    std::mt19937 rng1(42);
    std::mt19937 rng2(42);
    auto mv1 = random_move(state, setup, rng1);
    auto mv2 = random_move(state, setup, rng2);
    REQUIRE(mv1 == mv2);
}

TEST_CASE("random_move: distribution is approximately uniform", "[random_agent]") {
    auto setup = make_setup(0);
    SimplifiedGameState state;
    // Make only 3 legal moves by setting buttons to 0 so only Advance is legal,
    // then use a state with exactly 3 BuyPatch+1 Advance
    state.player(0).set_buttons(127);
    for (int i = 0; i < 33; ++i) state.set_patch_available(i, false);
    // Enable exactly 3 affordable patches from marker 0
    for (int i = 0; i < 3; ++i) {
        int patch_id = static_cast<int>(setup.circle()[static_cast<std::size_t>(i)]);
        state.set_patch_available(patch_id, true);
    }
    auto legal = legal_moves(state, setup);
    REQUIRE(legal.size() == 4);  // 3 BuyPatch + 1 Advance

    std::map<std::size_t, int> counts;
    std::mt19937 rng(0);
    const int N = 10000;
    for (int i = 0; i < N; ++i) {
        Move mv = random_move(state, setup, rng);
        for (std::size_t j = 0; j < legal.size(); ++j) {
            if (legal[j] == mv) { counts[j]++; break; }
        }
    }
    // Each move should be selected roughly 25% of the time (tolerance: 20%–35%)
    for (auto& [idx, cnt] : counts) {
        double frac = static_cast<double>(cnt) / N;
        REQUIRE(frac > 0.20);
        REQUIRE(frac < 0.35);
    }
}
