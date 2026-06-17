#include "game_logger.hpp"

#include "terminal_and_scoring.hpp"

namespace patchwork {

void log_game_start(std::ostream& out, long long seed, int setup_id,
                    const SimplifiedGameState& state) {
    out << R"({"event":"game_start","seed":)" << seed << R"(,"setup_id":)" << setup_id
        << R"(,"p0":{"buttons":)" << state.player(0).buttons() << R"(,"income":)"
        << state.player(0).income() << R"(,"free_spaces":)" << state.player(0).free_spaces()
        << R"(},"p1":{"buttons":)" << state.player(1).buttons() << R"(,"income":)"
        << state.player(1).income() << R"(,"free_spaces":)" << state.player(1).free_spaces()
        << R"(}})" << '\n';
}

void log_move(std::ostream& out, int ply, int player, const Move& move,
              const SimplifiedGameState& new_state) {
    out << R"({"event":"move","ply":)" << ply << R"(,"player":)" << player;
    if (std::holds_alternative<BuyPatch>(move)) {
        int idx = std::get<BuyPatch>(move).patch_index;
        out << R"(,"move_type":"buy_patch","patch_index":)" << idx;
    } else {
        out << R"(,"move_type":"advance")";
    }
    out << R"(,"position":)" << new_state.player(player).position() << R"(,"buttons":)"
        << new_state.player(player).buttons() << "}\n";
}

void log_game_end(std::ostream& out, const SimplifiedGameState& state) {
    out << R"({"event":"game_end","score_p0":)" << score(state, 0) << R"(,"score_p1":)"
        << score(state, 1) << R"(,"winner":)" << winner(state) << "}\n";
}

}  // namespace patchwork
