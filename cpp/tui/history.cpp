#include "history.hpp"

namespace patchwork::tui {

History::History(SimplifiedGameState initial_state, RngState initial_rng) {
    entries_.push_back({std::move(initial_state), std::move(initial_rng), {}});
    cursor_ = 0;
}

void History::push(SimplifiedGameState state, RngState rng, std::vector<std::string> log_entries) {
    // Truncate redo branch.
    entries_.erase(entries_.begin() + cursor_ + 1, entries_.end());
    entries_.push_back({std::move(state), std::move(rng), std::move(log_entries)});
    cursor_ = static_cast<int>(entries_.size()) - 1;
}

void History::undo() {
    if (cursor_ > 0) --cursor_;
}

void History::redo() {
    if (cursor_ < static_cast<int>(entries_.size()) - 1) ++cursor_;
}

const SimplifiedGameState& History::current_state() const {
    return entries_[static_cast<std::size_t>(cursor_)].state;
}

const RngState& History::current_rng() const {
    return entries_[static_cast<std::size_t>(cursor_)].rng;
}

const std::vector<std::string>& History::current_log_entries() const {
    return entries_[static_cast<std::size_t>(cursor_)].log_entries;
}

bool History::can_undo() const { return cursor_ > 0; }
bool History::can_redo() const { return cursor_ < static_cast<int>(entries_.size()) - 1; }

}  // namespace patchwork::tui
