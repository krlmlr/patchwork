#pragma once
#include "../simplified_game_state.hpp"
#include "../game_setup.hpp"

#include <string>
#include <vector>

namespace patchwork::tui {

// ── ANSI color constants ──────────────────────────────────────────────────

inline constexpr const char* kReset        = "\033[0m";
inline constexpr const char* kBold         = "\033[1m";
inline constexpr const char* kDim          = "\033[2m";
inline constexpr const char* kColorP1      = "\033[96m";   // bright cyan
inline constexpr const char* kColorP2      = "\033[93m";   // bright yellow
inline constexpr const char* kColorGreen   = "\033[32m";   // event-log prompt
inline constexpr const char* kColorAff     = "\033[92m";   // bright green – affordable
inline constexpr const char* kColorCyan    = "\033[36m";   // cyan – advance
inline constexpr const char* kColorErrFlash = "\033[1;31m";// bold red – illegal move

// ── Display state ─────────────────────────────────────────────────────────

/// Runtime display configuration queried from the terminal.
struct DisplayConfig {
    int width{80};
    int height{24};
    bool color_enabled{true};
};

/// State for the in-frame event log pane.
struct LogState {
    std::vector<std::string> entries;
    int scroll_offset{0};  // horizontal scroll offset in chars
    bool wrap_mode{false};
};

/// State for the NDJSON bottom pane.
struct NdjsonState {
    std::vector<std::string> lines;
    int height{5};          // visible content lines (0 = minimised)
    int prev_height{5};     // height before last minimise (for toggle restore)
};

/// Initialise display: query terminal size, check ≥80×24, detect color.
/// Returns config. Exits with non-zero status if terminal is too small.
DisplayConfig init_display(bool no_color_flag, int argc, char** argv);

/// Append an entry to the event log; resets horizontal scroll offset.
void append_log(LogState& log, std::string entry);

/// Append a raw NDJSON line to the NDJSON pane (no truncation).
void append_ndjson(NdjsonState& ndjson, std::string line);

/// Apply NDJSON-pane resize command. Returns updated NdjsonState.
void ndjson_toggle_minimize(NdjsonState& s, int max_lines);
void ndjson_maximize(NdjsonState& s, int max_lines);
void ndjson_semi_maximize(NdjsonState& s, int max_lines);
void ndjson_decr_lines(NdjsonState& s);
void ndjson_incr_lines(NdjsonState& s, int max_lines);

/// Render the full frame to stdout. Clears the terminal first.
void render_frame(const SimplifiedGameState& state,
                  const GameSetup& setup,
                  const LogState& log,
                  const NdjsonState& ndjson,
                  const DisplayConfig& cfg,
                  const std::string& last_error = "");

/// Render the full frame to a string (no terminal clear; for testing).
std::string render_frame_to_string(const SimplifiedGameState& state,
                                   const GameSetup& setup,
                                   const LogState& log,
                                   const NdjsonState& ndjson,
                                   const DisplayConfig& cfg);

}  // namespace patchwork::tui
