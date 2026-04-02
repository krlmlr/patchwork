#ifndef PATCHWORK_GAME_STATE_HPP
#define PATCHWORK_GAME_STATE_HPP

#include <cassert>
#include <cstdint>

#include "player_state.hpp"

namespace patchwork {

/// Bonus tile status (2 bits).
enum class BonusStatus : uint8_t {
    kUnclaimed = 0,
    kPlayer0 = 1,
    kPlayer1 = 2,
};

/// Full game state: two PlayerStates (2×128 bits) + shared state (64 bits).
///
/// Shared-state layout in shared_ (uint64_t):
///   bits  0–32 : patch availability (33 bits, one per patch)
///   bits 33–38 : circle marker position (6 bits, 0–32)
///   bits 39–40 : 7×7 bonus status (2 bits)
class GameState {
public:
    constexpr GameState() noexcept
        : players_{},
          shared_{kAllPatchesMask}  // all 33 patches available
    {}

    // ── Player accessors ──

    [[nodiscard]] constexpr PlayerState& player(int idx) noexcept {
        assert(idx == 0 || idx == 1);
        return players_[idx];
    }

    [[nodiscard]] constexpr const PlayerState& player(int idx) const noexcept {
        assert(idx == 0 || idx == 1);
        return players_[idx];
    }

    // ── Patch availability (33 bits) ──

    [[nodiscard]] constexpr bool patch_available(int idx) const noexcept {
        assert(idx >= 0 && idx < 33);
        return (shared_ >> idx) & 1U;
    }

    constexpr void set_patch_available(int idx, bool v) noexcept {
        assert(idx >= 0 && idx < 33);
        shared_ = v ? (shared_ | (uint64_t{1} << idx)) : (shared_ & ~(uint64_t{1} << idx));
    }

    // ── Circle marker (6 bits, range 0–32) ──

    [[nodiscard]] constexpr int circle_marker() const noexcept {
        return static_cast<int>((shared_ >> kCircleShift) & kCircleMask);
    }

    constexpr void set_circle_marker(int v) noexcept {
        assert(v >= 0 && v <= 32);
        shared_ =
            (shared_ & ~(kCircleMask << kCircleShift)) | (static_cast<uint64_t>(v) << kCircleShift);
    }

    // ── 7×7 bonus status (2 bits) ──

    [[nodiscard]] constexpr BonusStatus bonus_status() const noexcept {
        return static_cast<BonusStatus>((shared_ >> kBonusShift) & kBonusMask);
    }

    constexpr void set_bonus_status(BonusStatus s) noexcept {
        shared_ =
            (shared_ & ~(kBonusMask << kBonusShift)) | (static_cast<uint64_t>(s) << kBonusShift);
    }

private:
    static constexpr uint64_t kAllPatchesMask = (uint64_t{1} << 33) - 1;  // bits 0..32 set

    static constexpr int kCircleShift = 33;
    static constexpr uint64_t kCircleMask = 0x3F;  // 6 bits

    static constexpr int kBonusShift = 39;
    static constexpr uint64_t kBonusMask = 0x03;  // 2 bits

    PlayerState players_[2];
    uint64_t shared_;
};

}  // namespace patchwork

#endif  // PATCHWORK_GAME_STATE_HPP
