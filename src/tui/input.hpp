#pragma once
#include "../move.hpp"
#include "../simplified_game_state.hpp"
#include "../game_setup.hpp"

#include <variant>

namespace patchwork::tui {

// ── Command variant ───────────────────────────────────────────────────────

struct BuyPatchCmd   { int index; };
struct AdvanceCmd    {};
struct UndoCmd       {};
struct RedoCmd       {};
struct ScrollLogLeft {};
struct ScrollLogRight{};
struct ToggleLogWrap {};
struct NdjsonToggleMinimize {};
struct NdjsonMaximize       {};
struct NdjsonSemiMaximize   {};
struct NdjsonDecrLines      {};
struct NdjsonIncrLines      {};
struct QuitCmd       {};

using Command = std::variant<
    BuyPatchCmd, AdvanceCmd, UndoCmd, RedoCmd,
    ScrollLogLeft, ScrollLogRight, ToggleLogWrap,
    NdjsonToggleMinimize, NdjsonMaximize, NdjsonSemiMaximize,
    NdjsonDecrLines, NdjsonIncrLines,
    QuitCmd>;

// ── RawMode RAII guard ────────────────────────────────────────────────────

/// Switches stdin to raw mode on construction; restores original termios on
/// destruction. Also registers an atexit handler as a safety net.
class RawMode {
public:
    RawMode();
    ~RawMode();

    RawMode(const RawMode&) = delete;
    RawMode& operator=(const RawMode&) = delete;

private:
    struct SavedTermios;
    SavedTermios* saved_{nullptr};
};

// ── Input functions ───────────────────────────────────────────────────────

/// Block until a valid Command keypress is read. Unrecognised keys are ignored.
Command read_command();

/// Return true if `cmd` maps to a legal move in `state`.
bool is_legal(const Command& cmd,
              const SimplifiedGameState& state,
              const GameSetup& setup);

}  // namespace patchwork::tui
