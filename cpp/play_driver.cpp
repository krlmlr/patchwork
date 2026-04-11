#include <cstdlib>
#include <fstream>
#include <iostream>
#include <random>
#include <string>

#include "game_logger.hpp"
#include "game_setups.hpp"
#include "move_application.hpp"
#include "move_generation.hpp"
#include "random_agent.hpp"
#include "simplified_game_state.hpp"
#include "terminal_and_scoring.hpp"

namespace {

void usage(const char* prog) {
    std::cerr << "Usage: " << prog << " --seed <n> --setup <id> [--output <file>]\n";
}

}  // namespace

int main(int argc, char** argv) {
    long long seed = 0;
    int setup_id = 0;
    bool has_seed = false;
    bool has_setup = false;
    std::string output_file;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--seed" && i + 1 < argc) {
            try {
                seed = std::stoll(argv[++i]);
                has_seed = true;
            } catch (...) {
                usage(argv[0]);
                return 1;
            }
        } else if (arg == "--setup" && i + 1 < argc) {
            try {
                setup_id = std::stoi(argv[++i]);
                has_setup = true;
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

    if (!has_seed || !has_setup) {
        usage(argv[0]);
        return 1;
    }

    auto setup = patchwork::make_setup(setup_id);
    std::mt19937 rng(static_cast<unsigned>(seed));
    patchwork::SimplifiedGameState state;

    std::ofstream fout;
    if (!output_file.empty()) {
        fout.open(output_file);
        if (!fout) {
            std::cerr << "Error: cannot open output file: " << output_file << '\n';
            return 1;
        }
    }
    std::ostream& out = output_file.empty() ? std::cout : fout;

    patchwork::log_game_start(out, seed, setup_id, state, setup);

    int ply = 0;
    while (!patchwork::is_terminal(state)) {
        int player = state.active_player();
        patchwork::Move mv = patchwork::random_move(state, setup, rng);
        state = patchwork::apply_move(state, mv, setup);
        patchwork::log_move(out, ply, player, mv, state, setup);
        ++ply;
    }

    patchwork::log_game_end(out, state);
    return 0;
}
