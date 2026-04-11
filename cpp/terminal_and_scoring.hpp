#ifndef PATCHWORK_TERMINAL_AND_SCORING_HPP
#define PATCHWORK_TERMINAL_AND_SCORING_HPP

#include "game_state.hpp"
#include "simplified_game_state.hpp"

namespace patchwork {

/// Returns true when both players have reached or passed position 53.
[[nodiscard]] inline bool is_terminal(const SimplifiedGameState& s) noexcept {
    return s.player(0).position() >= 53 && s.player(1).position() >= 53;
}

/// Score for `player` (0 or 1): buttons − 2 × free_spaces + 7 if bonus held.
/// Only meaningful on a terminal state.
[[nodiscard]] inline int score(const SimplifiedGameState& s, int player) noexcept {
    int bonus = 0;
    if (player == 0 && s.bonus_status() == BonusStatus::kPlayer0) bonus = 7;
    if (player == 1 && s.bonus_status() == BonusStatus::kPlayer1) bonus = 7;
    return s.player(player).buttons() - 2 * s.player(player).free_spaces() + bonus;
}

/// Returns the index (0 or 1) of the winning player.
/// On equal scores, `first_to_finish` is the tiebreaker — draws are impossible.
/// Only meaningful on a terminal state.
[[nodiscard]] inline int winner(const SimplifiedGameState& s) noexcept {
    int s0 = score(s, 0);
    int s1 = score(s, 1);
    if (s0 > s1) return 0;
    if (s1 > s0) return 1;
    return s.first_to_finish();
}

}  // namespace patchwork

#endif  // PATCHWORK_TERMINAL_AND_SCORING_HPP
