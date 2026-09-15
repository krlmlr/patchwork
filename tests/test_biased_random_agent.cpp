#include <sys/wait.h>
#include <unistd.h>

#include <catch2/catch_test_macros.hpp>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <map>
#include <random>
#include <variant>

#include "biased_random_agent.hpp"
#include "game_setups.hpp"
#include "generated/patches.hpp"
#include "move_generation.hpp"

using namespace patchwork;

// Build a PatchData with the given cost/time/income; other fields are inert.
static PatchData make_patch(int buttons, int time, int income) {
    PatchData p{};
    p.id = 1;
    p.name = 'x';
    p.buttons = buttons;
    p.time = time;
    p.income = income;
    p.num_cells = 1;
    return p;
}

// Build a state whose only legal moves are exactly `n` affordable BuyPatch
// moves (from the front of the circle) plus the Advance move.
static SimplifiedGameState state_with_n_buyable(const GameSetup& setup, int n) {
    SimplifiedGameState state;
    state.player(0).set_buttons(127);  // afford anything
    for (int i = 0; i < 33; ++i) state.set_patch_available(i, false);
    int enabled = 0;
    for (int i = 0; i < 33 && enabled < n; ++i) {
        int patch_id = static_cast<int>(setup.circle()[static_cast<std::size_t>(i)]);
        state.set_patch_available(patch_id, true);
        ++enabled;
    }
    return state;
}

// ── Weight function tests (6.1) ────────────────────────────────────────────

TEST_CASE("weight_cheap: zero-cost patch has weight 1.0", "[biased_agent]") {
    REQUIRE(weight_cheap(make_patch(0, 1, 0)) == 1.0);
}

TEST_CASE("weight_cheap: lower cost has higher weight", "[biased_agent]") {
    REQUIRE(weight_cheap(make_patch(2, 1, 0)) > weight_cheap(make_patch(5, 1, 0)));
}

TEST_CASE("weight_income: zero-income patch has positive weight", "[biased_agent]") {
    REQUIRE(weight_income(make_patch(3, 2, 0)) > 0.0);
}

TEST_CASE("weight_income: higher income has higher weight", "[biased_agent]") {
    REQUIRE(weight_income(make_patch(3, 2, 3)) > weight_income(make_patch(3, 2, 1)));
}

TEST_CASE("weight_income_per_time: zero-income patch has positive weight", "[biased_agent]") {
    REQUIRE(weight_income_per_time(make_patch(3, 4, 0)) > 0.0);
}

TEST_CASE("weight_income_per_time: same time, higher income has higher weight", "[biased_agent]") {
    REQUIRE(weight_income_per_time(make_patch(3, 4, 2)) >
            weight_income_per_time(make_patch(3, 4, 1)));
}

// ── Distribution test (6.2) ────────────────────────────────────────────────

TEST_CASE("biased_random_move: higher-weight move selected > 60% of the time", "[biased_agent]") {
    auto setup = make_setup(0);
    SimplifiedGameState state = state_with_n_buyable(setup, 3);
    auto legal = legal_moves(state, setup);
    REQUIRE(legal.size() == 4);  // 3 BuyPatch + 1 Advance

    // Target the first buyable patch; weight it 9.0, everything else 1.0.
    int target_idx = -1;
    for (const auto& m : legal) {
        if (std::holds_alternative<BuyPatch>(m)) {
            target_idx = std::get<BuyPatch>(m).patch_index;
            break;
        }
    }
    REQUIRE(target_idx >= 0);
    int target_id = kPatches[static_cast<std::size_t>(target_idx)].id;
    std::function<double(const PatchData&)> weight_fn = [target_id](const PatchData& p) {
        return p.id == target_id ? 9.0 : 1.0;
    };

    std::mt19937 rng(123);
    const int N = 10000;
    int hits = 0;
    Move target_move = BuyPatch{target_idx};
    for (int i = 0; i < N; ++i) {
        if (biased_random_move(state, setup, rng, weight_fn, 1.0) == target_move) ++hits;
    }
    double frac = static_cast<double>(hits) / N;
    REQUIRE(frac > 0.60);
}

