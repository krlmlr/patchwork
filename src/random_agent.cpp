#include "random_agent.hpp"
#include "move_generation.hpp"

#include <cassert>

namespace patchwork {

Move random_move(const SimplifiedGameState& state, const GameSetup& setup,
                 std::mt19937& rng) {
    auto moves = legal_moves(state, setup);
    assert(!moves.empty());
    std::uniform_int_distribution<int> dist(0, static_cast<int>(moves.size()) - 1);
    return moves[static_cast<std::size_t>(dist(rng))];
}

}  // namespace patchwork
