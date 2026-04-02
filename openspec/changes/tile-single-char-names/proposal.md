## Why

Patches are currently identified only by their integer ID (1–33) in logs and debug output. Reading a log line like `player placed patch 17` requires looking up the catalog to recall what shape that is. A single memorable character derived from the patch's visual shape makes logs instantly scannable: `player placed H` is self-explanatory.

## What Changes

- Add a `name` field (single ASCII letter or digit) to every entry in **`data/patches.yaml`** — the single source of truth for patch data
- Update **`codegen/generate_patches.R`** to read the new field and emit it into the generated C++ header
- Update **`src/generated/patches.hpp`** (regenerated from the new data) to include a `char name` field in `PatchData`
- Update **`openspec/specs/patch-catalog/spec.md`** to require the `name` field

## Capabilities

### New Capabilities

- `patch_name`: every `PatchData` entry exposes a `char name` that identifies the patch by its shape at compile time

### Modified Capabilities

- `patch_catalog_yaml`: `data/patches.yaml` now requires a `name` field alongside `id`, `buttons`, `time`, `income`, and `shape`
- `patch_codegen`: `codegen/generate_patches.R` emits the `name` field into the generated struct
- `patch_header`: `src/generated/patches.hpp` exposes `char name` in `PatchData`

## Name Assignments

Names are chosen to resemble the patch's shape. Straight-line patches use digits matching their length; well-known polyomino shapes use their standard letter; ambiguous shapes are resolved creatively using upper/lower case to distinguish size variants.

| ID | Shape (ASCII) | Name | Rationale |
|----|--------------|------|-----------|
| 1  | `X. / XX`                    | `l` | L-triomino |
| 2  | `.X. / XXX / .X.`            | `x` | X-pentomino (plus/cross) |
| 3  | `.X. / .XX / XX. / .X.`      | `n` | N-like diagonal zigzag |
| 4  | `XXXXX`                       | `5` | 5-cell I-pentomino (line) |
| 5  | `.XX / .XX / XX.`             | `z` | Z-like staircase |
| 6  | `.X / XX`                     | `j` | J-triomino (mirror of `l`) |
| 7  | `...X / XXXX / X...`          | `N` | N-shaped hexomino |
| 8  | `.X / XX / .X`                | `t` | T-tetromino rotated |
| 9  | `X. / XX / XX`                | `P` | P-pentomino |
| 10 | `.X / XX / XX / X.`           | `Z` | S/Z hexomino (tall) |
| 11 | `..X / .XX / XX.`             | `w` | W-pentomino |
| 12 | `.XX. / XXXX`                 | `m` | arch/bridge shape (two bumps) |
| 13 | `.XXX / XX..`                 | `F` | F-pentomino variant |
| 14 | `XXXX / XX..`                 | `q` | hockey-stick hexomino |
| 15 | `XX / XX`                     | `O` | O-tetromino (2×2 square) |
| 16 | `.X.. / XXXX / .X..`          | `T` | T-hexomino (offset cross) |
| 17 | `X.X / XXX / X.X`             | `H` | H-heptomino |
| 18 | `X..X / XXXX`                 | `U` | U-hexomino (open top) |
| 19 | `.X / XX / X.`                | `S` | S-tetromino |
| 20 | `.X / .X / XX / .X`           | `y` | Y-like (stem with branch) |
| 21 | `XXXX`                        | `4` | 4-cell I-tetromino (line) |
| 22 | `.X. / .X. / .X. / XXX`       | `1` | tall stick with base (numeral 1) |
| 23 | `.X / XX / X.`                | `s` | S-tetromino (same shape as 19, different cost) |
| 24 | `.X / .X / XX`                | `L` | L-tetromino |
| 25 | `.X / .X / .X / XX`           | `J` | J-pentomino |
| 26 | `.XX. / XXXX / .XX.`          | `I` | I-octomino with serifs (capital I) |
| 27 | `..X.. / XXXXX / ..X..`       | `X` | X-heptomino (large plus) |
| 28 | `.X. / XXX / X.X`             | `Y` | Y-hexomino |
| 29 | `X.X / XXX`                   | `u` | U-pentomino |
| 30 | `XXX`                         | `3` | 3-cell line |
| 31 | `..X / XXX`                   | `r` | r-shaped corner |
| 32 | `.X. / .X. / XXX`             | `b` | T-pentomino (stem + base = b) |
| 33 | `XX`                          | `2` | 2-cell domino (line) |

## Impact

- No change to game logic or state encoding — `name` is a display/logging aid
- Logging and debug output can reference `patch.name` instead of `patch.id`
- All 33 names are unique ASCII characters (letters or digits); case is significant
- `data/patches.yaml` gains one required field per patch entry
- `src/generated/patches.hpp` gains one `char` field per struct — zero run-time cost
