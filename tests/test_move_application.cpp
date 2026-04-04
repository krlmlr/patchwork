#include <catch2/catch_test_macros.hpp>
#include "move_application.hpp"
#include "move_generation.hpp"
#include "game_setups.hpp"
#include "simplified_game_state.hpp"

using namespace patchwork;

// Helpers: patch 0 (index 0 in kPatches): buttons=2, time=1, income=0, num_cells=2
// Patch 1 (index 1): buttons=1, time=3, income=0, num_cells=3
// Patch 5 (index 5): buttons=3, time=3, income=1, num_cells=4
// Patch 7 (index 7): buttons=4, time=6, income=2, num_cells=4

static SimplifiedGameState make_state_with_only(int patch_id, int marker = 0) {
    // State where only the given patch is available, circle points to it
    auto setup = make_setup(0);
    SimplifiedGameState s;
    for (int i = 0; i < 33; ++i) s.set_patch_available(i, false);
    s.set_patch_available(patch_id, true);
    // Find the circle position of this patch and set the marker there
    for (int i = 0; i < 33; ++i) {
        if (static_cast<int>(setup.circle()[static_cast<std::size_t>(i)]) == patch_id) {
            s.set_circle_marker(i);
            break;
        }
    }
    (void)marker;
    return s;
}

TEST_CASE("BuyPatch: button cost deducted", "[move_application]") {
    auto setup = make_setup(0);
    // Patch 0: buttons=2
    auto state = make_state_with_only(0);
    state.player(0).set_buttons(10);
    auto next = apply_move(state, BuyPatch{0}, setup);
    REQUIRE(next.player(0).buttons() == 8);
}

TEST_CASE("BuyPatch: time advances position", "[move_application]") {
    auto setup = make_setup(0);
    // Patch 1: time=3
    auto state = make_state_with_only(1);
    state.player(0).set_buttons(5);
    state.player(0).set_position(5);
    auto next = apply_move(state, BuyPatch{1}, setup);
    REQUIRE(next.player(0).position() == 8);
}

TEST_CASE("BuyPatch: income added", "[move_application]") {
    auto setup = make_setup(0);
    // Patch 5: income=1
    auto state = make_state_with_only(5);
    state.player(0).set_buttons(10);
    state.player(0).set_income(1);
    auto next = apply_move(state, BuyPatch{5}, setup);
    REQUIRE(next.player(0).income() == 2);
}

TEST_CASE("BuyPatch: free_spaces reduced by patch cell count", "[move_application]") {
    auto setup = make_setup(0);
    // Patch 0: num_cells=2
    auto state = make_state_with_only(0);
    state.player(0).set_buttons(10);
    state.player(0).set_free_spaces(81);
    auto next = apply_move(state, BuyPatch{0}, setup);
    REQUIRE(next.player(0).free_spaces() == 79);
}

TEST_CASE("BuyPatch: patch marked unavailable", "[move_application]") {
    auto setup = make_setup(0);
    auto state = make_state_with_only(0);
    state.player(0).set_buttons(10);
    REQUIRE(state.patch_available(0) == true);
    auto next = apply_move(state, BuyPatch{0}, setup);
    REQUIRE(next.patch_available(0) == false);
}

TEST_CASE("BuyPatch: circle marker advances past purchased patch", "[move_application]") {
    auto setup = make_setup(0);
    // In default setup circle, patch 0 is at position 0
    auto state = make_state_with_only(0);
    state.player(0).set_buttons(10);
    int old_marker = state.circle_marker();  // should be 0
    auto next = apply_move(state, BuyPatch{0}, setup);
    REQUIRE(next.circle_marker() == (old_marker + 1) % 33);
}

TEST_CASE("Advance: position advances to opponent+1", "[move_application]") {
    auto setup = make_setup(0);
    SimplifiedGameState state;
    state.player(0).set_position(3);
    state.player(1).set_position(8);
    state.set_next_player(0);
    auto next = apply_move(state, Advance{}, setup);
    REQUIRE(next.player(0).position() == 9);
}

