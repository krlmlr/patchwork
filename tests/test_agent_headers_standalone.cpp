// Verifies the "headers compile independently" requirement: `agent_strategy.hpp`
// and `biased_random_agent.hpp` must be includable without pulling in the logger
// or play driver. This translation unit intentionally includes ONLY those two
// headers (each brings its own type dependencies) plus the test framework.
#include <catch2/catch_test_macros.hpp>

#include "agent_strategy.hpp"
#include "biased_random_agent.hpp"

TEST_CASE("agent headers compile and link standalone", "[agent_headers]") {
    // Reference a symbol from each header so the TU is not optimised away.
    auto fn = patchwork::make_weight_fn(patchwork::AgentStrategy::Cheap);
    REQUIRE(static_cast<bool>(fn));
    patchwork::PatchData p{};
    p.buttons = 0;
    REQUIRE(patchwork::weight_cheap(p) == 1.0);
}
