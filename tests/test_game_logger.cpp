#include <catch2/catch_test_macros.hpp>
#include "game_logger.hpp"
#include "game_setups.hpp"
#include "generated/patches.hpp"
#include "move_application.hpp"
#include "terminal_and_scoring.hpp"

#include <sstream>
#include <string>

using namespace patchwork;

static bool contains(const std::string& s, const std::string& sub) {
    return s.find(sub) != std::string::npos;
}

TEST_CASE("log_game_start: well-formed JSON with event field", "[game_logger]") {
    SimplifiedGameState state;
    GameSetup setup = make_setup(0);
    std::ostringstream oss;
    log_game_start(oss, 42, 7, state, setup);
    auto line = oss.str();
    REQUIRE(line.back() == '\n');
    REQUIRE(contains(line, R"("event":"game_start")"));
}

TEST_CASE("log_game_start: records seed and setup_id", "[game_logger]") {
    SimplifiedGameState state;
    GameSetup setup = make_setup(0);
    std::ostringstream oss;
    log_game_start(oss, 42, 7, state, setup);
    auto line = oss.str();
    REQUIRE(contains(line, R"("seed":42)"));
    REQUIRE(contains(line, R"("setup_id":7)"));
}

TEST_CASE("log_game_start: circle string is 33 characters", "[game_logger]") {
    SimplifiedGameState state;
    GameSetup setup = make_setup(0);
    std::ostringstream oss;
    log_game_start(oss, 1, 0, state, setup);
    auto line = oss.str();
    // Find "circle":"..." and check value length
    auto pos = line.find(R"("circle":")");
    REQUIRE(pos != std::string::npos);
    pos += std::string(R"("circle":")").size();
    auto end = line.find('"', pos);
    REQUIRE(end != std::string::npos);
    REQUIRE(end - pos == 33);
}

TEST_CASE("log_game_start: circle string ends with '2' (neutral token convention)", "[game_logger]") {
    SimplifiedGameState state;
    GameSetup setup = make_setup(0);
    std::ostringstream oss;
    log_game_start(oss, 1, 0, state, setup);
    auto line = oss.str();
    auto pos = line.find(R"("circle":")");
    REQUIRE(pos != std::string::npos);
    pos += std::string(R"("circle":")").size();
    auto end = line.find('"', pos);
    REQUIRE(end != std::string::npos);
    REQUIRE(end - pos == 33);
    REQUIRE(line[end - 1] == '2');
}

TEST_CASE("log_game_start: circle encodes patch names not IDs", "[game_logger]") {
    SimplifiedGameState state;
    GameSetup setup = make_setup(0);
    std::ostringstream oss;
    log_game_start(oss, 1, 0, state, setup);
    auto line = oss.str();
    // Each character in the circle string must be a valid patch name from kPatches
    auto pos = line.find(R"("circle":")");
    REQUIRE(pos != std::string::npos);
    pos += std::string(R"("circle":")").size();
    auto end = line.find('"', pos);
    REQUIRE(end != std::string::npos);
    for (std::size_t i = pos; i < end; ++i) {
        char c = line[i];
        bool found = false;
        for (const auto& p : kPatches) {
            if (p.name == c) { found = true; break; }
        }
        REQUIRE(found);
    }
}

TEST_CASE("log_move: BuyPatch fields present", "[game_logger]") {
    SimplifiedGameState state;
    GameSetup setup = make_setup(0);
    std::ostringstream oss;
    Move mv = BuyPatch{5};
    log_move(oss, 0, 0, mv, state, setup);
    auto line = oss.str();
    REQUIRE(contains(line, R"("event":"move")"));
    REQUIRE(contains(line, R"("player":0)"));
    REQUIRE(contains(line, R"("move_type":"buy_patch")"));
    REQUIRE(contains(line, R"("patch_index":5)"));
    // kPatches[5].name = '4'
    REQUIRE(contains(line, R"("patch_symbol":"4")"));
}

TEST_CASE("log_move: Advance fields present", "[game_logger]") {
    SimplifiedGameState state;
    GameSetup setup = make_setup(0);
    std::ostringstream oss;
    Move mv = Advance{};
    log_move(oss, 1, 1, mv, state, setup);
    auto line = oss.str();
    REQUIRE(contains(line, R"("event":"move")"));
    REQUIRE(contains(line, R"("player":1)"));
    REQUIRE(contains(line, R"("move_type":"advance")"));
}

TEST_CASE("log_move: ply sequence increments", "[game_logger]") {
    SimplifiedGameState state;
    GameSetup setup = make_setup(0);
    std::ostringstream oss;
    for (int i = 0; i < 5; ++i) {
        Move mv = Advance{};
        log_move(oss, i, 0, mv, state, setup);
    }
    auto s = oss.str();
    REQUIRE(contains(s, R"("ply":0)"));
    REQUIRE(contains(s, R"("ply":1)"));
    REQUIRE(contains(s, R"("ply":4)"));
}