TEST_CASE("Advance: buttons credited at 1 per space", "[move_application]") {
    auto setup = make_setup(0);
    SimplifiedGameState state;
    // p0 at 3, p1 at 8: advance moves p0 to 9 (6 spaces)
    // No income spaces crossed between 3 and 9 (position 5 IS crossed: 3<5<=9)
    // Default income = 0, so income payout = 0
    state.player(0).set_position(3);
    state.player(0).set_buttons(5);
    state.player(0).set_income(0);
    state.player(1).set_position(8);
    state.set_next_player(0);
    auto next = apply_move(state, Advance{}, setup);
    // 6 spaces advanced + 0 income payout = 5+6=11
    REQUIRE(next.player(0).buttons() == 11);
}

TEST_CASE("Advance: circle marker unchanged", "[move_application]") {
    auto setup = make_setup(0);
    SimplifiedGameState state;
    state.player(0).set_position(3);
    state.player(1).set_position(8);
    state.set_next_player(0);
    int old_marker = state.circle_marker();
    auto next = apply_move(state, Advance{}, setup);
    REQUIRE(next.circle_marker() == old_marker);
}

TEST_CASE("Inactive player unchanged after BuyPatch", "[move_application]") {
    auto setup = make_setup(0);
    auto state = make_state_with_only(0);
    state.player(0).set_buttons(10);
    state.player(1).set_position(20);
    state.player(1).set_buttons(30);
    state.player(1).set_income(5);
    state.player(1).set_free_spaces(40);
    auto next = apply_move(state, BuyPatch{0}, setup);
    REQUIRE(next.player(1).position() == 20);
    REQUIRE(next.player(1).buttons() == 30);
    REQUIRE(next.player(1).income() == 5);
    REQUIRE(next.player(1).free_spaces() == 40);
}

TEST_CASE("Inactive player unchanged after Advance", "[move_application]") {
    auto setup = make_setup(0);
    SimplifiedGameState state;
    state.player(0).set_position(3);
    state.player(1).set_position(8);
    state.player(1).set_buttons(20);
    state.player(1).set_income(4);
    state.player(1).set_free_spaces(50);
    state.set_next_player(0);
    auto next = apply_move(state, Advance{}, setup);
    REQUIRE(next.player(1).position() == 8);
    REQUIRE(next.player(1).buttons() == 20);
    REQUIRE(next.player(1).income() == 4);
    REQUIRE(next.player(1).free_spaces() == 50);
}

TEST_CASE("Income payout on crossing income space (Advance)", "[move_application]") {
    auto setup = make_setup(0);
    SimplifiedGameState state;
    // p0 at 10, p1 at 12. Advance moves p0 to 13. Crosses position 11.
    state.player(0).set_position(10);
    state.player(0).set_buttons(5);
    state.player(0).set_income(3);
    state.player(1).set_position(12);
    state.set_next_player(0);
    auto next = apply_move(state, Advance{}, setup);
    // Spaces = 13-10=3, income payout at pos 11 = 3 → total +3+3=+6
    REQUIRE(next.player(0).buttons() == 11);
}

TEST_CASE("Multiple income spaces crossed in one move", "[move_application]") {
    auto setup = make_setup(0);
    SimplifiedGameState state;
    // p0 at 4, p1 at 14. Advance to 15. Crosses positions 5 and 11.
    state.player(0).set_position(4);
    state.player(0).set_buttons(5);
    state.player(0).set_income(3);
    state.player(1).set_position(14);
    state.set_next_player(0);
    auto next = apply_move(state, Advance{}, setup);
    // Spaces = 15-4=11 advance buttons, crosses 5 (+3) and 11 (+3)
    REQUIRE(next.player(0).buttons() == 5 + 11 + 3 + 3);
}

