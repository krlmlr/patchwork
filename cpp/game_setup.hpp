#ifndef PATCHWORK_GAME_SETUP_HPP
#define PATCHWORK_GAME_SETUP_HPP

#include "generated/patches.hpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <ostream>
#include <string_view>

namespace patchwork {

/// Initial circular arrangement of all 33 patches.
///
/// Stores patch IDs (0–32, indices into kPatches) in circle order.
/// Constructed from a 33-character string_view of single-char patch names.
class GameSetup {
public:
    /// Construct from a 33-char string of single-char patch names.
    /// Each character must be a valid patch name from kPatches.
    explicit constexpr GameSetup(std::string_view sv) noexcept : circle_{} {
        assert(sv.size() == 33);
        for (int i = 0; i < 33; ++i) {
            circle_[i] = name_to_id(sv[i]);
            assert(circle_[i] < 33);  // catches invalid patch name characters
        }
    }

    /// The circle: 33 patch IDs (0–32) in circle order.
    [[nodiscard]] constexpr const std::array<uint8_t, 33>& circle() const noexcept {
        return circle_;
    }

    /// Serialise to a single NDJSON line: {"type":"setup","circle":"<33chars>"}
    void to_ndjson(std::ostream& out) const {
        out << R"({"type":"setup","circle":")";
        for (uint8_t id : circle_) {
            out << kPatches[id].name;
        }
        out << "\"}\n";
    }

private:
    static constexpr uint8_t name_to_id(char name) noexcept {
        for (int i = 0; i < 33; ++i) {
            if (kPatches[i].name == name) {
                return static_cast<uint8_t>(i);
            }
        }
        return 0xFF;  // unreachable for valid input
    }

    std::array<uint8_t, 33> circle_;
};

}  // namespace patchwork

#endif  // PATCHWORK_GAME_SETUP_HPP
