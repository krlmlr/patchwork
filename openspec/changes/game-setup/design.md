## Context

The project's Foundation phase established `GameState` (two `PlayerState` instances + shared 64-bit field). A known gap was explicitly deferred: the initial circular arrangement of the 33 patches is not stored anywhere. R already manages the patch catalog; this change extends that pattern to game setups. The first consumer of `GameSetup` will be the legal-move generator in the next phase (Piece Placement / Simplified Rules).

## Goals / Non-Goals

**Goals:**

- Define `GameSetup` as a lightweight, immutable value type suitable for sharing across MCTS tree nodes without copying
- Establish R as the authority for generating and versioning canonical setups
- Keep C++ parsing simple — no new heavy dependencies
- Make setups reproducible: given a seed, the same circle is always produced
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

### Circle stored as a 33-char string of patch names

Patch names are single ASCII characters (`[A-Za-z0-9]`, defined in `data/patches.yaml`). Encoding the circle as a `std::array<char, 33>` (or a `std::string_view` into the generated table) preserves the compactness of the array approach while making the circle human-readable at a glance — `"eV1q…"` is instantly scannable in logs and debugger output without a catalog lookup. It also aligns with the `char name` field already present in `PatchData`.

**Alternative considered:** `std::array<uint8_t, 33>` of patch IDs — compact but opaque; requires a catalog lookup to interpret any element.

### Seeded RNG: `std::mt19937_64` + `std::shuffle`

`std::mt19937_64` is the standard, portable, reproducible PRNG. `std::shuffle` over the identity permutation `[0, 1, …, 32]` produces a uniformly random permutation. The seed is stored alongside the circle so that logs and analysis can always reproduce the exact setup.

**Alternative considered:** `std::default_random_engine` — implementation-defined; not reproducible across platforms or compiler versions.

### Canonical setups as `constexpr` string literals in `src/generated/game_setups.hpp`

R reads `data/patches.yaml` to obtain the ordered single-char patch names, generates 100 random permutations via `sample()`, and writes `src/generated/game_setups.hpp`. The header exposes a `constexpr std::array` of 100 `GameSetupEntry` values — each pairing a 33-char `std::string_view` with its seed. Including this file costs zero runtime I/O and the entire setup table is available as a compile-time constant.

```cpp
// src/generated/game_setups.hpp
namespace patchwork {
struct GameSetupEntry { std::string_view circle; uint64_t seed; };
constexpr std::array<GameSetupEntry, 100> kGameSetups = {{
    {"eV1q...", 1},   // 33 chars
    ...
}};
}
```

**Alternative considered:** YAML files in `data/setups/` — file-per-setup approach is diff-friendly and incremental, but requires a C++ YAML parser at runtime and makes setups unavailable without disk access (a problem for MCTS, tests, and constexpr evaluation).

### No runtime I/O: setups are `constexpr` string literals

Because setups are embedded in `src/generated/game_setups.hpp`, no file loading is required at runtime. `GameSetup` has no `load()` method. Any function that needs a canonical setup indexes into `kGameSetups` directly. This eliminates a whole class of failure modes (missing files, path configuration, file format drift) and makes setup access zero-cost.

**Alternative considered:** A C++ YAML parser (hand-rolled or via `yaml-cpp`) — adds complexity and a runtime dependency for data that never changes between compilations.

### NDJSON log record: single-line JSON with circle as a string

The logging pipeline (NDJSON → DuckDB) is introduced in a later phase but should be designed for from day one. `GameSetup::to_ndjson(std::ostream&)` emits one JSON line: `{"type":"setup","seed":<seed>,"circle":"<33chars>"}`. The circle is now a string rather than a JSON array, making log lines shorter and directly grep-able by patch name character. This integrates naturally with the planned logging pipeline.

**Alternative considered:** Emit the circle as a JSON array of integers (patch IDs) — bulkier, less human-readable, inconsistent with the char-based representation in the rest of the codebase.

### Initial batch: 100 canonical setups in `src/generated/game_setups.hpp`

100 setups are sufficient for unit tests, early game-tree analysis, and RL training warm-up without significantly increasing build times or binary size (`constexpr` data is essentially free). The batch is R-generated with seeds `[1, 2, …, 100]` for simplicity and reproducibility. More can be re-generated on demand with `mise run codegen:setups`.

## Risks / Trade-offs

- **Generated header size** → Mitigation: 100 × 33 chars of `constexpr` data is ~3 KB; negligible impact on compile time or binary size.
- **Sequential seeds for canonical setups** → Mitigation: seeds are stored alongside each entry, so analysis can always identify which permutation is being studied; monotone seeds are not a security concern for a board game.
- **`GameSetup` not in `GameState` increases function-signature complexity** → Mitigation: the next-phase legal move functions will consistently take `(const GameSetup&, GameState&)` — documented as the project's calling convention.
