#ifndef PATCHWORK_BIASED_RANDOM_AGENT_HPP
#define PATCHWORK_BIASED_RANDOM_AGENT_HPP

#include <functional>
#include <random>

#include "game_setup.hpp"
#include "generated/patches.hpp"
#include "move.hpp"
#include "simplified_game_state.hpp"

namespace patchwork {

/// Built-in weight function: prefers lower button-cost patches.
/// Returns `1.0 / (patch.buttons + 1)` (cost 0 → 1.0).
[[nodiscard]] double weight_cheap(const PatchData& patch);

/// Built-in weight function: prefers higher button-income patches.
/// Returns `static_cast<double>(patch.income + 1)` (income 0 → 1.0).
[[nodiscard]] double weight_income(const PatchData& patch);

/// Built-in weight function: prefers better income-to-time-cost ratio.
/// Returns `(patch.income + 0.5) / patch.time`.
[[nodiscard]] double weight_income_per_time(const PatchData& patch);

/// Select a legal move by weighted sampling.
///
/// Each `BuyPatch` move is weighted by `weight_fn(patch)`; the `Advance` move
/// (when present) is weighted by `advance_weight`. The move is drawn from a
/// `std::discrete_distribution` over those weights.
///
/// Preconditions: `state` must not be terminal, `advance_weight > 0`, and at
/// least one resulting weight must be positive.
[[nodiscard]] Move biased_random_move(const SimplifiedGameState& state, const GameSetup& setup,
                                      std::mt19937& rng,
                                      const std::function<double(const PatchData&)>& weight_fn,
                                      double advance_weight);

}  // namespace patchwork

#endif  // PATCHWORK_BIASED_RANDOM_AGENT_HPP
