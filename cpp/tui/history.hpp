#pragma once
#include <random>
#include <vector>

#include "../simplified_game_state.hpp"

namespace patchwork::tui {

/// Snapshot of a player's mt19937 state.
using RngState = std::mt19937;

/// One entry in the undo/redo history.
struct HistoryEntry {
    SimplifiedGameState state;
    RngState rng_p0;                       // player 0 (human) RNG stream
    RngState rng_p1;                       // player 1 (opponent agent) RNG stream
    std::vector<std::string> log_entries;  // event-log snapshot at this point
};

/// Cursor-based undo/redo history of (GameState, per-player RngState) entries.
class History {
   public:
    /// Construct with the initial game state and both players' RNG snapshots.
    History(SimplifiedGameState initial_state, RngState initial_rng_p0, RngState initial_rng_p1);

    /// Push a new entry (with log snapshot), truncating any redo branch above
    /// the cursor.
    void push(SimplifiedGameState state, RngState rng_p0, RngState rng_p1,
              std::vector<std::string> log_entries = {});

    /// Move cursor back by one. No-op if already at the beginning.
    void undo();

    /// Move cursor forward by one. No-op if already at the end.
    void redo();

    /// Return the state at the current cursor position.
    [[nodiscard]] const SimplifiedGameState& current_state() const;

    /// Return player 0's RNG snapshot at the current cursor position.
    [[nodiscard]] const RngState& current_rng_p0() const;

    /// Return player 1's RNG snapshot at the current cursor position.
    [[nodiscard]] const RngState& current_rng_p1() const;

    /// Return the event-log snapshot at the current cursor position.
    [[nodiscard]] const std::vector<std::string>& current_log_entries() const;

    [[nodiscard]] bool can_undo() const;
    [[nodiscard]] bool can_redo() const;

   private:
    std::vector<HistoryEntry> entries_;
    int cursor_{0};
};

}  // namespace patchwork::tui
