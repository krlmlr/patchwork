#ifndef PATCHWORK_SEED_UTIL_HPP
#define PATCHWORK_SEED_UTIL_HPP

#include <cstdint>

namespace patchwork {

/// SplitMix64 finalizing mixer — a fast, well-distributed 64-bit hash step.
[[nodiscard]] inline std::uint64_t splitmix64(std::uint64_t x) noexcept {
    x += 0x9E3779B97F4A7C15ULL;
    x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ULL;
    x = (x ^ (x >> 27)) * 0x94D049BB133111EBULL;
    return x ^ (x >> 31);
}

/// Derive a deterministic per-game 32-bit seed from a master seed, the setup
/// id, and the game index. This is a pure function of its three inputs: it does
/// NOT advance a shared stream, so the game at `(setup_id, game_index)` always
/// gets the same seed regardless of the batch's ordering or size. The result
/// fits in 32 bits so it round-trips through the play driver's `--seed`
/// argument (which seeds `std::mt19937` via `static_cast<unsigned>`), letting
/// any single game be replayed independently.
[[nodiscard]] inline std::uint32_t derive_seed(std::uint64_t master_seed, int setup_id,
                                               int game_index) noexcept {
    std::uint64_t h = splitmix64(master_seed);
    h = splitmix64(h ^ (static_cast<std::uint64_t>(static_cast<std::uint32_t>(setup_id)) *
                        0x9E3779B97F4A7C15ULL));
    h = splitmix64(h ^ (static_cast<std::uint64_t>(static_cast<std::uint32_t>(game_index)) *
                        0xC2B2AE3D27D4EB4FULL));
    return static_cast<std::uint32_t>(h >> 32);
}

}  // namespace patchwork

#endif  // PATCHWORK_SEED_UTIL_HPP