// ── Selected move is always legal ──────────────────────────────────────────

TEST_CASE("biased_random_move: returns a legal move", "[biased_agent]") {
    auto setup = make_setup(0);
    SimplifiedGameState state;
    std::mt19937 rng(7);
    std::function<double(const PatchData&)> weight_fn = weight_cheap;
    Move mv = biased_random_move(state, setup, rng, weight_fn, 1.0);
    auto legal = legal_moves(state, setup);
    bool found = false;
    for (const auto& m : legal) {
        if (m == mv) {
            found = true;
            break;
        }
    }
    REQUIRE(found);
}

// ── Reproducibility (6.3) ──────────────────────────────────────────────────

TEST_CASE("biased_random_move: same seed produces same move", "[biased_agent]") {
    auto setup = make_setup(0);
    SimplifiedGameState state;
    std::function<double(const PatchData&)> weight_fn = weight_cheap;
    std::mt19937 rng1(42);
    std::mt19937 rng2(42);
    Move mv1 = biased_random_move(state, setup, rng1, weight_fn, 1.0);
    Move mv2 = biased_random_move(state, setup, rng2, weight_fn, 1.0);
    REQUIRE(mv1 == mv2);
}

// ── Advance-weight behaviour (6.4) ─────────────────────────────────────────

TEST_CASE("biased_random_move: advance_weight 1.0 gives ~50/50 split", "[biased_agent]") {
    auto setup = make_setup(0);
    SimplifiedGameState state = state_with_n_buyable(setup, 1);
    auto legal = legal_moves(state, setup);
    REQUIRE(legal.size() == 2);  // 1 BuyPatch + 1 Advance

    std::function<double(const PatchData&)> weight_fn = [](const PatchData&) { return 1.0; };
    std::mt19937 rng(99);
    const int N = 10000;
    int advance_hits = 0;
    for (int i = 0; i < N; ++i) {
        if (biased_random_move(state, setup, rng, weight_fn, 1.0) == Move{Advance{}})
            ++advance_hits;
    }
    double frac = static_cast<double>(advance_hits) / N;
    REQUIRE(frac > 0.40);
    REQUIRE(frac < 0.60);
}

TEST_CASE("biased_random_move: larger advance_weight increases Advance frequency",
          "[biased_agent]") {
    auto setup = make_setup(0);
    SimplifiedGameState state = state_with_n_buyable(setup, 1);
    auto legal = legal_moves(state, setup);
    REQUIRE(legal.size() == 2);

    std::function<double(const PatchData&)> weight_fn = [](const PatchData&) { return 1.0; };
    std::mt19937 rng(99);
    const int N = 10000;
    int advance_hits = 0;
    for (int i = 0; i < N; ++i) {
        if (biased_random_move(state, setup, rng, weight_fn, 9.0) == Move{Advance{}})
            ++advance_hits;
    }
    double frac = static_cast<double>(advance_hits) / N;
    REQUIRE(frac > 0.60);
}

#ifndef NDEBUG
// Death test: a non-positive advance_weight must trip the assertion. Run in a
// forked child so the abort() does not take down the test runner. Guarded by
// NDEBUG because assertions are compiled out in release builds.
TEST_CASE("biased_random_move: non-positive advance_weight asserts", "[biased_agent]") {
    auto setup = make_setup(0);
    SimplifiedGameState state;
    std::function<double(const PatchData&)> weight_fn = weight_cheap;

    pid_t pid = fork();
    REQUIRE(pid >= 0);
    if (pid == 0) {
        // Child: silence stderr so the assertion message does not clutter output.
        std::fclose(stderr);
        std::mt19937 rng(1);
        Move mv = biased_random_move(state, setup, rng, weight_fn, 0.0);
        (void)mv;
        _exit(0);  // reached only if the assertion did NOT fire
    }
    int status = 0;
    waitpid(pid, &status, 0);
    REQUIRE(WIFSIGNALED(status));
    REQUIRE(WTERMSIG(status) == SIGABRT);
}
#endif  // NDEBUG
