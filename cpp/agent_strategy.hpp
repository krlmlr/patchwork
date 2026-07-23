#ifndef PATCHWORK_AGENT_STRATEGY_HPP
#define PATCHWORK_AGENT_STRATEGY_HPP

#include <functional>
#include <optional>
#include <string_view>

#include "generated/patches.hpp"

namespace patchwork {

/// The agent selection strategies supported by the play driver and TUI.
enum class AgentStrategy {
    Random,         // uniform random selection (the existing baseline agent)
    Cheap,          // prefer patches with lower button cost
    Income,         // prefer patches with higher button income
    IncomePerTime,  // prefer patches with better income-to-time ratio
};

/// Canonical string name of a strategy, used for logging and CLI messages.
[[nodiscard]] inline std::string_view strategy_name(AgentStrategy strategy) {
    switch (strategy) {
        case AgentStrategy::Random:
            return "random";
        case AgentStrategy::Cheap:
            return "cheap";
        case AgentStrategy::Income:
            return "income";
        case AgentStrategy::IncomePerTime:
            return "income-per-time";
    }
    return "random";  // unreachable; silences -Wreturn-type
}

/// Parse a strategy name; returns `std::nullopt` for an unrecognized name.
[[nodiscard]] inline std::optional<AgentStrategy> parse_strategy(std::string_view name) {
    if (name == "random") return AgentStrategy::Random;
    if (name == "cheap") return AgentStrategy::Cheap;
    if (name == "income") return AgentStrategy::Income;
    if (name == "income-per-time") return AgentStrategy::IncomePerTime;
    return std::nullopt;
}

/// Return the weight function for a biased strategy, or `nullptr` for `Random`.
///
/// Declared here but defined in `biased_random_agent.cpp` so that this header
/// stays includable without pulling in the agent implementation.
[[nodiscard]] std::function<double(const PatchData&)> make_weight_fn(AgentStrategy strategy);

}  // namespace patchwork

#endif  // PATCHWORK_AGENT_STRATEGY_HPP