TEST_CASE("log_move: income and free_spaces present", "[game_logger]") {
    SimplifiedGameState state;
    GameSetup setup = make_setup(0);
    std::ostringstream oss;
    Move mv = Advance{};
    log_move(oss, 0, 0, mv, state, setup);
    auto line = oss.str();
    REQUIRE(contains(line, R"("income":)"));
    REQUIRE(contains(line, R"("free_spaces":)"));
}

TEST_CASE("log_move: board_value equals buttons minus 2x free_spaces", "[game_logger]") {
    SimplifiedGameState state;
    // Fresh player: buttons=5, free_spaces=81 -> board_value = 5 - 2*81 = -157
    GameSetup setup = make_setup(0);
    std::ostringstream oss;
    Move mv = Advance{};
    log_move(oss, 0, 0, mv, state, setup);
    auto line = oss.str();
    REQUIRE(contains(line, R"("board_value":-157)"));
}

TEST_CASE("log_move: projected_income and projected_score present", "[game_logger]") {
    SimplifiedGameState state;
    GameSetup setup = make_setup(0);
    std::ostringstream oss;
    Move mv = Advance{};
    log_move(oss, 0, 0, mv, state, setup);
    auto line = oss.str();
    REQUIRE(contains(line, R"("projected_income":)"));
    REQUIRE(contains(line, R"("projected_score":)"));
}

TEST_CASE("log_move: projected_income equals income times remaining triggers", "[game_logger]") {
    SimplifiedGameState state;
    // Set income=2 at position=0 (all 9 income spaces remain)
    // projected_income = 2 * 9 = 18, projected_score = 5 + 18 - 2*81 = -139
    state.player(0).set_income(2);
    GameSetup setup = make_setup(0);
    std::ostringstream oss;
    Move mv = Advance{};
    log_move(oss, 0, 0, mv, state, setup);
    auto line = oss.str();
    REQUIRE(contains(line, R"("projected_income":18)"));
    REQUIRE(contains(line, R"("projected_score":-139)"));
}

TEST_CASE("log_move: projected_income is 0 at or past last income space", "[game_logger]") {
    SimplifiedGameState state;
    // Position >= 53 means no income spaces remain
    state.player(0).set_position(53);
    state.player(0).set_income(5);
    state.player(1).set_position(53);
    GameSetup setup = make_setup(0);
    std::ostringstream oss;
    Move mv = Advance{};
    log_move(oss, 0, 0, mv, state, setup);
    auto line = oss.str();
    REQUIRE(contains(line, R"("projected_income":0)"));
}

TEST_CASE("log_move: circle shrinks after buy_patch move", "[game_logger]") {
    GameSetup setup = make_setup(0);
    SimplifiedGameState state;
    // With make_setup(0), circle[0] = kGameSetups[0][0] = 'Z' (kPatches index 25, buttons=4).
    // kPatches[25].buttons=4 is affordable with the default 5-button start.
    Move mv = BuyPatch{25};  // kPatches[25]='Z', the patch at circle position 0
    SimplifiedGameState new_state = apply_move(state, mv, setup);
    std::ostringstream oss;
    log_move(oss, 0, 0, mv, new_state, setup);
    auto line = oss.str();
    // The circle in the move log should have fewer chars than 33 (one patch removed)
    auto pos = line.find(R"("circle":")");
    REQUIRE(pos != std::string::npos);
    pos += std::string(R"("circle":")").size();
    auto end = line.find('"', pos);
    REQUIRE(end != std::string::npos);
    REQUIRE(end - pos < 33);
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

TEST_CASE("log_game_end: contains income and free_spaces for both players", "[game_logger]") {
    SimplifiedGameState state;
    state.player(0).set_position(53);
    state.player(1).set_position(53);
    state.player(0).set_income(3);
    state.player(0).set_free_spaces(10);
    state.player(1).set_income(2);
    state.player(1).set_free_spaces(5);
    std::ostringstream oss;
    log_game_end(oss, state);
    auto line = oss.str();
    REQUIRE(contains(line, R"("p0":{"income":3,"free_spaces":10})"));
    REQUIRE(contains(line, R"("p1":{"income":2,"free_spaces":5})"));
}

TEST_CASE("log_game_start: each line is independently parseable JSON object", "[game_logger]") {
    SimplifiedGameState state;
    GameSetup setup = make_setup(0);
    std::ostringstream oss;
    log_game_start(oss, 1, 0, state, setup);
    auto line = oss.str();
    // Remove trailing newline and check it starts with { and ends with }
    auto trimmed = line.substr(0, line.size() - 1);
    REQUIRE(trimmed.front() == '{');
    REQUIRE(trimmed.back() == '}');
}
