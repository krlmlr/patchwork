#include "display.hpp"
#include "../generated/patches.hpp"
#include "../terminal_and_scoring.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <sys/ioctl.h>
#include <unistd.h>

namespace patchwork::tui {

namespace {

// ── Box-drawing helpers (UTF-8 encoded) ─────────────────────────────────
// Each box-drawing char is 3 bytes in UTF-8.
static const char* const kBdr_TL  = "\xe2\x94\x8c";  // ┌
static const char* const kBdr_TR  = "\xe2\x94\x90";  // ┐
static const char* const kBdr_BL  = "\xe2\x94\x94";  // └
static const char* const kBdr_BR  = "\xe2\x94\x98";  // ┘
static const char* const kBdr_H   = "\xe2\x94\x80";  // ─
static const char* const kBdr_V   = "\xe2\x94\x82";  // │
static const char* const kBdr_LT  = "\xe2\x94\x9c";  // ├
static const char* const kBdr_RT  = "\xe2\x94\xa4";  // ┤
static const char* const kBdr_TT  = "\xe2\x94\xac";  // ┬
static const char* const kBdr_BT  = "\xe2\x94\xb4";  // ┴
static const char* const kBdr_CT  = "\xe2\x94\xbc";  // ┼
static const char* const kArrow   = "\xe2\x96\xb6";  // ▶
static const char* const kMult    = "\xc3\x97";       // ×

/// Repeat a UTF-8 string `n` times.
static std::string rep(const char* s, int n) {
    std::string out;
    for (int i = 0; i < n; ++i) out += s;
    return out;
}

/// Build a horizontal separator line: left + fill*n + right.
static std::string hline(const char* left, const char* fill, const char* right, int inner) {
    return std::string(left) + rep(fill, inner) + right;
}

/// Build a horizontal separator with a T-junction at position `pos`
/// (0-based from left inner edge).
static std::string hline_t(const char* left, const char* fill,
                            const char* junction, const char* right,
                            int inner, int junction_pos) {
    std::string out = left;
    for (int i = 0; i < inner; ++i) {
        if (i == junction_pos) out += junction;
        else out += fill;
    }
    out += right;
    return out;
}

/// Build a horizontal separator with two T-junctions (left and right of mid).
static std::string hline_2t(const char* left, const char* fill,
                             const char* juncL, const char* juncR,
                             const char* right,
                             int inner, int posL, int posR) {
    std::string out = left;
    for (int i = 0; i < inner; ++i) {
        if (i == posL) out += juncL;
        else if (i == posR) out += juncR;
        else out += fill;
    }
    out += right;
    return out;
}

/// Print a full-width box row: "│ content padded to IW │".
/// `content` is plain text (no ANSI codes in the visible count).
/// `visible_len` is the display width of `content` (chars, not bytes).
static void print_row(const std::string& content, int visible_len, int iw) {
    int pad = iw - 2 - visible_len;
    if (pad < 0) pad = 0;
    std::printf("%s %s%s %s\n",
                kBdr_V, content.c_str(),
                std::string(static_cast<std::size_t>(pad), ' ').c_str(),
                kBdr_V);
}

/// Render patch-circle sequence: 33 chars.
static std::string circle_line(const SimplifiedGameState& state,
                                const GameSetup& setup) {
    std::string out;
    for (int i = 0; i < 33; ++i) {
        uint8_t id = setup.circle()[static_cast<std::size_t>(i)];
        out += state.patch_available(static_cast<int>(id)) ? kPatches[id].name : '.';
    }
    return out;
}

/// Time-track bar, `bar_width` ASCII characters.
static std::string time_track(int pos0, int pos1, int bar_width) {
    if (bar_width < 6) return "";
    std::string bar(static_cast<std::size_t>(bar_width), '-');
    bar[0] = '0';
    bar[static_cast<std::size_t>(bar_width - 2)] = '5';
    bar[static_cast<std::size_t>(bar_width - 1)] = '3';

    auto map_pos = [&](int p) -> int {
        int usable = bar_width - 3;
        if (usable <= 0) return 1;
        int c = std::clamp(p, 0, 52);
        return 1 + static_cast<int>(std::round(c / 52.0 * usable));
    };
    int ip1 = map_pos(pos0);
    int ip2 = map_pos(pos1);

    auto place = [&](int pos, const char* tag) {
        for (int j = 0; tag[j] && pos + j < bar_width - 2; ++j)
            bar[static_cast<std::size_t>(pos + j)] = tag[j];
    };
    if (ip1 <= ip2) { place(ip2, "P2"); place(ip1, "P1"); }
    else             { place(ip1, "P1"); place(ip2, "P2"); }
    return bar;
}

/// Colorize a raw NDJSON line with concept colors.
static std::string colorize_ndjson(const std::string& raw) {
    std::string out;
    out.reserve(raw.size() * 4);
    std::size_t i = 0;
    std::string pending_key;

    while (i < raw.size()) {
        char c = raw[i];
        if (c == '{' || c == '}' || c == ':' || c == ',') {
            out += kDim; out += c; out += kReset;
            ++i;
        } else if (c == '"') {
            std::size_t start = i++;
            while (i < raw.size() && raw[i] != '"') {
                if (raw[i] == '\\') ++i;
                ++i;
            }
            if (i < raw.size()) ++i;
            std::string token = raw.substr(start, i - start);
            std::string inner = token.substr(1, token.size() > 2 ? token.size() - 2 : 0);

            // Determine key vs value.
            std::size_t j = i;
            while (j < raw.size() && raw[j] == ' ') ++j;
            bool is_key = (j < raw.size() && raw[j] == ':');

            if (is_key) {
                pending_key = inner;
                out += token;
            } else {
                const char* col = nullptr;
                if (pending_key == "event") {
                    if (inner == "game_start" || inner == "game_end") col = kBold;
                } else if (pending_key == "move_type") {
                    col = (inner == "buy_patch") ? kColorAff : kColorCyan;
                } else if (pending_key == "winner") {
                    col = (inner == "0") ? kColorP1 : kColorP2;
                }
                if (col) { out += col; out += token; out += kReset; }
                else out += token;
                pending_key.clear();
            }
        } else if (c >= '0' && c <= '9') {
            std::size_t start = i;
            while (i < raw.size() && raw[i] >= '0' && raw[i] <= '9') ++i;
            std::string num = raw.substr(start, i - start);
            const char* col = nullptr;
            if (pending_key == "player" || pending_key == "winner")
                col = (num == "0") ? kColorP1 : kColorP2;
            if (col) { out += col; out += num; out += kReset; }
            else out += num;
            pending_key.clear();
        } else {
            out += c;
            ++i;
        }
    }
    return out;
}

/// Print the NDJSON pane (header bar + content lines).
static void print_ndjson_pane(const NdjsonState& ndjson, bool color, int iw) {
    // Header bar.
    char hdr_buf[80];
    std::snprintf(hdr_buf, sizeof(hdr_buf),
                  " ndjson log (%d lines) ", ndjson.height);
    const char* shortcuts = "[m]v [f]^ [h]^/2 [,.]";
    int hdr_len = static_cast<int>(std::strlen(hdr_buf))
                + static_cast<int>(std::strlen(shortcuts));
    int fill = iw - 2 - hdr_len;
    if (fill < 0) fill = 0;
    std::string hdr_line = hdr_buf;
    hdr_line.append(static_cast<std::size_t>(fill), '-');
    hdr_line += shortcuts;
    print_row(hdr_line, static_cast<int>(hdr_line.size()), iw);

    int ndj_count = static_cast<int>(ndjson.lines.size());
    int start = std::max(0, ndj_count - ndjson.height);
    for (int k = 0; k < ndjson.height; ++k) {
        int idx = start + k;
        if (idx < ndj_count) {
            std::string line = color
                ? colorize_ndjson(ndjson.lines[static_cast<std::size_t>(idx)])
                : ndjson.lines[static_cast<std::size_t>(idx)];
            // Truncate to iw-4 visible chars.
            if (static_cast<int>(line.size()) > iw - 4)
                line.resize(static_cast<std::size_t>(iw - 4));
            print_row(line, static_cast<int>(
                ndjson.lines[static_cast<std::size_t>(idx)].size()), iw);
        } else {
            print_row("", 0, iw);
        }
    }
}

/// Collect buyable patch IDs starting from the circle marker (up to max_detail).
static std::vector<int> collect_buyable(const SimplifiedGameState& state,
                                         const GameSetup& setup,
                                         int max_detail) {
    std::vector<int> ids;
    int marker = state.circle_marker();
    for (int off = 0; off < 33 && static_cast<int>(ids.size()) < max_detail; ++off) {
        int pos = (marker + off) % 33;
        int id = static_cast<int>(setup.circle()[static_cast<std::size_t>(pos)]);
        if (state.patch_available(id)) ids.push_back(id);
    }
    return ids;
}

// ── Narrow layout (80–159 cols) ──────────────────────────────────────────

static void render_narrow(const SimplifiedGameState& state,
                           const GameSetup& setup,
                           const LogState& log,
                           const NdjsonState& ndjson,
                           const DisplayConfig& cfg) {
    const int W  = cfg.width;
    const int IW = W - 2;
    const bool C = cfg.color_enabled;
    const int active = state.active_player();

    // ── Top border.
    {
        std::string left_txt = " PATCHWORK -- seed ? / setup 0 --";
        std::string right_txt = std::string(" ") + kArrow + " P"
                              + char('1' + active) + " " + kBdr_H;
        int mid = IW - static_cast<int>(left_txt.size())
                     - static_cast<int>(right_txt.size());
        if (mid < 0) mid = 0;
        std::printf("%s%s%s%s%s\n",
                    kBdr_TL, left_txt.c_str(),
                    rep(kBdr_H, mid).c_str(),
                    right_txt.c_str(), kBdr_TR);
    }

    // ── Circle + marker.
    {
        std::string circ_line_txt = "Circle: " + circle_line(state, setup);
        print_row(circ_line_txt, static_cast<int>(circ_line_txt.size()), IW);
        std::string mline = "        " + std::string(
            static_cast<std::size_t>(state.circle_marker()), ' ') + "^";
        print_row(mline, static_cast<int>(mline.size()), IW);
    }

    // ── Adaptive detail lines.
    int extra = std::max(0, (W - 80) / 10);
    int detail_count = 3 + extra;
    auto buyable = collect_buyable(state, setup, detail_count);
    int p_buttons = state.player(active).buttons();

    static const char* const kKeys[] = {
        "[0-x]buy  [a]adv      [q]quit          ",
        "[z/u]undo [Z/r]redo   [</>]log  [w]wrap",
        "[m]v [f]^ [h]^/2  [,]- [.]+             "
    };

    for (int i = 0; i < detail_count; ++i) {
        const char* key_hint = (i < 3) ? kKeys[i] : "";
        int key_len = static_cast<int>(std::strlen(key_hint));
        int detail_width = IW - 2 - key_len;

        std::string detail;
        if (i < static_cast<int>(buyable.size())) {
            int id = buyable[static_cast<std::size_t>(i)];
            auto& p = kPatches[static_cast<std::size_t>(id)];
            bool affordable = (p_buttons >= p.buttons);
            char buf[60];
            std::snprintf(buf, sizeof(buf),
                          "[%d] %c  cost %2d  time %2d  inc %d",
                          i, p.name, p.buttons, p.time, p.income);
            if (C) {
                const char* col = affordable ? kColorAff : kDim;
                detail = col + std::string(buf) + kReset;
            } else {
                detail = buf;
            }
        }
        // Pad detail to detail_width.
        // Build plain text for padding calculation.
        std::string plain_detail;
        if (i < static_cast<int>(buyable.size())) {
            int id = buyable[static_cast<std::size_t>(i)];
            auto& p = kPatches[static_cast<std::size_t>(id)];
            char buf[60];
            std::snprintf(buf, sizeof(buf),
                          "[%d] %c  cost %2d  time %2d  inc %d",
                          i, p.name, p.buttons, p.time, p.income);
            plain_detail = buf;
        }
        int plain_len = static_cast<int>(plain_detail.size());
        int pad = detail_width - plain_len;
        if (pad < 0) pad = 0;

        std::printf("%s ", kBdr_V);
        if (C && i < static_cast<int>(buyable.size())) {
            int id = buyable[static_cast<std::size_t>(i)];
            auto& p = kPatches[static_cast<std::size_t>(id)];
            bool affordable = (p_buttons >= p.buttons);
            const char* col = affordable ? kColorAff : kDim;
            std::printf("%s%s%s", col, plain_detail.c_str(), kReset);
        } else {
            std::printf("%s", plain_detail.c_str());
        }
        std::printf("%s%s %s\n",
                    std::string(static_cast<std::size_t>(pad), ' ').c_str(),
                    key_hint, kBdr_V);
    }

    // ── Stats separator  ├───────────┬───────────┤
    int mid_col = IW / 2;
    std::printf("%s\n", hline_t(kBdr_LT, kBdr_H, kBdr_TT, kBdr_RT, IW, mid_col).c_str());

    // ── Player stats row (split).
    char stat0[60], stat1[60];
    std::snprintf(stat0, sizeof(stat0),
                  " P1  btn %3d  inc %2d  pos %2d  fr %2d ",
                  state.player(0).buttons(), state.player(0).income(),
                  state.player(0).position(), state.player(0).free_spaces());
    std::snprintf(stat1, sizeof(stat1),
                  " P2  btn %3d  inc %2d  pos %2d  fr %2d  ",
                  state.player(1).buttons(), state.player(1).income(),
                  state.player(1).position(), state.player(1).free_spaces());

    // Pad stat0 to mid_col chars, stat1 to IW - mid_col - 1 chars.
    auto pad_to = [](const char* s, int width) -> std::string {
        std::string out(s);
        if (static_cast<int>(out.size()) < width)
            out.append(static_cast<std::size_t>(width - static_cast<int>(out.size())), ' ');
        else out.resize(static_cast<std::size_t>(width));
        return out;
    };
    std::string p0col = C ? (std::string(kColorP1) + (active == 0 ? kBold : "")
                              + pad_to(stat0, mid_col - 1) + kReset)
                           : pad_to(stat0, mid_col - 1);
    std::string p1col = C ? (std::string(kColorP2) + (active == 1 ? kBold : "")
                              + pad_to(stat1, IW - mid_col - 1) + kReset)
                           : pad_to(stat1, IW - mid_col - 1);
    std::printf("%s%s%s%s%s\n",
                kBdr_V, p0col.c_str(), kBdr_V, p1col.c_str(), kBdr_V);

    // ── Quilt+log section separator  ├──────────┬──────────┴──...──┤
    constexpr int Q_INNER = 11;  // "?????????" (9) + 2 spaces = 11 col
    constexpr int Q_OUTER = Q_INNER + 2;  // include border │
    int quilt_sep_L = Q_OUTER;
    int quilt_sep_R = Q_OUTER * 2;
    // ├─────────────────┬─────────────────┴──...──┤
    // posL = quilt_sep_L - 1, posR = quilt_sep_R - 1 (0-based inner offsets)
    std::printf("%s\n",
        hline_2t(kBdr_LT, kBdr_H, kBdr_TT, kBdr_BT, kBdr_RT,
                 IW, quilt_sep_L - 1, quilt_sep_R - 1).c_str());

    // ── Quilt + log body (header row + 9 data rows).
    int log_start_col = quilt_sep_R + 1;  // character after the right quilt separator
    int log_pane_width = IW - log_start_col;  // visible chars in log pane

    // Header row.
    auto log_header = [&]() {
        std::printf("%s P1 quilt (9%s9) %s P2 quilt (9%s9)  %-*s%s\n",
                    kBdr_V, kMult, kBdr_V, kMult,
                    log_pane_width - 1, "Event log", kBdr_V);
    };
    log_header();

    int log_total = static_cast<int>(log.entries.size());
    int log_visible = std::min(9, log_total);
    int log_offset = log_total - log_visible;

    for (int row = 0; row < 9; ++row) {
        std::string log_cell;
        if (row < log_visible) {
            const std::string& e = log.entries[static_cast<std::size_t>(log_offset + row)];
            std::string display;
            if (log.wrap_mode) {
                display = e.substr(0, static_cast<std::size_t>(log_pane_width - 3));
            } else {
                int off = log.scroll_offset;
                if (off < static_cast<int>(e.size()))
                    display = e.substr(static_cast<std::size_t>(off),
                                       static_cast<std::size_t>(log_pane_width - 3));
            }
            const char* pc = C ? kColorGreen : "";
            const char* pr = C ? kReset : "";
            log_cell = std::string(pc) + "> " + pr + display;
        }
        int log_cell_visible = row < log_visible
            ? 2 + static_cast<int>(
                  log.entries[static_cast<std::size_t>(log_offset + row)].size())
            : 0;
        // Clamp.
        int pad = log_pane_width - 2 - std::min(log_cell_visible, log_pane_width - 2);
        if (pad < 0) pad = 0;
        std::printf("%s ????????? %s ?????????  %s%s %s\n",
                    kBdr_V, kBdr_V,
                    log_cell.c_str(),
                    std::string(static_cast<std::size_t>(pad), ' ').c_str(),
                    kBdr_V);
    }

    // ── NDJSON separator  ├──────────┴──────────┴──...──┤
    std::printf("%s\n",
        hline_2t(kBdr_LT, kBdr_H, kBdr_BT, kBdr_BT, kBdr_RT,
                 IW, quilt_sep_L - 1, quilt_sep_R - 1).c_str());

    // ── NDJSON pane.
    print_ndjson_pane(ndjson, C, IW + 2);

    // ── Bottom border.
    std::printf("%s\n", hline(kBdr_BL, kBdr_H, kBdr_BR, IW).c_str());
}

// ── Wide layout (≥160 cols) ───────────────────────────────────────────────

static void render_wide(const SimplifiedGameState& state,
                         const GameSetup& setup,
                         const LogState& log,
                         const NdjsonState& ndjson,
                         const DisplayConfig& cfg) {
    const int W  = cfg.width;
    const int IW = W - 2;
    const bool C = cfg.color_enabled;
    const int active = state.active_player();
    const int LEFT = 65;  // left column inner width (excl. border │ on right)
    const int RIGHT = IW - LEFT - 1;

    // ── Top border with mid divider.
    {
        std::string left_txt = " PATCHWORK -- seed ? / setup 0 ";
        std::string right_txt = std::string(" ") + kArrow + " P"
                              + char('1' + active) + " " + kBdr_H;
        int right_fill = RIGHT - static_cast<int>(right_txt.size());
        if (right_fill < 0) right_fill = 0;
        std::printf("%s%s%s%s%s%s%s\n",
                    kBdr_TL,
                    left_txt.c_str(),
                    rep(kBdr_H, LEFT - static_cast<int>(left_txt.size())).c_str(),
                    kBdr_TT,
                    rep(kBdr_H, right_fill).c_str(),
                    right_txt.c_str(),
                    kBdr_TR);
    }

    // Build left column lines.
    int extra = std::max(0, (W - 80) / 10);
    int detail_count = std::min(6, 3 + extra);
    auto buyable = collect_buyable(state, setup, detail_count);
    int p_buttons = state.player(active).buttons();

    std::vector<std::string> left_rows;
    std::vector<int> left_visible;  // visible char count for each row

    auto push_left = [&](const std::string& visible_text, const std::string& raw_for_print) {
        left_rows.push_back(raw_for_print);
        left_visible.push_back(static_cast<int>(visible_text.size()));
    };

    // Circle.
    std::string cl = "Circle: " + circle_line(state, setup);
    push_left(cl, cl);
    std::string ml = "        " + std::string(static_cast<std::size_t>(state.circle_marker()), ' ') + "^";
    push_left(ml, ml);

    // Detail rows.
    for (int i = 0; i < detail_count; ++i) {
        if (i < static_cast<int>(buyable.size())) {
            int id = buyable[static_cast<std::size_t>(i)];
            auto& p = kPatches[static_cast<std::size_t>(id)];
            bool affordable = (p_buttons >= p.buttons);
            char buf[70];
            std::snprintf(buf, sizeof(buf),
                          " [%d] %c  cost %2d  time %2d  income %d",
                          i, p.name, p.buttons, p.time, p.income);
            if (C) {
                const char* col = affordable ? kColorAff : kDim;
                push_left(buf, col + std::string(buf) + kReset);
            } else {
                push_left(buf, buf);
            }
        } else {
            push_left("", "");
        }
    }
    push_left("", "");  // spacer

    char stat0[60], stat1[60];
    std::snprintf(stat0, sizeof(stat0), " P1  btn %3d  inc %2d  pos %2d  fr %2d",
                  state.player(0).buttons(), state.player(0).income(),
                  state.player(0).position(), state.player(0).free_spaces());
    std::snprintf(stat1, sizeof(stat1), " P2  btn %3d  inc %2d  pos %2d  fr %2d",
                  state.player(1).buttons(), state.player(1).income(),
                  state.player(1).position(), state.player(1).free_spaces());
    if (C) {
        push_left(stat0, kColorP1 + std::string(stat0) + kReset);
        push_left(stat1, kColorP2 + std::string(stat1) + kReset);
    } else {
        push_left(stat0, stat0);
        push_left(stat1, stat1);
    }

    std::string tt = " time: " + time_track(state.player(0).position(),
                                             state.player(1).position(), 40);
    push_left(tt, tt);
    push_left(" [0-x]buy  [a]adv  [q]quit",
              " [0-x]buy  [a]adv  [q]quit");
    push_left(" [z/u]undo  [Z/r]redo  [</>]log  [w]wrap",
              " [z/u]undo  [Z/r]redo  [</>]log  [w]wrap");
    push_left(" [m]v [f]^ [h]^/2  [,]decrL  [.]incrL",
              " [m]v [f]^ [h]^/2  [,]decrL  [.]incrL");

    // Right column: header + 9 quilt rows + empty.
    int log_total = static_cast<int>(log.entries.size());
    int log_visible = std::min(9, log_total);
    int log_offset = log_total - log_visible;
    int log_pane_width = RIGHT - 24;  // two quilts ~24 cols
    if (log_pane_width < 10) log_pane_width = 10;

    auto right_row = [&](int row) -> std::pair<std::string, int> {
        if (row == 0) {
            std::string s = "  P1 quilt (9";
            s += kMult; s += "9)     P2 quilt (9"; s += kMult; s += "9)     Event log";
            return {s, static_cast<int>(s.size()) - 6};  // kMult is 2 bytes but 1 visible
        }
        if (row >= 1 && row <= 9) {
            int qrow = row - 1;
            std::string log_cell;
            if (qrow < log_visible) {
                const std::string& e = log.entries[static_cast<std::size_t>(log_offset + qrow)];
                std::string display;
                if (log.wrap_mode) {
                    display = e.substr(0, static_cast<std::size_t>(log_pane_width));
                } else {
                    int off = log.scroll_offset;
                    if (off < static_cast<int>(e.size()))
                        display = e.substr(static_cast<std::size_t>(off),
                                           static_cast<std::size_t>(log_pane_width));
                }
                const char* pc = C ? kColorGreen : "";
                const char* pr = C ? kReset : "";
                log_cell = std::string(pc) + "> " + pr + display;
            }
            char buf[200];
            std::snprintf(buf, sizeof(buf), "  ?????????          ?????????          %s",
                          log_cell.c_str());
            int vis = 40 + static_cast<int>(
                (qrow < log_visible
                    ? log.entries[static_cast<std::size_t>(log_offset + qrow)].size()
                    : 0));
            return {buf, vis};
        }
        return {"", 0};
    };

    int max_rows = std::max(static_cast<int>(left_rows.size()), 11);

    for (int row = 0; row < max_rows; ++row) {
        std::string ltext = (row < static_cast<int>(left_rows.size()))
                            ? left_rows[static_cast<std::size_t>(row)] : "";
        int lvis = (row < static_cast<int>(left_visible.size()))
                   ? left_visible[static_cast<std::size_t>(row)] : 0;
        auto [rtext, rvis] = right_row(row);

        int lpad = std::max(0, LEFT - 2 - lvis);
        int rpad = std::max(0, RIGHT - rvis);

        std::printf("%s %s%s%s%s%s%s\n",
                    kBdr_V,
                    ltext.c_str(),
                    std::string(static_cast<std::size_t>(lpad), ' ').c_str(),
                    kBdr_V,
                    rtext.c_str(),
                    std::string(static_cast<std::size_t>(rpad), ' ').c_str(),
                    kBdr_V);
    }

    // ── NDJSON separator.
    std::printf("%s\n",
        hline_t(kBdr_LT, kBdr_H, kBdr_BT, kBdr_RT, IW, LEFT - 1).c_str());

    // ── NDJSON pane.
    print_ndjson_pane(ndjson, C, IW + 2);

    // ── Bottom border.
    std::printf("%s\n", hline(kBdr_BL, kBdr_H, kBdr_BR, IW).c_str());
}

}  // namespace

// ── Public API ────────────────────────────────────────────────────────────

DisplayConfig init_display(bool no_color_flag, int /*argc*/, char** /*argv*/) {
    DisplayConfig cfg;
    struct winsize ws {};
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0) {
        cfg.width  = static_cast<int>(ws.ws_col);
        cfg.height = static_cast<int>(ws.ws_row);
    }
    if (cfg.width < 80 || cfg.height < 24) {
        std::fprintf(stderr,
                     "Terminal too small: %dx%d. Minimum required: 80x24.\n",
                     cfg.width, cfg.height);
        std::exit(1);
    }
    if (no_color_flag) {
        cfg.color_enabled = false;
    } else if (std::getenv("NO_COLOR") != nullptr) {
        cfg.color_enabled = false;
    } else {
        const char* term = std::getenv("TERM");
        if (term && std::strcmp(term, "dumb") == 0) cfg.color_enabled = false;
    }
    return cfg;
}

