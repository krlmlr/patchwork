#include <catch2/catch_test_macros.hpp>

#include <stdexcept>
#include <vector>

#include "setup_range.hpp"

using namespace patchwork;

TEST_CASE("single ids parse and preserve order", "[setups]") {
    REQUIRE(parse_setups("0") == std::vector<int>{0});
    REQUIRE(parse_setups("3,7,1") == std::vector<int>{3, 7, 1});
}

TEST_CASE("inclusive ranges expand", "[setups]") {
    REQUIRE(parse_setups("5-9") == std::vector<int>{5, 6, 7, 8, 9});
    REQUIRE(parse_setups("0-0") == std::vector<int>{0});
}

TEST_CASE("ranges and singles combine in one spec", "[setups]") {
    REQUIRE(parse_setups("0,2,5-9") == std::vector<int>{0, 2, 5, 6, 7, 8, 9});
    REQUIRE(parse_setups("1-3,10,20-21") == std::vector<int>{1, 2, 3, 10, 20, 21});
}

TEST_CASE("surrounding whitespace is ignored", "[setups]") {
    REQUIRE(parse_setups(" 0 , 2 , 5 - 9 ") == std::vector<int>{0, 2, 5, 6, 7, 8, 9});
}

TEST_CASE("malformed specs throw invalid_argument", "[setups]") {
    REQUIRE_THROWS_AS(parse_setups(""), std::invalid_argument);
    REQUIRE_THROWS_AS(parse_setups("a"), std::invalid_argument);
    REQUIRE_THROWS_AS(parse_setups("9-5"), std::invalid_argument);
    REQUIRE_THROWS_AS(parse_setups("1,,2"), std::invalid_argument);
    REQUIRE_THROWS_AS(parse_setups("-3"), std::invalid_argument);
    REQUIRE_THROWS_AS(parse_setups("1-"), std::invalid_argument);
    REQUIRE_THROWS_AS(parse_setups("5-9-3"), std::invalid_argument);
}
