#include "game_logger.hpp"

#include <array>

#include "generated/patches.hpp"
#include "terminal_and_scoring.hpp"

namespace patchwork {

namespace {

// Button-income spaces on the time track (from move_application.cpp).
static constexpr std::array<int, 9> kIncomeSpaces = {5, 11, 17, 23, 29, 35, 41, 47, 53};

// Number of income-space triggers remaining for a player at `position`.
static int remaining_income_triggers(int position) {
    int count = 0;
    for (int t : kIncomeSpaces) {
        if (t > position) ++count;
    }
    return count;
}

}  // namespace

void log_game_start(std::ostream& out, long long seed, int setup_id,
                    const SimplifiedGameState& state, const GameSetup& setup) {
    out << R"({"event":"game_start","seed":)" << seed << R"(,"setup_id":)" << setup_id
        << R"(,"p0":{"buttons":)" << state.player(0).buttons() << R"(,"income":)"
        << state.player(0).income() << R"(,"free_spaces":)" << state.player(0).free_spaces()
        << R"(},"p1":{"buttons":)" << state.player(1).buttons() << R"(,"income":)"
        << state.player(1).income() << R"(,"free_spaces":)" << state.player(1).free_spaces()
        << R"(},"circle":")";
    for (uint8_t id : setup.circle()) {
        out << kPatches[id].name;
    }
    out << "\"}\n";
}

void log_move(std::ostream& out, int ply, int player, const Move& move,
              const SimplifiedGameState& new_state, const GameSetup& setup) {
    out << R"({"event":"move","ply":)" << ply << R"(,"player":)" << player;
    if (std::holds_alternative<BuyPatch>(move)) {
        int idx = std::get<BuyPatch>(move).patch_index;
        out << R"(,"move_type":"buy_patch","patch_index":)" << idx
            << R"(,"patch_symbol":")" << kPatches[static_cast<std::size_t>(idx)].name << '"';
    } else {
        out << R"(,"move_type":"advance")";
    }
    out << R"(,"position":)" << new_state.player(player).position() << R"(,"buttons":)"
        << new_state.player(player).buttons() << R"(,"income":)" << new_state.player(player).income()
        << R"(,"free_spaces":)" << new_state.player(player).free_spaces()
        << R"(,"board_value":)" << (new_state.player(player).buttons() - 2 * new_state.player(player).free_spaces());
    {
        int projected_income = new_state.player(player).income() *
                               remaining_income_triggers(new_state.player(player).position());
        int projected_score = new_state.player(player).buttons() + projected_income -
                              2 * new_state.player(player).free_spaces();
        out << R"(,"projected_income":)" << projected_income
            << R"(,"projected_score":)" << projected_score;
    }
    out << R"(,"circle":")";
    // Emit available patches in circle order starting from circle_marker, wrapping mod 33.
    int marker = new_state.circle_marker();
    for (int i = 0; i < 33; ++i) {
        int pos = (marker + i) % 33;
        int id = setup.circle()[pos];
        if (new_state.patch_available(id)) {
            out << kPatches[id].name;
        }
    }
    out << "\"}\n";
}

void log_game_end(std::ostream& out, const SimplifiedGameState& state) {
    out << R"({"event":"game_end","score_p0":)" << score(state, 0) << R"(,"score_p1":)"
        << score(state, 1) << R"(,"winner":)" << winner(state)
        << R"(,"p0":{"income":)" << state.player(0).income()
        << R"(,"free_spaces":)" << state.player(0).free_spaces()
        << R"(},"p1":{"income":)" << state.player(1).income()
        << R"(,"free_spaces":)" << state.player(1).free_spaces()
        << "}}\n";
}

}  // namespace patchwork
