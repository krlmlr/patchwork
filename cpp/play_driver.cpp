#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

#include "game_loop.hpp"
#include "game_setups.hpp"

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

    std::ofstream fout;
    if (!output_file.empty()) {
        fout.open(output_file);
        if (!fout) {
            std::cerr << "Error: cannot open output file: " << output_file << '\n';
            return 1;
        }
    }
    std::ostream& out = output_file.empty() ? std::cout : fout;

    patchwork::play_game(setup, setup_id, seed, &out);
    return 0;
}
