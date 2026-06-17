## Context

The project's Foundation phase established `GameState` (two `PlayerState` instances + shared 64-bit field). A known gap was explicitly deferred: the initial circular arrangement of the 33 patches is not stored anywhere. R already manages the patch catalog; this change extends that pattern to game setups. The first consumer of `GameSetup` will be the legal-move generator in the next phase (Piece Placement / Simplified Rules).

## Goals / Non-Goals

**Goals:**

- Define `GameSetup` as a lightweight, immutable value type suitable for sharing across MCTS tree nodes without copying
- Establish R as the authority for generating and versioning canonical setups
- Keep C++ simple — no seed tracking, no RNG, no file I/O
- Ensure the new type is fully unit-tested

**Non-Goals:**

- Embedding the circle arrangement inside `GameState` (would bloat the hot copy-path for MCTS)
- Game logic or legal move generation (next phase)
- A full YAML parsing library in C++ (overkill for 33 integers)
- GUI or visualisation of the circle

## Decisions

### `GameSetup` is separate from `GameState` (not embedded)

For MCTS, `GameState` is copied millions of times per second. Adding 33 bytes for the circle arrangement to every copy would increase the hot struct from 40 bytes to 73 bytes — a 83 % increase that degrades cache performance with no benefit, because the circle is *immutable* across the whole search tree. `GameSetup` lives outside `GameState` and is passed by const-reference to any function that needs it.

**Alternative considered:** Store the circle in `GameState` — simpler API but unacceptable memory cost at MCTS scale.

### Circle stored as `uint8_t` integer IDs; constructed from a string

Patch IDs are 0–32 (fit in `uint8_t`). `GameSetup` stores a `std::array<uint8_t, 33>` internally to keep the hot lookup path index-based and allocation-free. The constructor accepts a 33-char `std::string_view` of single-char patch names and converts each character to its integer ID via the patch catalog — so callers write `GameSetup{"eV1q…"}` without needing to know the ID mapping.

**Alternative considered:** Store characters directly (`std::array<char, 33>`) — readable but requires a catalog lookup on every access to obtain the ID for game logic; shifts the per-use cost from construction to usage.

### Seeds are an R-only concern; not exposed in C++

`codegen/generate_setups.R` uses a named constant `N_SETUPS <- 100L` and generates each permutation deterministically from `set.seed(i); sample(33)`. The seeds are internal bookkeeping for the R script. Once the header is generated, C++ has no use for them — the permutations are already fixed as `constexpr` string literals. Omitting seeds from `GameSetup` and from the generated header keeps the type simple and the NDJSON log compact.

**Alternative considered:** Store the seed in `GameSetup` alongside the circle — would allow C++ to reproduce any setup independently, but that reproducing is already done by R and the result is committed; adding seed storage to C++ would be redundant and wasted bytes in the struct.

### Canonical setups as `constexpr` string literals in `cpp/generated/game_setups.hpp`

R reads `data/patches.yaml` to obtain the ordered single-char patch names, generates 100 random permutations via `sample()`, and writes `cpp/generated/game_setups.hpp`. The header exposes a `constexpr std::array<std::string_view, kNumGameSetups>` named `kGameSetups` where `kNumGameSetups` is a named constant set to `100`. Generating additional setups later uses a larger `N_SETUPS` value in R but leaves the first 100 entries bit-for-bit identical. Including this file costs zero runtime I/O and the entire setup table is available as a compile-time constant.

```cpp
// cpp/generated/game_setups.hpp
namespace patchwork {
inline constexpr std::size_t kNumGameSetups = 100;
constexpr std::array<std::string_view, kNumGameSetups> kGameSetups = {{
    "eV1q...",   // 33 chars
    ...
}};
}
```

**Alternative considered:** YAML files in `data/setups/` — file-per-setup approach is diff-friendly and incremental, but requires a C++ YAML parser at runtime and makes setups unavailable without disk access (a problem for MCTS, tests, and constexpr evaluation).

### No runtime I/O: setups are `constexpr` string literals

Because setups are embedded in `cpp/generated/game_setups.hpp`, no file loading is required at runtime. `GameSetup` has no `load()` method. Any function that needs a canonical setup indexes into `kGameSetups` directly. This eliminates a whole class of failure modes (missing files, path configuration, file format drift) and makes setup access zero-cost.

**Alternative considered:** A C++ YAML parser (hand-rolled or via `yaml-cpp`) — adds complexity and a runtime dependency for data that never changes between compilations.

### NDJSON log record: single-line JSON with circle as a string

The logging pipeline (NDJSON → DuckDB) is introduced in a later phase but should be designed for from day one. `GameSetup::to_ndjson(std::ostream&)` emits one JSON line: `{"type":"setup","circle":"<33chars>"}`. The circle is a string of single-char patch names, making log lines short and directly grep-able by patch name character. Seeds are not included — they are R metadata, not game state.

**Alternative considered:** Include the seed in the NDJSON record for traceability — unnecessary since canonical setup index (position in `kGameSetups`) is the stable identifier, and seeds are available in the R script.

### Initial batch: 100 canonical setups in `cpp/generated/game_setups.hpp`

100 setups are sufficient for unit tests, early game-tree analysis, and RL training warm-up without significantly increasing build times or binary size (`constexpr` data is essentially free). The batch is R-generated with seeds `[1, 2, …, 100]` for simplicity and reproducibility. More can be re-generated on demand with `mise run codegen:setups`.

## Risks / Trade-offs

- **Generated header size** → Mitigation: 100 × 33 chars of `constexpr` data is ~3 KB; negligible impact on compile time or binary size.
- **First-100 stability depends on R PRNG reproducibility** → Mitigation: `set.seed(i); sample(33)` is deterministic in any R version; the generated header is committed and only updated by an intentional re-run of `codegen/generate_setups.R`.
- **`GameSetup` not in `GameState` increases function-signature complexity** → Mitigation: the next-phase legal move functions will consistently take `(const GameSetup&, GameState&)` — documented as the project's calling convention.
