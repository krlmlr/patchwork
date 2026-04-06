#include "display.hpp"
#include "../generated/patches.hpp"
#include "../terminal_and_scoring.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <string>
#include <vector>
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
// static const char* const kBdr_CT  = "\xe2\x94\xbc";  // ┼
static const char* const kArrow   = "\xe2\x96\xb6";  // ▶
// static const char* const kMult    = "\xc3\x97";       // ×

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

/// Build a horizontal separator with three T-junctions.
static std::string hline_3t(const char* left, const char* fill,
                             const char* jA, const char* jB, const char* jC,
                             const char* right,
                             int inner, int posA, int posB, int posC) {
    std::string out = left;
    for (int i = 0; i < inner; ++i) {
        if (i == posA) out += jA;
        else if (i == posB) out += jB;
        else if (i == posC) out += jC;
        else out += fill;
    }
    out += right;
    return out;
}

/// Build the NDJSON separator line that embeds the label and shortcuts inside
/// the fill.  `junctions` are ┴ positions (in IW inner space, 0-indexed);
/// the label+shortcuts text fills the space after the last junction.
static std::string ndjson_sep_line(int iw,
                                   std::initializer_list<int> junctions,
                                   int height) {
    // Build label and shortcuts (ASCII so 1 byte = 1 display col).
    char label[60];
    std::snprintf(label, sizeof(label), " ndjson log (%d lines) ", height);
    const char* shortcuts = "[m]v [f]^ [h]^/2 [,.]";
    int label_len   = static_cast<int>(std::strlen(label));
    int short_len   = static_cast<int>(std::strlen(shortcuts));

    // Find last junction position; text goes after it.
    int last_j = -1;
    for (int j : junctions) if (j > last_j) last_j = j;

    // Chars available after last_j (exclusive): iw - last_j - 1.
    int after = iw - last_j - 1;
    // Layout: ─ label fill shortcuts ─  (2 flanking dashes)
    int fill = after - 2 - label_len - short_len;
    if (fill < 0) fill = 0;

    std::string out = kBdr_LT;
    int ji = 0;  // current junction index
    std::vector<int> jvec(junctions);
    std::sort(jvec.begin(), jvec.end());
    for (int i = 0; i < iw; ++i) {
        if (ji < static_cast<int>(jvec.size()) && i == jvec[ji]) {
            out += kBdr_BT;
            ++ji;
        } else if (i == last_j + 1) {
            // Start embedded text region.
            out += kBdr_H;
            out += label;
            out += rep(kBdr_H, fill);
            out += shortcuts;
            out += kBdr_H;
            // Skip: we already output 1+label+fill+shortcuts+1 chars in this
            // iteration; advance i by the total minus 1 (the loop adds 1).
            i += label_len + fill + short_len + 1;
        } else {
            out += kBdr_H;
        }
    }
    out += kBdr_RT;
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

/// Print the NDJSON pane content lines (no header -- the header is embedded in
/// the separator line produced by ndjson_sep_line()).
/// `rows` is the effective number of rows to render (may differ from ndjson.height
/// when the pane is clamped to fit the terminal height).
static void print_ndjson_pane(const NdjsonState& ndjson, bool color, int iw, int rows) {
    int ndj_count = static_cast<int>(ndjson.lines.size());
    int start = std::max(0, ndj_count - rows);
    for (int k = 0; k < rows; ++k) {
        int idx = start + k;
        if (idx < ndj_count) {
            const std::string& raw = ndjson.lines[static_cast<std::size_t>(idx)];
            // NDJSON event data is always ASCII so raw.size() == visual width.
            int raw_vis = std::min(static_cast<int>(raw.size()), iw - 4);
            std::string raw_trunc = raw.substr(0, static_cast<std::size_t>(raw_vis));
            std::string line = color ? colorize_ndjson(raw_trunc) : raw_trunc;
            print_row(line, raw_vis, iw);
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
        // right_txt contains kArrow (3 bytes, 1 vis) and kBdr_H (3 bytes, 1 vis):
        // 4 extra bytes.  Use visible width = byte_size - 4 for the mid calculation.
        int right_vis = static_cast<int>(right_txt.size()) - 4;
        int mid = IW - static_cast<int>(left_txt.size()) - right_vis;
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
    // Fixed frame rows (excluding detail and ndjson): 18
    //   top(1)+circle(2)+stats-sep(1)+stats(1)+quilt-sep(1)+quilt-header(1)+
    //   quilt-data(9)+ndjson-sep(1)+bottom(1) = 18
    // Target total = cfg.height - 1 (fill terminal minus last line).
    // Effective ndjson rows (clamped to fit terminal):
    int max_ndjson_narrow = std::max(0, cfg.height - 22);  // = height-1-18-3
    int eff_ndjson = std::min(ndjson.height, max_ndjson_narrow);
    // Detail fills remaining space (at least 3).
    int detail_count = std::max(3, cfg.height - 19 - eff_ndjson);
    auto buyable = collect_buyable(state, setup, detail_count);
    int p_buttons = state.player(active).buttons();

    static const char* const kKeys[] = {
        "[1/2/3]buy  [a]adv      [q]quit        ",
        "[z/u]undo [Z/r]redo   [</>]log  [w]wrap",
        "[m]v [f]^ [h]^/2  [,]- [.]+            "
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
                          "[%2d] %c  cost %2d  time %2d  inc %d",
                          i + 1, p.name, p.buttons, p.time, p.income);
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
                          "[%2d] %c  cost %2d  time %2d  inc %d",
                          i + 1, p.name, p.buttons, p.time, p.income);
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
    // Row: │ stat0(mid_col) │ stat1(IW-mid_col-1) │  →  1+mid_col+1+(IW-mid_col-1)+1 = IW+2 ✓
    auto pad_to = [](const char* s, int width) -> std::string {
        std::string out(s);
        if (static_cast<int>(out.size()) < width)
            out.append(static_cast<std::size_t>(width - static_cast<int>(out.size())), ' ');
        else out.resize(static_cast<std::size_t>(width));
        return out;
    };
    std::string p0col = C ? (std::string(kColorP1) + (active == 0 ? kBold : "")
                              + pad_to(stat0, mid_col) + kReset)
                           : pad_to(stat0, mid_col);
    std::string p1col = C ? (std::string(kColorP2) + (active == 1 ? kBold : "")
                              + pad_to(stat1, IW - mid_col - 1) + kReset)
                           : pad_to(stat1, IW - mid_col - 1);
    std::printf("%s%s%s%s%s\n",
                kBdr_V, p0col.c_str(), kBdr_V, p1col.c_str(), kBdr_V);

    // ── Quilt+log section separator  ├──Q──┬──Q──┬──────┴──────┤
    // Q_INNER = 11: each quilt col is " ????????? " = 11 display cols.
    // Junction positions (0-indexed in IW inner space):
    //   pos_q1 = Q_INNER      (┬: divides Q1 from Q2)
    //   pos_q2 = Q_INNER*2+1  (┬: divides Q2 from event log)
    //   mid_col                (┴: aligns with the stats ┬ above)
    constexpr int Q_INNER = 11;
    const int pos_q1 = Q_INNER;
    const int pos_q2 = Q_INNER * 2 + 1;
    std::printf("%s\n",
        hline_3t(kBdr_LT, kBdr_H, kBdr_TT, kBdr_TT, kBdr_BT, kBdr_RT,
                 IW, pos_q1, pos_q2, mid_col).c_str());

    // ── Quilt + log body (header row + 9 data rows).
    // event log column inner width = IW - Q_INNER - 1 - Q_INNER - 1 = IW-24
    const int EVENT_INNER = IW - Q_INNER - 1 - Q_INNER - 1;
    // usable log content = EVENT_INNER - 2 (leading/trailing space absorbed by format)
    const int log_pane_width = EVENT_INNER;

    // Header row: │ P1 quilt  │ P2 quilt  │ Event log ... │
    // Visible: 1(│)+11(Q1)+1(│)+11(Q2)+1(│)+EVENT(log)+1(│) = IW+2 ✓
    // Format: " P1 quilt  │ P2 quilt  │ %-*s" where %-*s fills EVENT-1 chars
    // (" " + padded text = 1 + (EVENT-1) = EVENT)
    {
        char hdr[200];
        std::snprintf(hdr, sizeof(hdr), " P1 quilt  %s P2 quilt  %s %-*s",
                      kBdr_V, kBdr_V, log_pane_width - 1, "Event log");
        std::printf("%s%s%s\n", kBdr_V, hdr, kBdr_V);
    }

    int log_total = static_cast<int>(log.entries.size());
    int log_visible = std::min(9, log_total);
    int log_offset = log_total - log_visible;

    for (int row = 0; row < 9; ++row) {
        std::string log_cell;
        int log_cell_vis = 0;
        if (row < log_visible) {
            const std::string& e = log.entries[static_cast<std::size_t>(log_offset + row)];
            std::string display;
            int usable = log_pane_width - 4;  // space + "> " + space before │
            if (usable < 1) usable = 1;
            if (log.wrap_mode) {
                display = e.substr(0, static_cast<std::size_t>(usable));
            } else {
                int off = log.scroll_offset;
                if (off < static_cast<int>(e.size()))
                    display = e.substr(static_cast<std::size_t>(off),
                                       static_cast<std::size_t>(usable));
            }
            const char* pc = C ? kColorGreen : "";
            const char* pr = C ? kReset : "";
            log_cell = std::string(pc) + "> " + pr + display;
            log_cell_vis = 2 + static_cast<int>(display.size());
        }
        // Pad event column: total EVENT_INNER = 1(space)+log_cell+pad+1(space)
        int pad = EVENT_INNER - 2 - log_cell_vis;
        if (pad < 0) pad = 0;
        // Print: │ ????????? │ ????????? │ [space][log_cell][pad][space] │
        std::printf("%s ????????? %s ????????? %s %s%s %s\n",
                    kBdr_V, kBdr_V, kBdr_V,
                    log_cell.c_str(),
                    std::string(static_cast<std::size_t>(pad), ' ').c_str(),
                    kBdr_V);
    }

    // ── NDJSON separator with embedded label: ├──┴──┴─ ndjson ... ─┤
    std::printf("%s\n",
        ndjson_sep_line(IW, {pos_q1, pos_q2}, eff_ndjson).c_str());

    // ── NDJSON pane content rows.
    print_ndjson_pane(ndjson, C, IW, eff_ndjson);

    // ── Bottom border.
    std::printf("%s\n", hline(kBdr_BL, kBdr_H, kBdr_BR, IW).c_str());
}

// ── Wide layout (≥160 cols) ───────────────────────────────────────────────
//
// Column structure:  │ LEFT(78) │ Q1(11) │ Q2(11) │ EVENT(IW-102) │
//
// The left section uses the same format as the narrow layout (IW_NARROW=78).
// Two quilt columns (Q1, Q2) and the event log are explicit peer columns.
// Stats row is shown at the bottom of the left section via a sub-separator.

static void render_wide(const SimplifiedGameState& state,
                         const GameSetup& setup,
                         const LogState& log,
                         const NdjsonState& ndjson,
                         const DisplayConfig& cfg) {
    const int W  = cfg.width;
    const int IW = W - 2;
    const bool C = cfg.color_enabled;
    const int active = state.active_player();

    // Column constants (display cols).
    constexpr int LEFT = 78;    // inner width of left section
    constexpr int Q_INNER = 11; // each quilt column (space+9×?+space)
    const int EVENT = IW - LEFT - 1 - Q_INNER - 1 - Q_INNER - 1;

    // T-junction positions (0-indexed in IW inner space).
    const int pos_lq  = LEFT;                          // left / Q1
    const int pos_q1q2 = LEFT + Q_INNER + 1;           // Q1  / Q2
    const int pos_q2ev = LEFT + Q_INNER + 1 + Q_INNER + 1; // Q2 / event
    // Stats mid within left section.
    const int stats_mid = LEFT / 2;

    // ── Top border: ┌─left─┬─Q1─┬─Q2─┬─event─▶P1─┐
    {
        std::string left_txt = " PATCHWORK -- seed ? / setup 0 ";
        std::string right_txt = std::string(" ") + kArrow + " P"
                              + char('1' + active) + " " + kBdr_H;
        // right_txt contains kArrow (3B→1vis) and kBdr_H (3B→1vis): 4 extra bytes.
        int right_vis = static_cast<int>(right_txt.size()) - 4;
        int left_fill = LEFT - static_cast<int>(left_txt.size());
        if (left_fill < 0) left_fill = 0;
        int event_fill = EVENT - right_vis;
        if (event_fill < 0) event_fill = 0;
        std::printf("%s%s%s%s%s%s%s%s%s%s%s\n",
                    kBdr_TL,
                    left_txt.c_str(),
                    rep(kBdr_H, left_fill).c_str(),
                    kBdr_TT,
                    rep(kBdr_H, Q_INNER).c_str(),
                    kBdr_TT,
                    rep(kBdr_H, Q_INNER).c_str(),
                    kBdr_TT,
                    rep(kBdr_H, event_fill).c_str(),
                    right_txt.c_str(),
                    kBdr_TR);
    }

    // ── Build detail rows (same format as narrow, left section = LEFT inner).
    // Wide layout has TOTAL=10 fixed body rows; detail fills rows 2..(TOTAL-3).
    // For width-adaptive detail: wider terminals show more lines.
    constexpr int TOTAL = 10;  // quilt header + 9 data rows (also used below)
    int extra = std::max(0, (W - 80) / 10);
    int detail_count = std::min(TOTAL - 4, 3 + extra);  // at most TOTAL-4 = 6
    auto buyable = collect_buyable(state, setup, detail_count);
    int p_buttons = state.player(active).buttons();

    static const char* const kWideKeys[] = {
        "[1/2/3]buy  [a]adv      [q]quit        ",
        "[z/u]undo [Z/r]redo   [</>]log  [w]wrap",
        "[m]v [f]^ [h]^/2  [,]- [.]+            "
    };

    // Effective NDJSON rows clamped to fit terminal.
    // Wide total: top(1)+TOTAL(10)+ndjson_sep(1)+ndjson+bottom(1) = 13+ndjson
    int max_ndjson_wide = std::max(0, cfg.height - 14);
    int eff_ndjson = std::min(ndjson.height, max_ndjson_wide);

    // ── Build right (quilt+event) columns.
    int log_total   = static_cast<int>(log.entries.size());
    int log_visible = std::min(9, log_total);
    int log_offset  = log_total - log_visible;

    // Pad a string to `width` display cols.
    auto pad_to = [](const std::string& s, int width) -> std::string {
        if (static_cast<int>(s.size()) >= width)
            return s.substr(0, static_cast<std::size_t>(width));
        return s + std::string(static_cast<std::size_t>(width - static_cast<int>(s.size())), ' ');
    };

    // Returns the event column string (EVENT display cols) for quilt row `qrow`
    // (-1 = header).
    auto event_col = [&](int qrow) -> std::string {
        if (qrow < 0) {
            // Header.
            return pad_to(" Event log", EVENT);
        }
        if (qrow < log_visible) {
            const std::string& e = log.entries[static_cast<std::size_t>(log_offset + qrow)];
            std::string display;
            int usable = EVENT - 4;
            if (usable < 1) usable = 1;
            if (log.wrap_mode) {
                display = e.substr(0, static_cast<std::size_t>(usable));
            } else {
                int off = log.scroll_offset;
                if (off < static_cast<int>(e.size()))
                    display = e.substr(static_cast<std::size_t>(off),
                                       static_cast<std::size_t>(usable));
            }
            const char* pc = C ? kColorGreen : "";
            const char* pr = C ? kReset : "";
            std::string cell = std::string(pc) + "> " + pr + display;
            int cell_vis = 2 + static_cast<int>(display.size());
            int epad = EVENT - 2 - cell_vis;
            if (epad < 0) epad = 0;
            return " " + cell + std::string(static_cast<std::size_t>(epad), ' ') + " ";
        }
        return std::string(static_cast<std::size_t>(EVENT), ' ');
    };

    // Returns Q1/Q2 content (Q_INNER=11 display cols) for quilt row `qrow`
    // (-1 = header, 0..8 = data).
    auto quilt_col = [&](int which, int qrow) -> std::string {
        if (qrow < 0) {
            // Header: " P1 quilt  " or " P2 quilt  "
            return which == 0 ? " P1 quilt  " : " P2 quilt  ";
        }
        return " ????????? ";
    };

    // Total rows in the main body:
    //   Row 0:               circle  |  quilt header
    //   Row 1:               marker  |  quilt data row 0
    //   Rows 2..detail+1:   detail  |  quilt data rows 1..(detail)
    //   Row detail+2:        blank   |  quilt data row detail+1   (if needed)
    //   ...
    //   Row TOTAL-2:         stats sep |  quilt data row TOTAL-3
    //   Row TOTAL-1:         stats   |  quilt data row TOTAL-2
    // TOTAL = 10 (quilt header + 9 data rows, defined above)

    // Build left section content strings (plain + colored).
    std::vector<std::string> lplain(TOTAL), lcolored(TOTAL);
    // Row 0: Circle.
    lplain[0] = lcolored[0] = "Circle: " + circle_line(state, setup);
    // Row 1: Marker.
    lplain[1] = lcolored[1] = "        "
        + std::string(static_cast<std::size_t>(state.circle_marker()), ' ') + "^";
    // Rows 2..(2+detail_count-1): Detail rows with inline key hints.
    for (int i = 0; i < detail_count && (2 + i) < TOTAL - 2; ++i) {
        const char* key_hint = (i < 3) ? kWideKeys[i] : "";
        int key_len = static_cast<int>(std::strlen(key_hint));
        // content column = LEFT - 2 - key_len (matching narrow formula with IW=LEFT)
        int detail_width = LEFT - 2 - key_len;

        std::string plain_buf;
        if (i < static_cast<int>(buyable.size())) {
            int id = buyable[static_cast<std::size_t>(i)];
            auto& p = kPatches[static_cast<std::size_t>(id)];
            char buf[60];
            std::snprintf(buf, sizeof(buf),
                          "[%2d] %c  cost %2d  time %2d  inc %d",
                          i + 1, p.name, p.buttons, p.time, p.income);
            plain_buf = buf;
        }
        int pad = detail_width - static_cast<int>(plain_buf.size());
        if (pad < 0) pad = 0;
        // Assemble: plain_buf + pad + key_hint (total = LEFT-2)
        std::string plain_row = plain_buf
            + std::string(static_cast<std::size_t>(pad), ' ')
            + key_hint;
        lplain[2 + i] = plain_row;
        if (C && i < static_cast<int>(buyable.size())) {
            int id = buyable[static_cast<std::size_t>(i)];
            auto& p = kPatches[static_cast<std::size_t>(id)];
            bool affordable = (p_buttons >= p.buttons);
            const char* col = affordable ? kColorAff : kDim;
            lcolored[2 + i] = col + plain_buf + kReset
                + std::string(static_cast<std::size_t>(pad), ' ')
                + key_hint;
        } else {
            lcolored[2 + i] = plain_row;
        }
    }
    // Remaining rows before stats: blank.
    for (int r = 2 + detail_count; r < TOTAL - 2; ++r) {
        lplain[r] = lcolored[r] = "";
    }
    // Rows TOTAL-2 and TOTAL-1: stats separator + stats content (handled below).

    // Stats strings.
    char stat0[60], stat1[60];
    std::snprintf(stat0, sizeof(stat0),
                  " P1  btn %3d  inc %2d  pos %2d  fr %2d ",
                  state.player(0).buttons(), state.player(0).income(),
                  state.player(0).position(), state.player(0).free_spaces());
    std::snprintf(stat1, sizeof(stat1),
                  " P2  btn %3d  inc %2d  pos %2d  fr %2d ",
                  state.player(1).buttons(), state.player(1).income(),
                  state.player(1).position(), state.player(1).free_spaces());
    // Each stat padded to stats_mid and LEFT-stats_mid-1 cols.
    std::string p0col_str = pad_to(stat0, stats_mid);
    std::string p1col_str = pad_to(stat1, LEFT - stats_mid - 1);
    std::string p0col_colored = C
        ? (std::string(kColorP1) + (active == 0 ? kBold : "") + p0col_str + kReset)
        : p0col_str;
    std::string p1col_colored = C
        ? (std::string(kColorP2) + (active == 1 ? kBold : "") + p1col_str + kReset)
        : p1col_str;

    // ── Print main rows.
    for (int row = 0; row < TOTAL; ++row) {
        // quilt row index: row 0 = header (-1), rows 1..9 = data 0..8
        int qrow = row - 1;

        std::string q1 = quilt_col(0, qrow);
        std::string q2 = quilt_col(1, qrow);
        std::string ev = event_col(qrow);

        if (row == TOTAL - 2) {
            // Stats separator: ├──stats_mid──┬──rest──┤ Q1 │ Q2 │ event │
            std::string sep = kBdr_LT
                + rep(kBdr_H, stats_mid) + kBdr_TT
                + rep(kBdr_H, LEFT - stats_mid - 1) + kBdr_RT;
            std::printf("%s%s%s%s%s%s%s\n",
                        sep.c_str(),
                        q1.c_str(), kBdr_V,
                        q2.c_str(), kBdr_V,
                        ev.c_str(), kBdr_V);
        } else if (row == TOTAL - 1) {
            // Stats row: │ P1 stats │ P2 stats │ Q1 │ Q2 │ event │
            std::printf("%s%s%s%s%s%s%s%s%s%s%s\n",
                        kBdr_V,
                        p0col_colored.c_str(), kBdr_V,
                        p1col_colored.c_str(), kBdr_V,
                        q1.c_str(), kBdr_V,
                        q2.c_str(), kBdr_V,
                        ev.c_str(), kBdr_V);
        } else {
            // Normal row: │ left_content │ Q1 │ Q2 │ event │
            const std::string& plain = lplain[static_cast<std::size_t>(row)];
            const std::string& colored = lcolored[static_cast<std::size_t>(row)];
            int plain_len = static_cast<int>(plain.size());
            int lpad = LEFT - 2 - plain_len;
            if (lpad < 0) lpad = 0;
            std::printf("%s %s%s %s%s%s%s%s%s%s\n",
                        kBdr_V,
                        colored.c_str(),
                        std::string(static_cast<std::size_t>(lpad), ' ').c_str(),
                        kBdr_V,
                        q1.c_str(), kBdr_V,
                        q2.c_str(), kBdr_V,
                        ev.c_str(), kBdr_V);
        }
    }

    // ── NDJSON separator: ├──stats_mid──┴──left_rest──┴──Q1──┴──Q2──┴─label─┤
    std::printf("%s\n",
        ndjson_sep_line(IW, {stats_mid, pos_lq, pos_q1q2, pos_q2ev},
                        eff_ndjson).c_str());

    // ── NDJSON pane content rows.
    print_ndjson_pane(ndjson, C, IW, eff_ndjson);

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
    std::fflush(stdout);  // flush any buffered output before redirecting
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
