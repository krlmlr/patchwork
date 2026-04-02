## Context

The Patchwork engine identifies patches by integer ID (1–33). The ID is the only identifier present in game logs and any future debug output. While unambiguous, numeric IDs require a look-up to understand what shape is being referred to. A single memorable character per patch — chosen to echo its visual shape — makes logs immediately human-readable without any look-up.

The patch catalog lives in `data/patches.yaml` and is the single source of truth. A generated C++ header (`src/generated/patches.hpp`) is produced by `codegen/generate_patches.R` and committed. Adding the name follows the same pipeline: one edit in YAML → one edit in the codegen script → regenerate and commit the header.

## Goals / Non-Goals

**Goals:**

- Every patch has a unique single ASCII letter or digit as its `name`
- `data/patches.yaml` is the source of truth for all 33 names
- The generated `PatchData` struct exposes `char name` at compile time
- Names are self-explanatory where the shape matches a recognisable letter/digit; where ambiguity exists the choice is consistent and documented in the proposal

**Non-Goals:**

- Multi-character names or full English names — single char only
- Rotation/flip-invariant naming (two patches with the same shape at different orientations may get different names)
- Changing game logic, state encoding, or move generation — this is a display/logging layer only

## Decisions

### Name selection strategy

Straight-line patches (I-polyominoes) use the digit matching their length: 2-cell domino → `2`, 3-cell → `3`, 4-cell → `4`, 5-cell → `5`. This makes their size immediately obvious and reserves the digit namespace for shapes where the cell count is the defining feature.

For non-linear shapes, standard polyomino letter names are used where they unambiguously match: `x` (X-pentomino), `w` (W-pentomino), `H` (H-heptomino), `O` (O-tetromino), `P` (P-pentomino), `F` (F-pentomino), `U`/`u` (U-hexomino/pentomino), `L`/`l`/`J`/`j` (L- and J-triominoes/tetrominoes/pentominoes), `S`/`s`/`Z`/`z`/`N`/`n` (S/Z/N families), `T`/`t` (T-tetromino and T-hexomino), `Y`/`y` (Y-hexomino and Y-pentomino variant), `X`/`x` (plus-shapes at two sizes).

Upper/lower-case pairs are used deliberately to distinguish shape families by size or orientation:
- `l`/`L`: L-triomino vs L-tetromino; `j`/`J`: J-triomino vs J-pentomino
- `t`/`T`: T-tetromino vs T-hexomino; `x`/`X`: X-pentomino vs X-heptomino
- `s`/`S`: two S-tetrominoes with identical shape but different costs; `z`/`Z`: Z-family hexominoes
- `n`/`N`: N-family hexominoes; `y`/`Y`: Y-family shapes; `u`/`U`: U-family shapes

Remaining patches use creative but defensible choices: `m` for the arch/bridge hexomino (two bumps = two humps of m), `q` for the hockey-stick hexomino, `r` for the L-corner tetromino, `b` for the T-pentomino (stem + base), `1` for the tall-stick-with-base hexomino (numeral 1 with a serif), `I` for the seriffed octomino.

**Alternative considered:** Use only lowercase letters — simpler, but runs out of distinct letters for 33 patches and loses the natural size-pairing signal.

**Alternative considered:** Use a fixed hash or abbreviation of `(buttons, time, income)` — fully unique and mnemonic, but multi-character and not shape-related.

### ID 19 and ID 23: identical shapes, different costs

Both patches have the shape `.X/XX/X.` (the S-tetromino). The Patchwork game includes two physically distinct patches with the same shape but different button/time/income values. Their names `S` and `s` convey the shared shape while distinguishing them. In logging, `S` (upper) refers to the costlier piece (7 buttons) and `s` (lower) to the cheaper one (3 buttons).

### YAML field order

The `name` field is placed immediately after `id` so that a human reading the YAML sees the visual label before the numeric attributes.

### Spec update

`openspec/specs/patch-catalog/spec.md` must be updated to document the `name` field as a required attribute and to add a scenario ensuring all 33 names are unique and each is a single ASCII letter or digit.

## Risks / Trade-offs

- **33 unique characters from [A-Za-z0-9] (62 available):** No shortage; the full set fits easily with room to spare.
- **Same shape, different names (`S` vs `s`):** Slightly surprising, but documented. Alternatives (same name for both, or a digit suffix) are worse: same name breaks uniqueness; digit suffix violates single-char constraint.
- **Case sensitivity in logging:** Implementers must treat `name` as case-sensitive. This is standard for `char` in C++.
- **Future shape additions:** If a patch is ever added (unlikely — the game has exactly 33), one of the ~29 unused characters can be assigned.
