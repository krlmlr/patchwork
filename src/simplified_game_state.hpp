#ifndef PATCHWORK_SIMPLIFIED_GAME_STATE_HPP
#define PATCHWORK_SIMPLIFIED_GAME_STATE_HPP

#include "game_state.hpp"
#include "player_state.hpp"

#include <cassert>
#include <cstdint>
#include <type_traits>

namespace patchwork {

/// Per-player economy state packed into 32 bits.
///
/// The 81-bit quilt board is replaced by a single 7-bit counter of free
/// (uncovered) cells. Spatial placement information is intentionally discarded.
///
/// Layout (bits, LSB-first within the uint32_t):
///   bits  0– 6 : free_spaces (7 bits, 0–81)
///   bits  7–12 : position    (6 bits, 0–63)
///   bits 13–19 : buttons     (7 bits, 0–127)
///   bits 20–24 : income      (5 bits, 0–31)
///   bits 25–31 : [unused]
class SimplifiedPlayerState {
public:
    constexpr SimplifiedPlayerState() noexcept
        : data_{encode_defaults()}
    {}

    // ── Free spaces (7 bits, range 0–81) ──

    [[nodiscard]] constexpr int free_spaces() const noexcept {
        return static_cast<int>(data_ & kFreeSpacesMask);
    }

    constexpr void set_free_spaces(int v) noexcept {
        assert(v >= 0 && v <= 81);
        data_ = (data_ & ~kFreeSpacesMask)
              | (static_cast<uint32_t>(v) & kFreeSpacesMask);
    }

    // ── Position (6 bits, range 0–63) ──

    [[nodiscard]] constexpr int position() const noexcept {
        return static_cast<int>((data_ >> kPosShift) & kPosMask);
    }

    constexpr void set_position(int v) noexcept {
        assert(v >= 0 && v <= 63);
        data_ = (data_ & ~(kPosMask << kPosShift))
              | (static_cast<uint32_t>(v) << kPosShift);
    }

    // ── Buttons (7 bits, range 0–127) ──

    [[nodiscard]] constexpr int buttons() const noexcept {
        return static_cast<int>((data_ >> kBtnShift) & kBtnMask);
    }

    constexpr void set_buttons(int v) noexcept {
        assert(v >= 0 && v <= 127);
        data_ = (data_ & ~(kBtnMask << kBtnShift))
              | (static_cast<uint32_t>(v) << kBtnShift);
    }

    // ── Income (5 bits, range 0–31) ──

    [[nodiscard]] constexpr int income() const noexcept {
        return static_cast<int>((data_ >> kIncShift) & kIncMask);
    }

    constexpr void set_income(int v) noexcept {
        assert(v >= 0 && v <= 31);
        data_ = (data_ & ~(kIncMask << kIncShift))
              | (static_cast<uint32_t>(v) << kIncShift);
    }

private:
    static constexpr uint32_t kFreeSpacesMask = 0x7F;  // 7 bits at shift 0

    static constexpr int kPosShift = 7;
    static constexpr uint32_t kPosMask = 0x3F;  // 6 bits

    static constexpr int kBtnShift = 13;
    static constexpr uint32_t kBtnMask = 0x7F;  // 7 bits

    static constexpr int kIncShift = 20;
    static constexpr uint32_t kIncMask = 0x1F;  // 5 bits

    static constexpr uint32_t encode_defaults() noexcept {
        // free_spaces=81, position=0, buttons=5, income=0
        return (static_cast<uint32_t>(81) & kFreeSpacesMask)
             | (static_cast<uint32_t>(5) << kBtnShift);
    }

    uint32_t data_;
};

static_assert(sizeof(SimplifiedPlayerState) <= 4,
              "SimplifiedPlayerState must fit in 32 bits");
static_assert(!std::is_same_v<SimplifiedPlayerState, PlayerState>,
              "SimplifiedPlayerState and PlayerState must be distinct types");

/// Simplified game state: two SimplifiedPlayerStates + shared state.
///
/// Shared-state layout mirrors GameState (same bit positions):
///   bits  0–32 : patch availability (33 bits, one per patch)
///   bits 33–38 : circle marker position (6 bits, 0–32)
///   bits 39–40 : 7×7 bonus status (2 bits)
///   bit  41    : next_player (0 = player 0, 1 = player 1)
///   bit  42    : first_to_finish (player who first reached position ≥ 53)
class SimplifiedGameState {
public:
    constexpr SimplifiedGameState() noexcept
        : players_{}
        , shared_{kAllPatchesMask}  // all 33 patches available
    {}

    // ── Player accessors ──

    [[nodiscard]] constexpr SimplifiedPlayerState& player(int idx) noexcept {
        assert(idx == 0 || idx == 1);
        return players_[idx];
    }

    [[nodiscard]] constexpr const SimplifiedPlayerState& player(int idx) const noexcept {
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
        shared_ = v ? (shared_ | (uint64_t{1} << idx))
                    : (shared_ & ~(uint64_t{1} << idx));
    }

    // ── Circle marker (6 bits, range 0–32) ──

    [[nodiscard]] constexpr int circle_marker() const noexcept {
        return static_cast<int>((shared_ >> kCircleShift) & kCircleMask);
    }

    constexpr void set_circle_marker(int v) noexcept {
        assert(v >= 0 && v <= 32);
        shared_ = (shared_ & ~(kCircleMask << kCircleShift))
                | (static_cast<uint64_t>(v) << kCircleShift);
    }

    // ── 7×7 bonus status (2 bits) ──

    [[nodiscard]] constexpr BonusStatus bonus_status() const noexcept {
        return static_cast<BonusStatus>((shared_ >> kBonusShift) & kBonusMask);
    }

    constexpr void set_bonus_status(BonusStatus s) noexcept {
        shared_ = (shared_ & ~(kBonusMask << kBonusShift))
                | (static_cast<uint64_t>(s) << kBonusShift);
    }

    // ── Active player / next_player (bit 41) ──

    [[nodiscard]] constexpr int active_player() const noexcept {
        return static_cast<int>((shared_ >> kNextPlayerShift) & 1U);
    }

    constexpr void set_next_player(int v) noexcept {
        assert(v == 0 || v == 1);
        shared_ = (shared_ & ~(uint64_t{1} << kNextPlayerShift))
                | (static_cast<uint64_t>(v) << kNextPlayerShift);
    }

    // ── First to finish (bit 42) ──

    [[nodiscard]] constexpr int first_to_finish() const noexcept {
        return static_cast<int>((shared_ >> kFirstToFinishShift) & 1U);
    }

    constexpr void set_first_to_finish(int v) noexcept {
        assert(v == 0 || v == 1);
        shared_ = (shared_ & ~(uint64_t{1} << kFirstToFinishShift))
                | (static_cast<uint64_t>(v) << kFirstToFinishShift);
    }

private:
    static constexpr uint64_t kAllPatchesMask = (uint64_t{1} << 33) - 1;

    static constexpr int kCircleShift = 33;
    static constexpr uint64_t kCircleMask = 0x3F;  // 6 bits

    static constexpr int kBonusShift = 39;
    static constexpr uint64_t kBonusMask = 0x03;  // 2 bits

    static constexpr int kNextPlayerShift    = 41;
    static constexpr int kFirstToFinishShift = 42;

    SimplifiedPlayerState players_[2];
    uint64_t shared_;
};

}  // namespace patchwork

#endif  // PATCHWORK_SIMPLIFIED_GAME_STATE_HPP
