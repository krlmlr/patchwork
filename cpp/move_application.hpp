#ifndef PATCHWORK_MOVE_APPLICATION_HPP
#define PATCHWORK_MOVE_APPLICATION_HPP

#include "game_setup.hpp"
#include "move.hpp"
#include "simplified_game_state.hpp"

namespace patchwork {

/// Apply `move` to `state` and return the successor state.
///
/// Precondition: `move` must be a legal move in `state` (asserted in debug builds).
/// Takes `state` by value — the original is unchanged.
[[nodiscard]] SimplifiedGameState apply_move(SimplifiedGameState state, Move move,
                                             const GameSetup& setup);

}  // namespace patchwork

#endif  // PATCHWORK_MOVE_APPLICATION_HPP
