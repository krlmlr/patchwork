#ifndef PATCHWORK_GAME_SETUPS_HPP
#define PATCHWORK_GAME_SETUPS_HPP

#include <cstddef>

#include "game_setup.hpp"
#include "generated/game_setups.hpp"

namespace patchwork {

/// Build a GameSetup from the canonical `kGameSetups` table — the single source
/// of truth for setup circles, shared by the play driver and the TUI.
/// `id` selects `kGameSetups[id mod kNumGameSetups]`, wrapping (and handling
/// negative ids via floored modulo) so any integer id is valid.
[[nodiscard]] inline GameSetup make_setup(int id) {
    int index = id % static_cast<int>(kNumGameSetups);
    if (index < 0) index += static_cast<int>(kNumGameSetups);
    return GameSetup(kGameSetups[static_cast<std::size_t>(index)]);
}

}  // namespace patchwork

#endif  // PATCHWORK_GAME_SETUPS_HPP
