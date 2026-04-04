#ifndef PATCHWORK_GAME_SETUPS_HPP
#define PATCHWORK_GAME_SETUPS_HPP

#include "game_setup.hpp"
#include "generated/patches.hpp"

#include <string>

namespace patchwork {

/// Build a GameSetup by rotating the canonical patch circle by `id` positions.
/// Setup ID 0 = canonical order (kPatches[0]..kPatches[32]).
/// Setup ID n = rotate the starting position by n (mod 33).
[[nodiscard]] inline GameSetup make_setup(int id) {
    int offset = id % 33;
    if (offset < 0) offset += 33;
    std::string circle(33, ' ');
    for (int i = 0; i < 33; ++i) {
        circle[static_cast<std::size_t>(i)] =
            kPatches[static_cast<std::size_t>((i + offset) % 33)].name;
    }
    return GameSetup(std::string_view(circle));
}

}  // namespace patchwork

#endif  // PATCHWORK_GAME_SETUPS_HPP
