#ifndef PATCHWORK_PLAYER_STATE_HPP
#define PATCHWORK_PLAYER_STATE_HPP

#include <cassert>
#include <cstdint>

namespace patchwork {

/// Per-player game state packed into 128 bits (16 bytes).
///
/// Layout (bits, LSB-first within each uint64_t):
///   word0 (64 bits): board bits  0–63
///   word1 (64 bits): board bits 64–80 (17 bits)
///                    position   bits 81–86  (6 bits, 0–53)
///                    buttons    bits 87–93  (7 bits, 0–127)
///                    income     bits 94–98  (5 bits, 0–31)
///                    [unused]   bits 99–127
///
/// Board is a 9×9 = 81-bit grid; cell (r,c) maps to bit index r*9+c.
class PlayerState {
public:
    constexpr PlayerState() noexcept
        : word0_{0},
          word1_{encode_buttons(5)}  // starting buttons = 5
    {}

    // ── Board accessors ──

    [[nodiscard]] constexpr bool cell(int row, int col) const noexcept {
        assert(row >= 0 && row < 9 && col >= 0 && col < 9);
        int idx = row * 9 + col;
        if (idx < 64) return (word0_ >> idx) & 1U;
        return (word1_ >> (idx - 64)) & 1U;
    }

    constexpr void set_cell(int row, int col, bool v) noexcept {
        assert(row >= 0 && row < 9 && col >= 0 && col < 9);
        int idx = row * 9 + col;
        if (idx < 64) {
            word0_ = v ? (word0_ | (uint64_t{1} << idx)) : (word0_ & ~(uint64_t{1} << idx));
        } else {
            int shift = idx - 64;
            word1_ = v ? (word1_ | (uint64_t{1} << shift)) : (word1_ & ~(uint64_t{1} << shift));
        }
    }

    // ── Position (6 bits, range 0–53) ──

    [[nodiscard]] constexpr int position() const noexcept {
        return static_cast<int>((word1_ >> kPosShift) & kPosMask);
    }

    constexpr void set_position(int v) noexcept {
        assert(v >= 0 && v <= 53);
        word1_ = (word1_ & ~(kPosMask << kPosShift)) | (static_cast<uint64_t>(v) << kPosShift);
    }

    // ── Buttons (7 bits, range 0–127) ──

    [[nodiscard]] constexpr int buttons() const noexcept {
        return static_cast<int>((word1_ >> kBtnShift) & kBtnMask);
    }

    constexpr void set_buttons(int v) noexcept {
        assert(v >= 0 && v <= 127);
        word1_ = (word1_ & ~(kBtnMask << kBtnShift)) | (static_cast<uint64_t>(v) << kBtnShift);
    }

    // ── Income (5 bits, range 0–31) ──

    [[nodiscard]] constexpr int income() const noexcept {
        return static_cast<int>((word1_ >> kIncShift) & kIncMask);
    }

    constexpr void set_income(int v) noexcept {
        assert(v >= 0 && v <= 31);
        word1_ = (word1_ & ~(kIncMask << kIncShift)) | (static_cast<uint64_t>(v) << kIncShift);
    }

private:
    static constexpr uint64_t encode_buttons(int v) noexcept {
        return static_cast<uint64_t>(v) << kBtnShift;
    }

    // Board occupies bits 0..80 across word0_ (0..63) and word1_ (0..16).
    // Scalar fields start at bit 17 of word1_ (= global bit 81).
    static constexpr int kPosShift = 17;        // 81 - 64
    static constexpr uint64_t kPosMask = 0x3F;  // 6 bits

    static constexpr int kBtnShift = 23;        // 17 + 6
    static constexpr uint64_t kBtnMask = 0x7F;  // 7 bits

    static constexpr int kIncShift = 30;        // 23 + 7
    static constexpr uint64_t kIncMask = 0x1F;  // 5 bits

    uint64_t word0_;
    uint64_t word1_;
};

static_assert(sizeof(PlayerState) <= 16, "PlayerState must fit in 128 bits");

}  // namespace patchwork

#endif  // PATCHWORK_PLAYER_STATE_HPP
