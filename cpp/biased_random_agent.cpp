#include "biased_random_agent.hpp"

#include <algorithm>
#include <cassert>
#include <vector>

#include "agent_strategy.hpp"
#include "move_generation.hpp"

namespace patchwork {

double weight_cheap(const PatchData& patch) { return 1.0 / (patch.buttons + 1); }

double weight_income(const PatchData& patch) { return static_cast<double>(patch.income + 1); }

double weight_income_per_time(const PatchData& patch) { return (patch.income + 0.5) / patch.time; }

Move biased_random_move(const SimplifiedGameState& state, const GameSetup& setup, std::mt19937& rng,
                        const std::function<double(const PatchData&)>& weight_fn,
                        double advance_weight) {
    assert(advance_weight > 0);
    auto moves = legal_moves(state, setup);
    assert(!moves.empty());

    std::vector<double> weights;
    weights.reserve(moves.size());
    for (const auto& move : moves) {
        if (std::holds_alternative<BuyPatch>(move)) {
            int idx = std::get<BuyPatch>(move).patch_index;
            weights.push_back(weight_fn(kPatches[static_cast<std::size_t>(idx)]));
        } else {
            weights.push_back(advance_weight);
        }
    }

    // The distribution is degenerate if every weight is zero.
    assert(std::any_of(weights.begin(), weights.end(), [](double w) { return w > 0.0; }));

    std::discrete_distribution<std::size_t> dist(weights.begin(), weights.end());
    return moves[dist(rng)];
}

std::function<double(const PatchData&)> make_weight_fn(AgentStrategy strategy) {
    switch (strategy) {
        case AgentStrategy::Random:
            return nullptr;
        case AgentStrategy::Cheap:
            return weight_cheap;
        case AgentStrategy::Income:
            return weight_income;
        case AgentStrategy::IncomePerTime:
            return weight_income_per_time;
    }
    return nullptr;  // unreachable
}

}  // namespace patchwork
