#include <catch2/catch_test_macros.hpp>
#include "move.hpp"

using patchwork::BuyPatch;
using patchwork::Advance;
using patchwork::Move;

TEST_CASE("BuyPatch carries patch index", "[move]") {
    Move m = BuyPatch{7};
    REQUIRE(std::holds_alternative<BuyPatch>(m));
    REQUIRE(std::get<BuyPatch>(m).patch_index == 7);
}

TEST_CASE("BuyPatch equality", "[move]") {
    REQUIRE(BuyPatch{3} == BuyPatch{3});
    REQUIRE(!(BuyPatch{3} == BuyPatch{4}));
}

TEST_CASE("Advance equality", "[move]") {
    REQUIRE(Advance{} == Advance{});
}

TEST_CASE("Advance and BuyPatch are distinct variants", "[move]") {
    Move a = Advance{};
    Move b = BuyPatch{0};
    REQUIRE(a != b);
}
