#include <cstdlib>
#include <fstream>
#include <iostream>
#include <optional>
#include <random>
#include <string>

#include "agent.hpp"
#include "agent_strategy.hpp"
#include "game_logger.hpp"
#include "game_loop.hpp"
#include "game_setups.hpp"
#include "generated/game_setups.hpp"
#include "move_application.hpp"
#include "move_generation.hpp"
#include "simplified_game_state.hpp"
#include "terminal_and_scoring.hpp"

namespace {

void usage(const char* prog) {
    std::cerr
        << "Usage: " << prog
        << " [--setup <id>] [--agent1 <strategy>] [--agent2 <strategy>]"
           " [--seed1 <n>] [--seed2 <n>] [--advance-weight <w>] [--seed <n>] [--output <file>]\n"
           "\n"
           "All arguments are optional. Defaults: --setup 0, --agent1 random, --agent2 random,\n"
           "--seed1 42, --seed2 42, --advance-weight 1.0, output to stdout.\n"
           "Strategies: random, cheap, income, income-per-time.\n"
           "--advance-weight must be strictly positive; it has no effect for the random "
           "strategy.\n"
           "--seed <n> runs a uniform random self-play game with a single shared RNG stream\n"
           "(the form the batch driver records for replay); it is mutually exclusive with the\n"
           "per-player --seed1/--seed2/--agent1/--agent2/--advance-weight flags.\n";
}

}  // namespace

int main(int argc, char** argv) {
    int setup_id = 0;
    long long seed1 = 42;
    long long seed2 = 42;
    long long single_seed = 0;
    bool has_single_seed = false;
    bool has_per_player = false;  // any per-player flag explicitly supplied
    patchwork::AgentStrategy agent1 = patchwork::AgentStrategy::Random;
    patchwork::AgentStrategy agent2 = patchwork::AgentStrategy::Random;
    double advance_weight = 1.0;
    std::string output_file;

    auto parse_agent = [&](const char* value, patchwork::AgentStrategy& out) -> bool {
        std::optional<patchwork::AgentStrategy> s = patchwork::parse_strategy(value);
        if (!s) {
            std::cerr << "Error: unknown strategy '" << value
                      << "'. Valid strategies: random, cheap, income, income-per-time.\n";
            return false;
        }
        out = *s;
        return true;
    };

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--setup" && i + 1 < argc) {
            try {
                setup_id = std::stoi(argv[++i]);
            } catch (...) {
                usage(argv[0]);
                return 1;
            }
        } else if (arg == "--seed" && i + 1 < argc) {
            try {
                single_seed = std::stoll(argv[++i]);
                has_single_seed = true;
            } catch (...) {
                usage(argv[0]);
                return 1;
            }
        } else if (arg == "--seed1" && i + 1 < argc) {
            try {
                seed1 = std::stoll(argv[++i]);
                has_per_player = true;
            } catch (...) {
                usage(argv[0]);
                return 1;
            }
        } else if (arg == "--seed2" && i + 1 < argc) {
            try {
                seed2 = std::stoll(argv[++i]);
                has_per_player = true;
            } catch (...) {
                usage(argv[0]);
                return 1;
            }
        } else if (arg == "--agent1" && i + 1 < argc) {
            if (!parse_agent(argv[++i], agent1)) return 1;
            has_per_player = true;
        } else if (arg == "--agent2" && i + 1 < argc) {
            if (!parse_agent(argv[++i], agent2)) return 1;
            has_per_player = true;
        } else if (arg == "--advance-weight" && i + 1 < argc) {
            try {
                advance_weight = std::stod(argv[++i]);
                has_per_player = true;
            } catch (...) {
                usage(argv[0]);
                return 1;
            }
        } else if (arg == "--output" && i + 1 < argc) {
            output_file = argv[++i];
        } else {
            usage(argv[0]);
            return 1;
        }
    }

    // The single-seed uniform mode and the per-player mode are mutually exclusive.
    if (has_single_seed && has_per_player) {
        std::cerr << "Error: --seed cannot be combined with the per-player flags "
                     "(--seed1/--seed2/--agent1/--agent2/--advance-weight).\n";
        usage(argv[0]);
        return 1;
    }

    // Validate setup range (make_setup wraps, so an out-of-range id must be
    // rejected explicitly here).
    if (setup_id < 0 || setup_id >= static_cast<int>(patchwork::kNumGameSetups)) {
        std::cerr << "Error: --setup must be in the range [0, " << patchwork::kNumGameSetups - 1
                  << "].\n";
        usage(argv[0]);
        return 1;
    }

    // Advance weight must be strictly positive.
    if (!(advance_weight > 0.0)) {
        std::cerr << "Error: --advance-weight must be strictly positive.\n";
        usage(argv[0]);
        return 1;
    }

    auto setup = patchwork::make_setup(setup_id);

    std::ofstream fout;
    if (!output_file.empty()) {
        fout.open(output_file);
        if (!fout) {
            std::cerr << "Error: cannot open output file: " << output_file << '\n';
            return 1;
        }
    }
    std::ostream& out = output_file.empty() ? std::cout : fout;

    // Uniform single-shared-stream mode: identical to the game the batch driver
    // records, so a recorded (setup, seed) can be replayed exactly.
    if (has_single_seed) {
        patchwork::play_game(setup, setup_id, single_seed, &out);
        return 0;
    }

    // Per-player mode: two independent RNG streams and per-player strategies.
    std::mt19937 rng_p0(static_cast<unsigned>(seed1));
    std::mt19937 rng_p1(static_cast<unsigned>(seed2));
    patchwork::SimplifiedGameState state;

    patchwork::log_game_start(out, setup_id, state, setup, patchwork::strategy_name(agent1),
                              patchwork::strategy_name(agent2), seed1, seed2, advance_weight);

    int ply = 0;
    while (!patchwork::is_terminal(state)) {
        int player = state.active_player();
        std::mt19937& rng = (player == 0) ? rng_p0 : rng_p1;
        patchwork::AgentStrategy strategy = (player == 0) ? agent1 : agent2;
        patchwork::Move mv = patchwork::select_move(state, setup, rng, strategy, advance_weight);
        state = patchwork::apply_move(state, mv, setup);
        patchwork::log_move(out, ply, player, mv, state, setup);
        ++ply;
    }

    patchwork::log_game_end(out, state);
    return 0;
}