void append_log(LogState& log, std::string entry) {
    log.entries.push_back(std::move(entry));
    if (static_cast<int>(log.entries.size()) > 50)
        log.entries.erase(log.entries.begin());
    log.scroll_offset = 0;
}

void append_ndjson(NdjsonState& ndjson, std::string line) {
    ndjson.lines.push_back(std::move(line));
}

void ndjson_toggle_minimize(NdjsonState& s, int /*max_lines*/) {
    if (s.height > 0) { s.prev_height = s.height; s.height = 0; }
    else s.height = s.prev_height > 0 ? s.prev_height : 5;
}
void ndjson_maximize(NdjsonState& s, int max_lines)   { s.prev_height = s.height; s.height = max_lines; }
void ndjson_semi_maximize(NdjsonState& s, int max_lines) { s.prev_height = s.height; s.height = max_lines / 2; }
void ndjson_decr_lines(NdjsonState& s)                { if (s.height > 0) --s.height; }
void ndjson_incr_lines(NdjsonState& s, int max_lines) { if (s.height < max_lines) ++s.height; }

void render_frame(const SimplifiedGameState& state,
                  const GameSetup& setup,
                  const LogState& log,
                  const NdjsonState& ndjson,
                  const DisplayConfig& cfg,
                  const std::string& /*last_error*/) {
    std::printf("\033[H\033[2J");
    std::fflush(stdout);
    if (cfg.width >= 160)
        render_wide(state, setup, log, ndjson, cfg);
    else
        render_narrow(state, setup, log, ndjson, cfg);
    std::fflush(stdout);
}

std::string render_frame_to_string(const SimplifiedGameState& state,
                                    const GameSetup& setup,
                                    const LogState& log,
                                    const NdjsonState& ndjson,
                                    const DisplayConfig& cfg) {
    int pipefd[2];
    if (pipe(pipefd) != 0) return {};
    int saved_stdout = dup(STDOUT_FILENO);
    dup2(pipefd[1], STDOUT_FILENO);
    close(pipefd[1]);

    if (cfg.width >= 160)
        render_wide(state, setup, log, ndjson, cfg);
    else
        render_narrow(state, setup, log, ndjson, cfg);
    std::fflush(stdout);

    dup2(saved_stdout, STDOUT_FILENO);
    close(saved_stdout);

    std::string result;
    char buf[4096];
    ssize_t n;
    while ((n = read(pipefd[0], buf, sizeof(buf))) > 0)
        result.append(buf, static_cast<std::size_t>(n));
    close(pipefd[0]);
    return result;
}

}  // namespace patchwork::tui