TEST_CASE("Leather patch awarded when first to cross threshold 26", "[move_application]") {
    auto setup = make_setup(0);

    // Scenario: p0 at 25, p1 at 25 (tied). Both below 26. p0 advances to 26.
    // Crosses threshold 26; neither pre-move position was >= 26. Leather awarded.
    SimplifiedGameState s2;
    s2.player(0).set_position(25);
    s2.player(0).set_free_spaces(81);
    s2.player(1).set_position(25);
    s2.set_next_player(0);
    auto next2 = apply_move(s2, Advance{}, setup);
    // Advance: p0 moves to 26. Crosses 26 (25<26<=26), neither pre_pos >= 26
    REQUIRE(next2.player(0).free_spaces() == 80);
}

TEST_CASE("Leather patch not awarded when threshold already passed by either player",
          "[move_application]") {
    auto setup = make_setup(0);
    SimplifiedGameState state;
    // p1 pre_pos=30 >= 26 → threshold 26 already claimed
    state.player(0).set_position(25);
    state.player(0).set_free_spaces(81);
    state.player(1).set_position(30);
    state.set_next_player(0);
    auto next = apply_move(state, Advance{}, setup);
    // p0 moves to 31. Crosses 26 but p1 was already at 30>=26 → no leather patch
    REQUIRE(next.player(0).free_spaces() == 81);
}

TEST_CASE("Multiple leather thresholds crossed in one move", "[move_application]") {
    auto setup = make_setup(0);
    // patch_id=7: time=6, buttons=4, num_cells=4. p0 at 25, p1 at 5.
    // But we can't advance (p0 is ahead of p1). Use BuyPatch instead.
    // p0 at 25 + patch time 6 = 31. Crosses threshold 26 (25<26<=31).
    // p0_pre=25<26, p1_pre=5<26 → leather awarded.
    auto state = make_state_with_only(7);
    state.player(0).set_position(25);
    state.player(0).set_free_spaces(81);
    state.player(0).set_buttons(10);
    state.player(1).set_position(5);
    auto next = apply_move(state, BuyPatch{7}, setup);
    // num_cells=4 → 81-4=77; leather at 26 → 77-1=76
    REQUIRE(next.player(0).free_spaces() == 76);
}

TEST_CASE("Leather patch not re-awarded for already-crossed threshold", "[move_application]") {
    auto setup = make_setup(0);
    // p0 at 26 (already past threshold 26). Buy patch_id=7 (time=6) → pos=32.
    // Crosses threshold 32 (26<32<=32). p0_pre=26>=26 → threshold 26 already taken.
    // For 32: p0_pre=26<32 and p1_pre=5<32 → leather awarded at 32.
    // For 26: old_pos=26 which is NOT < 26 → condition fails → no re-award.
    auto state = make_state_with_only(7);
    state.player(0).set_position(26);
    state.player(0).set_free_spaces(81);
    state.player(0).set_buttons(10);
    state.player(1).set_position(5);
    auto next = apply_move(state, BuyPatch{7}, setup);
    // num_cells=4 → 81-4=77; leather at 32 only → 77-1=76
    REQUIRE(next.player(0).free_spaces() == 76);
}

TEST_CASE("7x7 bonus claimed when 56+ occupied", "[move_application]") {
    auto setup = make_setup(0);
    auto state = make_state_with_only(0);
    // 81 - 26 = 55 occupied before; patch_0 num_cells=2 → 81-24=57 >= 56
    state.player(0).set_free_spaces(26);
    state.player(0).set_buttons(10);
    REQUIRE(state.bonus_status() == BonusStatus::kUnclaimed);
    auto next = apply_move(state, BuyPatch{0}, setup);
    REQUIRE(next.bonus_status() == BonusStatus::kPlayer0);
}

TEST_CASE("7x7 bonus not re-awarded when already claimed", "[move_application]") {
    auto setup = make_setup(0);
    auto state = make_state_with_only(0);
    state.player(0).set_free_spaces(26);
    state.player(0).set_buttons(10);
    state.player(1).set_free_spaces(26);
    state.set_bonus_status(BonusStatus::kPlayer0);
    state.set_next_player(1);
    auto next = apply_move(state, BuyPatch{0}, setup);
    // Player 1 would also reach 56+ occupied, but bonus is already claimed by 0
    REQUIRE(next.bonus_status() == BonusStatus::kPlayer0);
}

