#include "input.hpp"
#include "../move_generation.hpp"
#include "../terminal_and_scoring.hpp"

#include <cstdio>
#include <termios.h>
#include <unistd.h>

namespace patchwork::tui {

// ── RawMode ───────────────────────────────────────────────────────────────

struct SavedTermiosData {
    struct termios orig;
};

static SavedTermiosData* g_saved = nullptr;

static void restore_termios_atexit() {
    if (g_saved) tcsetattr(STDIN_FILENO, TCSAFLUSH, &g_saved->orig);
}

struct RawMode::SavedTermios : SavedTermiosData {};

RawMode::RawMode() : saved_(new SavedTermios{}) {
    tcgetattr(STDIN_FILENO, &saved_->orig);
    g_saved = saved_;
    std::atexit(restore_termios_atexit);

    struct termios raw = saved_->orig;
    raw.c_lflag &= ~static_cast<tcflag_t>(ICANON | ECHO);
    raw.c_cc[VMIN]  = 1;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

RawMode::~RawMode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &saved_->orig);
    g_saved = nullptr;
    delete saved_;
    saved_ = nullptr;
}

// ── read_command ──────────────────────────────────────────────────────────

Command read_command() {
    for (;;) {
        char c = '\0';
        if (read(STDIN_FILENO, &c, 1) <= 0) continue;
        if (c >= '1' && c <= '3') return BuyPatchCmd{c - '1'};
        if (c == 'a' || c == ' ') return AdvanceCmd{};
        if (c == 'z' || c == 'u') return UndoCmd{};
        if (c == 'Z' || c == 'r') return RedoCmd{};
        if (c == '<')              return ScrollLogLeft{};
        if (c == '>')              return ScrollLogRight{};
        if (c == 'w')              return ToggleLogWrap{};
        if (c == 'm')              return NdjsonToggleMinimize{};
        if (c == 'f')              return NdjsonMaximize{};
        if (c == 'h')              return NdjsonSemiMaximize{};
        if (c == ',')              return NdjsonDecrLines{};
        if (c == '.')              return NdjsonIncrLines{};
        if (c == 'q' || c == 'Q') return QuitCmd{};
    }
}

// ── is_legal ─────────────────────────────────────────────────────────────

bool is_legal(const Command& cmd,
              const SimplifiedGameState& state,
              const GameSetup& setup) {
    if (is_terminal(state)) return false;
    auto moves = legal_moves(state, setup);
    if (std::holds_alternative<BuyPatchCmd>(cmd)) {
        int idx = std::get<BuyPatchCmd>(cmd).index;
        for (auto& m : moves) {
            if (std::holds_alternative<BuyPatch>(m) &&
                std::get<BuyPatch>(m).patch_index == idx)
                return true;
        }
        return false;
    }
    if (std::holds_alternative<AdvanceCmd>(cmd)) {
        for (auto& m : moves)
            if (std::holds_alternative<Advance>(m)) return true;
        return false;
    }
    return false;
}

}  // namespace patchwork::tui
