#include <catch2/catch_test_macros.hpp>
#include <functional>
#include <random>

#include "agent.hpp"
#include "agent_strategy.hpp"
#include "biased_random_agent.hpp"
#include "game_setups.hpp"
#include "random_agent.hpp"

using namespace patchwork;

// ── parse_strategy (6.6) ───────────────────────────────────────────────────

TEST_CASE("parse_strategy: valid names map to enum values", "[agent]") {
    REQUIRE(parse_strategy("random") == AgentStrategy::Random);
    REQUIRE(parse_strategy("cheap") == AgentStrategy::Cheap);
    REQUIRE(parse_strategy("income") == AgentStrategy::Income);
    REQUIRE(parse_strategy("income-per-time") == AgentStrategy::IncomePerTime);
}

TEST_CASE("parse_strategy: unknown name returns nullopt", "[agent]") {
    REQUIRE_FALSE(parse_strategy("best").has_value());
    REQUIRE_FALSE(parse_strategy("").has_value());
}

TEST_CASE("strategy_name: round-trips through parse_strategy", "[agent]") {
    for (AgentStrategy s : {AgentStrategy::Random, AgentStrategy::Cheap, AgentStrategy::Income,
                            AgentStrategy::IncomePerTime}) {
        REQUIRE(parse_strategy(strategy_name(s)) == s);
    }
}

// ── make_weight_fn (agents spec) ───────────────────────────────────────────

TEST_CASE("make_weight_fn: non-null for biased strategies", "[agent]") {
    REQUIRE(static_cast<bool>(make_weight_fn(AgentStrategy::Cheap)));
    REQUIRE(static_cast<bool>(make_weight_fn(AgentStrategy::Income)));
    REQUIRE(static_cast<bool>(make_weight_fn(AgentStrategy::IncomePerTime)));
}

TEST_CASE("make_weight_fn: null for Random", "[agent]") {
    REQUIRE_FALSE(static_cast<bool>(make_weight_fn(AgentStrategy::Random)));
}

// ── select_move dispatch (6.5) ─────────────────────────────────────────────

TEST_CASE("select_move: Random delegates to random_move", "[agent]") {
    auto setup = make_setup(0);
    SimplifiedGameState state;
    std::mt19937 rng1(5);
    std::mt19937 rng2(5);
    Move via_select = select_move(state, setup, rng1, AgentStrategy::Random, 1.0);
    Move via_random = random_move(state, setup, rng2);
    REQUIRE(via_select == via_random);
}

TEST_CASE("select_move: Cheap delegates to biased_random_move with weight_cheap", "[agent]") {
    auto setup = make_setup(0);
    SimplifiedGameState state;
    std::function<double(const PatchData&)> weight_fn = weight_cheap;
    std::mt19937 rng1(5);
    std::mt19937 rng2(5);
    Move via_select = select_move(state, setup, rng1, AgentStrategy::Cheap, 1.0);
    Move via_biased = biased_random_move(state, setup, rng2, weight_fn, 1.0);
    REQUIRE(via_select == via_biased);
}
