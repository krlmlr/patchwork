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

### Circle stored as `std::array<uint8_t, 33>`

Patch IDs are 0–32 (fit in `uint8_t`). A fixed-size array avoids heap allocation and has `constexpr`-friendly semantics. The array index is the position around the circle (0 = start, 32 = last); the value is the patch ID at that position.

**Alternative considered:** `std::vector<uint8_t>` — dynamic allocation, no `constexpr`, no advantage for a fixed-size collection.

### Seeded RNG: `std::mt19937_64` + `std::shuffle`

`std::mt19937_64` is the standard, portable, reproducible PRNG. `std::shuffle` over the identity permutation `[0, 1, …, 32]` produces a uniformly random permutation. The seed is stored alongside the circle so that logs and analysis can always reproduce the exact setup.

**Alternative considered:** `std::default_random_engine` — implementation-defined; not reproducible across platforms or compiler versions.

### Canonical setups stored as YAML in `data/setups/`

Consistency with `data/patches.yaml`. YAML is human-readable, diff-friendly, and trivially parsed by R. Each file is named `setup-NNNNN.yaml` (zero-padded five-digit sequential ID) and contains:

```yaml
id: 1
seed: 3735928559
circle: [5, 22, 1, 17, 30, 8, 13, 2, 28, 19, 6, 25, 11, 0, 31, 16, 7, 24, 14, 3, 27, 20, 9, 29, 18, 12, 4, 26, 23, 10, 15, 21, 32]
```

The `id` field is informational; the file name is canonical.

**Alternative considered:** JSON — easier to parse in C++ but inconsistent with the rest of `data/`; the YAML format here is simple enough that a C++ hand-parser is trivial.

### C++ loader: hand-rolled line parser (no new dependency)

The YAML format for setups is deliberately minimal — one `id:` line, one `seed:` line, one `circle: [...]` line. A 20-line C++ parser handles this without adding a YAML library dependency. The parsed result is a `GameSetup` value.

**Alternative considered:** `nlohmann/json` with JSON files — would require a new Meson wrap and a format change; unjustified for this scope.

**Alternative considered:** `libyaml` or `yaml-cpp` — heavyweight; the format doesn't require it.

### NDJSON log record: single-line JSON emitted to `std::ostream`

The logging pipeline (NDJSON → DuckDB) is introduced in a later phase but should be designed for from day one. `GameSetup::to_ndjson(std::ostream&)` emits one JSON line: `{"type":"setup","id":1,"seed":3735928559,"circle":[…]}`. This integrates naturally with the planned logging pipeline.

**Alternative considered:** Defer logging entirely — the log helper is trivial to add now and avoids a second pass over this struct later.

### Initial batch: 10 canonical setups committed to `data/setups/`

Ten setups are enough for unit tests and early analysis without bloating the repository. More can be generated on demand with `mise run codegen:setups`. The batch is R-generated with seeds `[1, 2, …, 10]` for simplicity and reproducibility.

## Risks / Trade-offs

- **Hand-rolled YAML parser brittleness** → Mitigation: format is locked to the R generator's output; a round-trip test in C++ (generate via R, load in C++) catches any mismatch.
- **Sequential seeds for canonical setups** → Mitigation: seeds are stored in each file, so analysis can always identify which permutation is being studied; monotone seeds are not a security concern for a board game.
- **`GameSetup` not in `GameState` increases function-signature complexity** → Mitigation: the next-phase legal move functions will consistently take `(const GameSetup&, GameState&)` — documented as the project's calling convention.
