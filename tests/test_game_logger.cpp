#include <catch2/catch_test_macros.hpp>
#include "game_logger.hpp"
#include "terminal_and_scoring.hpp"

#include <sstream>
#include <string>

using namespace patchwork;

static bool contains(const std::string& s, const std::string& sub) {
    return s.find(sub) != std::string::npos;
}

TEST_CASE("log_game_start: well-formed JSON with event field", "[game_logger]") {
    SimplifiedGameState state;
    std::ostringstream oss;
    log_game_start(oss, 42, 7, state);
    auto line = oss.str();
    REQUIRE(line.back() == '\n');
    REQUIRE(contains(line, R"("event":"game_start")"));
}

TEST_CASE("log_game_start: records seed and setup_id", "[game_logger]") {
    SimplifiedGameState state;
    std::ostringstream oss;
    log_game_start(oss, 42, 7, state);
    auto line = oss.str();
    REQUIRE(contains(line, R"("seed":42)"));
    REQUIRE(contains(line, R"("setup_id":7)"));
}

TEST_CASE("log_move: BuyPatch fields present", "[game_logger]") {
    SimplifiedGameState state;
    std::ostringstream oss;
    Move mv = BuyPatch{5};
    log_move(oss, 0, 0, mv, state);
    auto line = oss.str();
    REQUIRE(contains(line, R"("event":"move")"));
    REQUIRE(contains(line, R"("player":0)"));
    REQUIRE(contains(line, R"("move_type":"buy_patch")"));
    REQUIRE(contains(line, R"("patch_index":5)"));
}

TEST_CASE("log_move: Advance fields present", "[game_logger]") {
    SimplifiedGameState state;
    std::ostringstream oss;
    Move mv = Advance{};
    log_move(oss, 1, 1, mv, state);
    auto line = oss.str();
    REQUIRE(contains(line, R"("event":"move")"));
    REQUIRE(contains(line, R"("player":1)"));
    REQUIRE(contains(line, R"("move_type":"advance")"));
}

TEST_CASE("log_move: ply sequence increments", "[game_logger]") {
    SimplifiedGameState state;
    std::ostringstream oss;
    for (int i = 0; i < 5; ++i) {
        Move mv = Advance{};
        log_move(oss, i, 0, mv, state);
    }
    auto s = oss.str();
    REQUIRE(contains(s, R"("ply":0)"));
    REQUIRE(contains(s, R"("ply":1)"));
    REQUIRE(contains(s, R"("ply":4)"));
}

TEST_CASE("log_game_end: records scores and winner", "[game_logger]") {
    SimplifiedGameState state;
    state.player(0).set_position(53);
    state.player(1).set_position(53);
    state.player(0).set_buttons(18);
    state.player(0).set_free_spaces(0);
    state.player(1).set_buttons(14);
    state.player(1).set_free_spaces(0);
    std::ostringstream oss;
    log_game_end(oss, state);
    auto line = oss.str();
    REQUIRE(contains(line, R"("event":"game_end")"));
    REQUIRE(contains(line, R"("score_p0":18)"));
    REQUIRE(contains(line, R"("score_p1":14)"));
    REQUIRE(contains(line, R"("winner":0)"));
}

TEST_CASE("log_game_end: line ends with newline", "[game_logger]") {
    SimplifiedGameState state;
    state.player(0).set_position(53);
    state.player(1).set_position(53);
    std::ostringstream oss;
    log_game_end(oss, state);
    REQUIRE(oss.str().back() == '\n');
}

TEST_CASE("log_game_start: each line is independently parseable JSON object", "[game_logger]") {
    SimplifiedGameState state;
    std::ostringstream oss;
    log_game_start(oss, 1, 0, state);
    auto line = oss.str();
    // Remove trailing newline and check it starts with { and ends with }
    auto trimmed = line.substr(0, line.size() - 1);
    REQUIRE(trimmed.front() == '{');
    REQUIRE(trimmed.back() == '}');
}
