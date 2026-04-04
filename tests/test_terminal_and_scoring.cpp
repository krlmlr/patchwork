#include <catch2/catch_test_macros.hpp>
#include "terminal_and_scoring.hpp"
#include "simplified_game_state.hpp"

using namespace patchwork;

TEST_CASE("is_terminal: false while a player is below 53", "[terminal_and_scoring]") {
    SimplifiedGameState s;
    s.player(0).set_position(52);
    s.player(1).set_position(53);
    REQUIRE(is_terminal(s) == false);
}

TEST_CASE("is_terminal: true when both players are done", "[terminal_and_scoring]") {
    SimplifiedGameState s;
    s.player(0).set_position(53);
    s.player(1).set_position(55);
    REQUIRE(is_terminal(s) == true);
}

TEST_CASE("score: no bonus, no free spaces", "[terminal_and_scoring]") {
    SimplifiedGameState s;
    s.player(0).set_position(53);
    s.player(1).set_position(53);
    s.player(0).set_buttons(20);
    s.player(0).set_free_spaces(0);
    REQUIRE(score(s, 0) == 20);
}

TEST_CASE("score: free spaces penalised at 2x", "[terminal_and_scoring]") {
    SimplifiedGameState s;
    s.player(0).set_position(53);
    s.player(1).set_position(53);
    s.player(0).set_buttons(10);
    s.player(0).set_free_spaces(5);
    REQUIRE(score(s, 0) == 0);
}

TEST_CASE("score: bonus tile adds 7", "[terminal_and_scoring]") {
    SimplifiedGameState s;
    s.player(0).set_position(53);
    s.player(1).set_position(53);
    s.player(0).set_buttons(10);
    s.player(0).set_free_spaces(0);
    s.set_bonus_status(BonusStatus::kPlayer0);
    REQUIRE(score(s, 0) == 17);
}

TEST_CASE("score: bonus not included for other player", "[terminal_and_scoring]") {
    SimplifiedGameState s;
    s.player(0).set_position(53);
    s.player(1).set_position(53);
    s.player(0).set_buttons(10);
    s.player(0).set_free_spaces(0);
    s.player(1).set_buttons(10);
    s.player(1).set_free_spaces(0);
    s.set_bonus_status(BonusStatus::kPlayer0);
    REQUIRE(score(s, 1) == 10);
}

TEST_CASE("winner: higher score wins", "[terminal_and_scoring]") {
    SimplifiedGameState s;
    s.player(0).set_position(53);
    s.player(1).set_position(53);
    s.player(0).set_buttons(15);
    s.player(0).set_free_spaces(0);
    s.player(1).set_buttons(12);
    s.player(1).set_free_spaces(0);
    REQUIRE(winner(s) == 0);
}

TEST_CASE("winner: equal scores resolved by first_to_finish", "[terminal_and_scoring]") {
    SimplifiedGameState s;
    s.player(0).set_position(53);
    s.player(1).set_position(53);
    s.player(0).set_buttons(10);
    s.player(0).set_free_spaces(0);
    s.player(1).set_buttons(10);
    s.player(1).set_free_spaces(0);
    s.set_first_to_finish(1);
    REQUIRE(winner(s) == 1);
}
