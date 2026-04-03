#include <catch2/catch_test_macros.hpp>
#include "game_setup.hpp"
#include "generated/game_setups.hpp"
#include "generated/patches.hpp"

#include <algorithm>
#include <array>
#include <set>
#include <sstream>
#include <string>
#include <string_view>

using patchwork::GameSetup;
using patchwork::kGameSetups;
using patchwork::kNumGameSetups;
using patchwork::kPatches;

TEST_CASE("GameSetup construction from string", "[game_setup]") {
    // Build the 33-char string from kPatches in order (identity permutation)
    std::string identity(33, ' ');
    for (int i = 0; i < 33; ++i)
        identity[i] = kPatches[i].name;

    GameSetup gs(identity);

    SECTION("circle contains each ID 0-32 exactly once") {
        std::array<bool, 33> seen{};
        for (uint8_t id : gs.circle()) {
            REQUIRE(id < 33);
            REQUIRE_FALSE(seen[id]);
            seen[id] = true;
        }
        for (int i = 0; i < 33; ++i)
            REQUIRE(seen[i]);
    }

    SECTION("two constructions from the same string are identical") {
        GameSetup gs2(identity);
        REQUIRE(gs.circle() == gs2.circle());
    }

    SECTION("identity string maps name at position i to ID i") {
        for (int i = 0; i < 33; ++i)
            REQUIRE(gs.circle()[i] == static_cast<uint8_t>(i));
    }
}

TEST_CASE("kGameSetups canonical setups", "[game_setup]") {
    SECTION("array has exactly 100 entries") {
        REQUIRE(kNumGameSetups == 100);
        REQUIRE(kGameSetups.size() == 100);
    }

    // Collect the expected set of 33 patch name chars
    std::set<char> expected_names;
    for (const auto& p : kPatches)
        expected_names.insert(p.name);

    for (std::size_t i = 0; i < kNumGameSetups; ++i) {
        std::string_view sv = kGameSetups[i];

        SECTION("each entry is exactly 33 characters") {
            REQUIRE(sv.size() == 33);
        }

        SECTION("each entry is a permutation of the 33 patch names") {
            std::set<char> chars(sv.begin(), sv.end());
            REQUIRE(chars == expected_names);
        }

        SECTION("last character is always '2' (two-square tile)") {
            REQUIRE(sv.back() == '2');
        }
    }
}

TEST_CASE("GameSetup to_ndjson serialisation", "[game_setup]") {
    GameSetup gs(kGameSetups[0]);

    std::ostringstream oss;
    gs.to_ndjson(oss);
    std::string line = oss.str();

    SECTION("output is a single newline-terminated line") {
        REQUIRE(line.back() == '\n');
        // Exactly one newline
        REQUIRE(std::count(line.begin(), line.end(), '\n') == 1);
    }

    SECTION("output contains type:setup") {
        REQUIRE(line.find(R"("type":"setup")") != std::string::npos);
    }

    SECTION("circle field is a 33-char string equal to the input") {
        // Find the circle value between quotes after "circle":"
        auto pos = line.find(R"("circle":")");
        REQUIRE(pos != std::string::npos);
        pos += std::string_view(R"("circle":")").size();
        std::string_view circle_val(line.data() + pos, 33);
        REQUIRE(circle_val == kGameSetups[0]);
    }
}
