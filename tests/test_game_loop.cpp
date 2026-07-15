#include <catch2/catch_test_macros.hpp>

#include <sstream>
#include <string>

#include "game_loop.hpp"
#include "game_setups.hpp"
#include "seed_util.hpp"
#include "terminal_and_scoring.hpp"

using namespace patchwork;

TEST_CASE("play_game reaches a terminal state with a valid winner and positive plies", "[loop]") {
    auto setup = make_setup(0);
    GameOutcome r = play_game(setup, 0, derive_seed(1, 0, 0), nullptr);
    REQUIRE(is_terminal(r.state));
    REQUIRE(r.plies > 0);
    int w = winner(r.state);
    REQUIRE((w == 0 || w == 1));
}

TEST_CASE("play_game is reproducible for the same seed", "[loop]") {
    auto setup = make_setup(3);
    long long seed = derive_seed(99, 3, 5);
    GameOutcome a = play_game(setup, 3, seed, nullptr);
    GameOutcome b = play_game(setup, 3, seed, nullptr);
    REQUIRE(a.plies == b.plies);
    REQUIRE(score(a.state, 0) == score(b.state, 0));
    REQUIRE(score(a.state, 1) == score(b.state, 1));
    REQUIRE(winner(a.state) == winner(b.state));
}

TEST_CASE("full-mode game_end scores match the returned terminal state", "[loop]") {
    auto setup = make_setup(2);
    long long seed = derive_seed(7, 2, 4);
    std::ostringstream full;
    GameOutcome r = play_game(setup, 2, seed, &full);
    std::string s = full.str();

    const int sp0 = score(r.state, 0);
    const int sp1 = score(r.state, 1);
    const int w = winner(r.state);

    REQUIRE(s.find(R"("event":"game_start")") != std::string::npos);
    REQUIRE(s.find(R"("event":"game_end")") != std::string::npos);
    REQUIRE(s.find(R"("score_p0":)" + std::to_string(sp0)) != std::string::npos);
    REQUIRE(s.find(R"("score_p1":)" + std::to_string(sp1)) != std::string::npos);
    REQUIRE(s.find(R"("winner":)" + std::to_string(w)) != std::string::npos);
}

TEST_CASE("full-mode log has exactly one game_start and one game_end", "[loop]") {
    auto setup = make_setup(1);
    std::ostringstream full;
    play_game(setup, 1, derive_seed(5, 1, 0), &full);
    std::string s = full.str();

    auto count = [&](const std::string& needle) {
        int n = 0;
        for (std::size_t p = s.find(needle); p != std::string::npos; p = s.find(needle, p + 1)) ++n;
        return n;
    };
    REQUIRE(count(R"("event":"game_start")") == 1);
    REQUIRE(count(R"("event":"game_end")") == 1);
    REQUIRE(count(R"("event":"move")") > 0);
}
