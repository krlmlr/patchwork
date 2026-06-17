#pragma once
#include <random>
#include <vector>

#include "../simplified_game_state.hpp"

namespace patchwork::tui {

/// Snapshot of the random agent's mt19937 state.
using RngState = std::mt19937;

/// One entry in the undo/redo history.
struct HistoryEntry {
    SimplifiedGameState state;
    RngState rng;
    std::vector<std::string> log_entries;  // event-log snapshot at this point
};

/// Cursor-based undo/redo history of (GameState, RngState) pairs.
class History {
   public:
    /// Construct with the initial game state and RNG snapshot.
    History(SimplifiedGameState initial_state, RngState initial_rng);

    /// Push a new entry (with log snapshot), truncating any redo branch above
    /// the cursor.
    void push(SimplifiedGameState state, RngState rng, std::vector<std::string> log_entries = {});

    /// Move cursor back by one. No-op if already at the beginning.
    void undo();

    /// Move cursor forward by one. No-op if already at the end.
    void redo();

    /// Return the state at the current cursor position.
    [[nodiscard]] const SimplifiedGameState& current_state() const;

    /// Return the RNG snapshot at the current cursor position.
    [[nodiscard]] const RngState& current_rng() const;

    /// Return the event-log snapshot at the current cursor position.
    [[nodiscard]] const std::vector<std::string>& current_log_entries() const;

    [[nodiscard]] bool can_undo() const;
    [[nodiscard]] bool can_redo() const;

   private:
    std::vector<HistoryEntry> entries_;
    int cursor_{0};
};

}  // namespace patchwork::tui
