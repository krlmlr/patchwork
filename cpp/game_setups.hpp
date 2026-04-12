#ifndef PATCHWORK_GAME_SETUPS_HPP
#define PATCHWORK_GAME_SETUPS_HPP

#include <string>

#include "game_setup.hpp"
#include "generated/patches.hpp"

namespace patchwork {

/// Build a GameSetup by rotating the 32 non-'2' patches and appending '2' last.
///
/// By game convention the neutral token is placed after the '2' (two-square)
/// tile, so '2' (kPatches[0]) always occupies the last circle position (32).
/// The other 32 patches (kPatches[1..32]) are rotated left by `id` positions.
///
///   Setup ID 0: kPatches[1], kPatches[2], ..., kPatches[32], kPatches[0]='2'
///   Setup ID n: kPatches[n%32+1], ..., kPatches[32], kPatches[1], ..., kPatches[0]='2'
[[nodiscard]] inline GameSetup make_setup(int id) {
    int offset = id % 32;
    if (offset < 0) offset += 32;
    std::string circle(33, ' ');
    for (int i = 0; i < 32; ++i) {
        circle[static_cast<std::size_t>(i)] =
            kPatches[static_cast<std::size_t>((i + offset) % 32) + 1].name;
    }
    circle[32] = kPatches[0].name;  // '2' is always last
    return GameSetup(std::string_view(circle));
}

}  // namespace patchwork

#endif  // PATCHWORK_GAME_SETUPS_HPP
