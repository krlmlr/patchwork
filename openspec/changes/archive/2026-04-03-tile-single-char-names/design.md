## Context

The Patchwork engine identifies patches by integer ID (1–33). The ID is the only identifier present in game logs and any future debug output. While unambiguous, numeric IDs require a look-up to understand what shape is being referred to. A single memorable character per patch — chosen to echo its visual shape — makes logs immediately human-readable without any look-up.

The patch catalog lives in `data/patches.yaml` and is the single source of truth. A generated C++ header (`src/generated/patches.hpp`) is produced by `codegen/generate_patches.R` and committed. Adding the name follows the same pipeline: edits in YAML → edit in the codegen script → regenerate and commit the header.

This change also takes the opportunity to sort and renumber patches in a logical order (size → cost → income) and to normalise each `shape` value to its canonical form.

## Goals / Non-Goals

**Goals:**

- Every patch has a unique single ASCII letter or digit as its `name`
- `data/patches.yaml` is the source of truth for all 33 names
- Patches in `data/patches.yaml` are ordered by cell count → button cost → income (descending) and IDs are renumbered 1–33 accordingly
- All `shape` values in `data/patches.yaml` are stored in canonical form (lexicographically minimal across all rotations and reflections)
- The generated `PatchData` struct exposes `char name` at compile time
- Names are self-explanatory where the shape matches a recognisable letter/digit; where ambiguity exists the choice is consistent and documented in the proposal

**Non-Goals:**

- Multi-character names or full English names — single char only
- Changing game logic, state encoding, or move generation — this is a display/logging layer only

## Decisions

### Canonical shape normalisation

Patchwork tiles are physically identical under any rotation or reflection: both orientations of the L/J-tromino are the same piece in the game. The `shape` field therefore stores the **canonical form** — the lexicographically smallest grid string across all 8 orientations (4 rotations × 2 reflections).

Three pairs of patches share the same canonical shape but have different costs and/or income values; they are stored as separate entries with distinct `name` characters. Within each pair the cheaper tile takes the lowercase letter and the costlier tile takes the uppercase (or a visually related character).

### Ordering and renumbering

Ordering by (cell count, button cost, −income) groups logically similar tiles together and makes the catalog self-documenting. IDs are renumbered 1–33 to match, so the numeric ID carries size/cost information implicitly.

### Name selection strategy

**I-polyominoes use the digit matching their cell count:** 2-cell domino → `2`, 3-cell straight tromino → `3`, 4-cell straight tetromino → `4`, 5-cell straight pentomino → `5`. This makes line-pieces immediately recognisable and reserves digits for cases where cell count is the defining feature.

**Well-known polyomino families use their standard letter:** `t`/`T` (T-tetromino / T-pentomino), `s`/`S` (S/Z-tetromino cheap / costly), `L`/`l` (L-tetromino high-income / low-income), `o` (O-tetromino), `u`/`U` (U-pentomino / U-hexomino), `z`/`Z` (N-pentomino / Z-hexomino family), `N` (N/Z-hexomino diagonal), `w` (W-pentomino), `x`/`X` (X-pentomino small plus / X-heptomino large plus), `H` (H-heptomino), `p` (P-pentomino), `y` (Y-pentomino), `J`/`j` (J-pentomino / J-tromino mirror).

**Remaining patches use shape-inspired characters:**
- `v` — the cheaper L/J-tromino seen as a bent line resembling v
- `m` — arch/bridge hexomino (two humps match the two bumps of m)
- `1` — tall-stick hexomino (looks like the numeral 1 with a serif base)
- `A` — asymmetric T-hexomino (offset cross; no standard letter matches precisely)
- `k` — asymmetric zigzag hexomino (unique shape with no standard name; `k` is unambiguous)
- `e`/`d`/`q` — hexominoes with elongated or step shapes
- `O` — serif I-beam octomino (uppercase O evokes the closed, blocky silhouette of the piece)

**Upper/lower-case pairing is used deliberately** to distinguish same-canonical-shape pairs and to pair polyomino family members by size:
- `s`/`S`: S/Z-tetromino cheap vs costly
- `l`/`L`: L/J-tetromino low-income vs high-income
- `j`/`J`: J-tromino mirror vs J-pentomino
- `u`/`U`: U-pentomino vs U-hexomino
- `z`/`Z`: N-pentomino (small, N-family) vs Z-hexomino (larger, elongated S/Z)
- `t`/`T`: T-tetromino vs T-pentomino
- `x`/`X`: X-pentomino (small plus) vs X-heptomino (large plus)
- `o`/`O`: O-tetromino (small 2×2 square) vs I-octomino (largest piece)

**Alternative considered:** Use only lowercase letters — simpler, but provides fewer intuitive size-pairing signals.

**Alternative considered:** Use a fixed hash or abbreviation of `(buttons, time, income)` — fully unique and mnemonic, but multi-character and not shape-related.

### Same-canonical-shape pairs: different costs

Two physical pairs of Patchwork patches share the same canonical shape:

- **New IDs 2 & 4** (old 1 & 6): L/J-tromino, buttons 1 vs 3. Names `v` and `j`.
- **New IDs 7 & 11** (old 23 & 19): S/Z-tetromino, buttons 3 vs 7. Names `s` and `S`. In logging, `s` (lower) is the cheaper piece and `S` (upper) is the costlier one.
- **New IDs 8 & 9** (old 24 & 31): L/J-tetromino, buttons 4 / 4 (same cost), income 2 vs 1. Names `L` and `l`.

### YAML field order

The `name` field is placed immediately after `id` so that a human reading the YAML sees the visual label before the numeric attributes.

### Spec update

`openspec/specs/patch-catalog/spec.md` must be updated to document the `name` field as required, to add a uniqueness scenario, to add a canonical-shape scenario, and to add an ordering scenario.

## Risks / Trade-offs

- **33 unique characters from [A-Za-z0-9] (62 available):** No shortage; the full set fits easily with room to spare.
- **Same shape, different names (`s` vs `S`, `l` vs `L`, `v` vs `j`):** Slightly surprising, but documented. Alternatives (same name for both, or a digit suffix) are worse: same name breaks uniqueness; digit suffix violates single-char constraint.
- **Case sensitivity in logging:** Implementers must treat `name` as case-sensitive. This is standard for `char` in C++.
- **ID renumbering:** Any external reference to the old numeric IDs (documentation, tests, saved games) must be updated. Since the project is early-stage and IDs are only used internally, the impact is limited.
- **Future shape additions:** If a patch is ever added (unlikely — the game has exactly 33), one of the ~29 unused characters can be assigned.
