#ifndef PATCHWORK_SETUP_RANGE_HPP
#define PATCHWORK_SETUP_RANGE_HPP

#include <string_view>
#include <vector>

namespace patchwork {

/// Parse a setup specification of comma-separated single ids and inclusive
/// ranges into an expanded list, e.g. "0,2,5-9" -> {0, 2, 5, 6, 7, 8, 9}.
///
/// Grammar: tokens separated by ','. Each token is either a single
/// non-negative integer `N` or an ascending inclusive range `A-B` (A <= B).
/// Surrounding whitespace around tokens and bounds is ignored. Order is
/// preserved and duplicates are kept as written.
///
/// Throws std::invalid_argument on malformed input: an empty spec, an empty
/// token, a non-numeric or negative value, or a descending range (A > B).
[[nodiscard]] std::vector<int> parse_setups(std::string_view spec);

}  // namespace patchwork

#endif  // PATCHWORK_SETUP_RANGE_HPP