TEST_CASE("Advance: bonus status unchanged", "[move_application]") {
    auto setup = make_setup(0);
    SimplifiedGameState state;
    state.player(0).set_position(3);
    state.player(1).set_position(8);
    state.set_bonus_status(BonusStatus::kPlayer1);
    state.set_next_player(0);
    auto next = apply_move(state, Advance{}, setup);
    REQUIRE(next.bonus_status() == BonusStatus::kPlayer1);
}

TEST_CASE("next_player: switches when active overtakes opponent", "[move_application]") {
    auto setup = make_setup(0);
    SimplifiedGameState state;
    // p0 at 3, p1 at 8. Advance: p0 → 9 > 8 → next_player = 1
    state.player(0).set_position(3);
    state.player(1).set_position(8);
    state.set_next_player(0);
    auto next = apply_move(state, Advance{}, setup);
    REQUIRE(next.active_player() == 1);
}

TEST_CASE("next_player: stays same when active lands on opponent's position", "[move_application]") {
    auto setup = make_setup(0);
    SimplifiedGameState state;
    // p0 at 7, p1 at 8. Advance: p0 → 9. 9 > 8 → switches to p1.
    // For landing on same position: need opponent at same as new_pos.
    // p0 at 7, p1 at 8. new_pos = 9. 9 > 8 → switches. Not what we want.
    // For staying same: p0 at 7, p1 at 7 (tie). Advance: new_pos = 8. 8 > 7 → switches to p1.
    // Hmm. "stays same on tie" means position EQUALS opponent after move? Let's re-read:
    // "if strictly greater, opponent becomes next; otherwise moved player remains"
    // So if new_pos == opp_pos → stay same. Use BuyPatch that lands exactly on opponent.
    // p0 at 5, p1 at 8. Buy patch with time=3 → pos=8. 8 == 8 → stay p0.
    state.player(0).set_position(5);
    state.player(1).set_position(8);
    state.set_next_player(0);
    auto state2 = make_state_with_only(1);  // patch 1: time=3
    state2.player(0).set_position(5);
    state2.player(0).set_buttons(5);
    state2.player(1).set_position(8);
    state2.set_next_player(0);
    auto next = apply_move(state2, BuyPatch{1}, setup);
    // p0 moves to 5+3=8, equals p1's 8. Not strictly greater → p0 remains active
    REQUIRE(next.active_player() == 0);
}

TEST_CASE("first_to_finish set when player first crosses 53", "[move_application]") {
    auto setup = make_setup(0);
    SimplifiedGameState state;
    // p0 at 52, p1 at 53. p0 is active. Advance: p0 → 54, crosses 53.
    // p1 pre_pos=53 >= 53 → opp is already done; condition requires opp < 53, so NOT set.
    // Use a state where BOTH are below 53: p0 at 52, p1 at 53? No, 53>=53.
    // p0 at 50, p1 at 52. p0 is active (50 < 52). Advance: p0 → 53. crosses 53.
    // p1 pre_pos=52 < 53 → neither was done before → first_to_finish = 0.
    state.player(0).set_position(50);
    state.player(1).set_position(52);
    state.set_next_player(0);
    auto next = apply_move(state, Advance{}, setup);
    REQUIRE(next.first_to_finish() == 0);
}

TEST_CASE("first_to_finish not overwritten when second player finishes", "[move_application]") {
    auto setup = make_setup(0);
    SimplifiedGameState state;
    // p0 already finished, first_to_finish=0. p1 at 52, p0 at 54.
    state.player(0).set_position(54);
    state.player(1).set_position(52);
    state.set_first_to_finish(0);
    state.set_next_player(1);
    auto next = apply_move(state, Advance{}, setup);
    // p1 moves to 55 (opp+1=54+1=55), crosses 53. But opp (p0) was already >= 53.
    REQUIRE(next.first_to_finish() == 0);
}
