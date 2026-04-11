#include "move_application.hpp"

#include <algorithm>
#include <array>
#include <cassert>

#include "generated/patches.hpp"

namespace patchwork {

namespace {

static constexpr std::array<int, 9> kIncomeSpaces = {5, 11, 17, 23, 29, 35, 41, 47, 53};
static constexpr std::array<int, 5> kLeatherThresholds = {26, 32, 38, 44, 50};

void apply_income_spaces(SimplifiedGameState& state, int player, int old_pos, int new_pos) {
    for (int t : kIncomeSpaces) {
        if (old_pos < t && new_pos >= t) {
            int b = state.player(player).buttons() + state.player(player).income();
            state.player(player).set_buttons(std::min(b, 127));
        }
    }
}

void apply_leather_patches(SimplifiedGameState& state, int player, int old_pos, int new_pos,
                           int pre_pos_0, int pre_pos_1) {
    for (int t : kLeatherThresholds) {
        if (old_pos < t && new_pos >= t && pre_pos_0 < t && pre_pos_1 < t) {
            state.player(player).set_free_spaces(state.player(player).free_spaces() - 1);
        }
    }
}

int next_active_player(int active, int active_pos, int opp, int opp_pos) noexcept {
    // The moved player stays active unless they strictly overtook the opponent.
    return active_pos > opp_pos ? opp : active;
}

}  // namespace

SimplifiedGameState apply_move(SimplifiedGameState state, Move move, const GameSetup& setup) {
    int active = state.active_player();
    int opp = 1 - active;
    int pre_pos_0 = state.player(0).position();
    int pre_pos_1 = state.player(1).position();
    int old_pos = state.player(active).position();
    int opp_pos = state.player(opp).position();

    if (std::holds_alternative<BuyPatch>(move)) {
        int patch_id = std::get<BuyPatch>(move).patch_index;
        const auto& p = kPatches[static_cast<std::size_t>(patch_id)];

        // Deduct button cost
        state.player(active).set_buttons(state.player(active).buttons() - p.buttons);
        // Add patch income (before income-space payout so payout uses updated income)
        state.player(active).set_income(std::min(state.player(active).income() + p.income, 31));
        // Reduce free spaces by patch cell count
        state.player(active).set_free_spaces(state.player(active).free_spaces() - p.num_cells);
        // Advance position (cap at 63 — the field is 6 bits)
        int new_pos = std::min(old_pos + p.time, 63);
        state.player(active).set_position(new_pos);

        // Mark patch unavailable
        state.set_patch_available(patch_id, false);

        // Advance circle marker to the position just after this patch in the circle
        int marker = state.circle_marker();
        for (int i = 0; i < 33; ++i) {
            int pos = (marker + i) % 33;
            if (static_cast<int>(setup.circle()[static_cast<std::size_t>(pos)]) == patch_id) {
                state.set_circle_marker((pos + 1) % 33);
                break;
            }
        }

        // Button income-space payouts (with updated income)
        apply_income_spaces(state, active, old_pos, new_pos);
        // Leather patch awards
        apply_leather_patches(state, active, old_pos, new_pos, pre_pos_0, pre_pos_1);

        // 7×7 bonus check (only after BuyPatch)
        if (state.bonus_status() == BonusStatus::kUnclaimed) {
            int occupied = 81 - state.player(active).free_spaces();
            if (occupied >= 56) {
                state.set_bonus_status(active == 0 ? BonusStatus::kPlayer0 : BonusStatus::kPlayer1);
            }
        }

        // first_to_finish: set once when a player first crosses position 53
        int new_active_pos = state.player(active).position();
        if (old_pos < 53 && new_active_pos >= 53 && opp_pos < 53) {
            state.set_first_to_finish(active);
        }

        // Update next_player
        state.set_next_player(
            next_active_player(active, new_active_pos, opp, state.player(opp).position()));

    } else {
        // Advance: move to opponent_pos + 1, earn 1 button per space advanced
        int new_pos = std::min(opp_pos + 1, 63);
        int spaces = new_pos - old_pos;
        state.player(active).set_buttons(std::min(state.player(active).buttons() + spaces, 127));
        state.player(active).set_position(new_pos);

        // Button income-space payouts
        apply_income_spaces(state, active, old_pos, new_pos);
        // Leather patch awards
        apply_leather_patches(state, active, old_pos, new_pos, pre_pos_0, pre_pos_1);

        // first_to_finish
        if (old_pos < 53 && new_pos >= 53 && opp_pos < 53) {
            state.set_first_to_finish(active);
        }

        // Update next_player
        state.set_next_player(
            next_active_player(active, new_pos, opp, state.player(opp).position()));
    }

    return state;
}

}  // namespace patchwork
