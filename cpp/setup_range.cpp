#include "setup_range.hpp"

#include <cctype>
#include <charconv>
#include <stdexcept>
#include <string>

namespace patchwork {

namespace {

// Trim ASCII whitespace from both ends of a view.
std::string_view trim(std::string_view s) {
    std::size_t b = 0;
    std::size_t e = s.size();
    while (b < e && std::isspace(static_cast<unsigned char>(s[b]))) ++b;
    while (e > b && std::isspace(static_cast<unsigned char>(s[e - 1]))) --e;
    return s.substr(b, e - b);
}

// Parse a trimmed, non-empty view as a non-negative integer.
int parse_nonneg(std::string_view s) {
    if (s.empty()) throw std::invalid_argument("empty number");
    int value = 0;
    const char* first = s.data();
    const char* last = s.data() + s.size();
    auto [ptr, ec] = std::from_chars(first, last, value);
    if (ec != std::errc{} || ptr != last || value < 0) {
        throw std::invalid_argument("invalid setup id: '" + std::string(s) + "'");
    }
    return value;
}

}  // namespace

std::vector<int> parse_setups(std::string_view spec) {
    std::vector<int> out;
    std::string_view rest = spec;
    bool any_token = false;

    while (true) {
        std::size_t comma = rest.find(',');
        std::string_view token = trim(rest.substr(0, comma));
        any_token = true;

        if (token.empty()) {
            throw std::invalid_argument("empty setup token");
        }

        std::size_t dash = token.find('-');
        if (dash == std::string_view::npos) {
            out.push_back(parse_nonneg(token));
        } else {
            int lo = parse_nonneg(trim(token.substr(0, dash)));
            int hi = parse_nonneg(trim(token.substr(dash + 1)));
            if (hi < lo) {
                throw std::invalid_argument("descending setup range: '" + std::string(token) + "'");
            }
            for (int v = lo; v <= hi; ++v) out.push_back(v);
        }

        if (comma == std::string_view::npos) break;
        rest = rest.substr(comma + 1);
    }

    if (!any_token) {
        throw std::invalid_argument("empty setup specification");
    }
    return out;
}

}  // namespace patchwork
