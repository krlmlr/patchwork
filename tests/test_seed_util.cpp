#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <set>

#include "seed_util.hpp"

using namespace patchwork;

TEST_CASE("derive_seed is deterministic for fixed inputs", "[seed]") {
    REQUIRE(derive_seed(42, 3, 7) == derive_seed(42, 3, 7));
    REQUIRE(derive_seed(0, 0, 0) == derive_seed(0, 0, 0));
}

TEST_CASE("derive_seed depends on every coordinate", "[seed]") {
    REQUIRE(derive_seed(42, 3, 7) != derive_seed(42, 3, 8));
    REQUIRE(derive_seed(42, 3, 7) != derive_seed(42, 4, 7));
    REQUIRE(derive_seed(42, 3, 7) != derive_seed(43, 3, 7));
}

TEST_CASE("derive_seed is a pure function, independent of batch order/size", "[seed]") {
    // The seed for (setup 3, game 7) is fixed no matter which other setups or
    // how many games surround it: there is no shared state to perturb it.
    const std::uint32_t reference = derive_seed(99, 3, 7);
    for (int other_setup = 0; other_setup < 20; ++other_setup) {
        for (int other_game = 0; other_game < 20; ++other_game) {
            (void)derive_seed(99, other_setup, other_game);  // "run" the neighbours
        }
    }
    REQUIRE(derive_seed(99, 3, 7) == reference);
}

TEST_CASE("derive_seed has negligible collisions over a 10k grid", "[seed]") {
    std::set<std::uint32_t> seen;
    int collisions = 0;
    for (int s = 0; s < 10; ++s) {
        for (int g = 0; g < 1000; ++g) {
            if (!seen.insert(derive_seed(42, s, g)).second) ++collisions;
        }
    }
    // 10 000 draws from a 2^32 space: expected collisions well under 1.
    REQUIRE(collisions < 3);
}
