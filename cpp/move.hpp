#ifndef PATCHWORK_MOVE_HPP
#define PATCHWORK_MOVE_HPP

#include <variant>

namespace patchwork {

struct BuyPatch {
    int patch_index;
};
struct Advance {};

using Move = std::variant<BuyPatch, Advance>;

inline bool operator==(const BuyPatch& a, const BuyPatch& b) {
    return a.patch_index == b.patch_index;
}
inline bool operator==(const Advance&, const Advance&) { return true; }

}  // namespace patchwork

#endif  // PATCHWORK_MOVE_HPP
