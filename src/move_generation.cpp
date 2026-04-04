#include "move_generation.hpp"
#include "generated/patches.hpp"

namespace patchwork {

std::vector<Move> legal_moves(const SimplifiedGameState& state, const GameSetup& setup) {
    if (state.player(0).position() >= 53 && state.player(1).position() >= 53) {
        return {};
    }

    std::vector<Move> moves;
    int active  = state.active_player();
    int buttons = state.player(active).buttons();
    int marker  = state.circle_marker();

    int found = 0;
    for (int i = 0; i < 33 && found < 3; ++i) {
        int pos      = (marker + i) % 33;
        int patch_id = static_cast<int>(setup.circle()[static_cast<std::size_t>(pos)]);
        if (state.patch_available(patch_id)) {
            ++found;
            if (buttons >= kPatches[static_cast<std::size_t>(patch_id)].buttons) {
                moves.push_back(BuyPatch{patch_id});
            }
        }
    }

    moves.push_back(Advance{});
    return moves;
}

}  // namespace patchwork
