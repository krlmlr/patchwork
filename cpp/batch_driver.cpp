#include <cstdint>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "game_loop.hpp"
#include "game_setups.hpp"
#include "seed_util.hpp"
#include "setup_range.hpp"
#include "terminal_and_scoring.hpp"

namespace {

void usage(const char* prog) {
    std::cerr << "Usage: " << prog
              << " --setups <spec> --games <N> --master-seed <n> [--output <file>] [--full]\n"
              << "  --setups       comma-separated ids and inclusive ranges, e.g. 0,2,5-9\n"
              << "  --games        number of games per setup (positive integer)\n"
              << "  --master-seed  base seed; each game's seed is derived deterministically\n"
              << "  --output       write to a file instead of stdout\n"
              << "  --full         emit full per-move NDJSON instead of per-game summaries\n";
}

}  // namespace

int main(int argc, char** argv) {
    std::string setups_spec;
    long long games = 0;
    unsigned long long master_seed = 0;
    bool has_setups = false;
    bool has_games = false;
    bool has_master = false;
    bool full = false;
    std::string output_file;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--setups" && i + 1 < argc) {
            setups_spec = argv[++i];
            has_setups = true;
        } else if (arg == "--games" && i + 1 < argc) {
            try {
                games = std::stoll(argv[++i]);
                has_games = true;
            } catch (...) {
                usage(argv[0]);
                return 1;
            }
        } else if (arg == "--master-seed" && i + 1 < argc) {
            try {
                master_seed = std::stoull(argv[++i]);
                has_master = true;
            } catch (...) {
                usage(argv[0]);
                return 1;
            }
        } else if (arg == "--output" && i + 1 < argc) {
            output_file = argv[++i];
        } else if (arg == "--full") {
            full = true;
        } else {
            usage(argv[0]);
            return 1;
        }
    }

    if (!has_setups || !has_games || !has_master || games <= 0) {
        usage(argv[0]);
        return 1;
    }

    std::vector<int> setups;
    try {
        setups = patchwork::parse_setups(setups_spec);
    } catch (const std::exception& e) {
        std::cerr << "Error: invalid --setups: " << e.what() << '\n';
        usage(argv[0]);
        return 1;
    }

    std::ofstream fout;
    if (!output_file.empty()) {
        fout.open(output_file);
        if (!fout) {
            std::cerr << "Error: cannot open output file: " << output_file << '\n';
            return 1;
        }
    }
    std::ostream& out = output_file.empty() ? std::cout : fout;

    for (int setup_id : setups) {
        auto setup = patchwork::make_setup(setup_id);
        for (long long g = 0; g < games; ++g) {
            std::uint32_t seed = patchwork::derive_seed(master_seed, setup_id, static_cast<int>(g));
            if (full) {
                patchwork::play_game(setup, setup_id, static_cast<long long>(seed), &out);
            } else {
                patchwork::GameOutcome r =
                    patchwork::play_game(setup, setup_id, static_cast<long long>(seed), nullptr);
                out << R"({"event":"game_summary","setup_id":)" << setup_id << R"(,"seed":)" << seed
                    << R"(,"score_p0":)" << patchwork::score(r.state, 0) << R"(,"score_p1":)"
                    << patchwork::score(r.state, 1) << R"(,"winner":)" << patchwork::winner(r.state)
                    << R"(,"plies":)" << r.plies << "}\n";
            }
        }
    }

    return 0;
}
