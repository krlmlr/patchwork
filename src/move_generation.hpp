#ifndef PATCHWORK_MOVE_GENERATION_HPP
#define PATCHWORK_MOVE_GENERATION_HPP

#include "move.hpp"
#include "simplified_game_state.hpp"
#include "game_setup.hpp"

#include <vector>

namespace patchwork {

/// Return all legal moves for the active player in `state`.
///
/// Returns an empty vector if the game is terminal (both players at position ≥ 53).
/// Otherwise returns up to three BuyPatch moves (affordable patches ahead of the
/// circle marker in circular order) plus an Advance move.
[[nodiscard]] std::vector<Move> legal_moves(const SimplifiedGameState& state,
                                            const GameSetup& setup);

}  // namespace patchwork

#endif  // PATCHWORK_MOVE_GENERATION_HPP
