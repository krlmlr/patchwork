## Why

Patches are currently identified only by their integer ID (1–33) in logs and debug output. Reading a log line like `player placed patch 17` requires looking up the catalog to recall what shape that is. A single memorable character derived from the patch's visual shape makes logs instantly scannable: `player placed H` is self-explanatory.

## What Changes

- **Re-order and renumber** every entry in **`data/patches.yaml`** by tile size → button cost → income (descending); renumber `id` fields 1–33 to match the new order
- **Canonicalise** all `shape` values in `data/patches.yaml` to the lexicographically minimal grid representation across all rotations and reflections
- **Add a `name` field** (single ASCII letter or digit) to every entry — placed immediately after `id`
- Update **`codegen/generate_patches.R`** to read the new field and emit it into the generated C++ header
- Update **`cpp/generated/patches.hpp`** (regenerated) to include a `char name` field in `PatchData`
- Update **`openspec/specs/patch-catalog/spec.md`** to require the `name` field, canonical shapes, and the ordering

## Capabilities

### New Capabilities

- `patch_name`: every `PatchData` entry exposes a `char name` that identifies the patch by its shape at compile time

### Modified Capabilities

- `patch_catalog_yaml`: `data/patches.yaml` now requires a `name` field alongside `id`, `buttons`, `time`, `income`, and `shape`; entries are ordered by size → cost → income and shapes are canonical
- `patch_codegen`: `codegen/generate_patches.R` emits the `name` field into the generated struct
- `patch_header`: `cpp/generated/patches.hpp` exposes `char name` in `PatchData`

## Ordering

Patches are ordered in the catalog — and IDs renumbered — by:

1. **Cell count** (ascending): smaller tiles first
2. **Button cost** (ascending): cheaper tiles within the same size first
3. **Income** (descending): higher-income tiles within the same size+cost group first

The income tiebreaker is included for completeness; no pair of same-size, same-cost tiles is expected to require it.

## Canonical Shapes

Shapes in `data/patches.yaml` SHALL be stored in canonical form: the lexicographically smallest grid string (rows joined by newline) that can be produced by any rotation (0°/90°/180°/270°) or reflection of the tile.

Three pairs of patches share the same canonical shape — they have different button costs and/or incomes and therefore remain distinct entries with distinct names:

| New ID pair | Old ID pair | Canonical shape    | Family           |
|-------------|-------------|--------------------|------------------|
| 2 & 4       | 1 & 6       | `XX / X.`          | L/J-tromino      |
| 7 & 11      | 23 & 19     | `XX. / .XX`        | S/Z-tetromino    |
| 8 & 9       | 24 & 31     | `XXX / X..`        | L/J-tetromino    |

## Name Assignments

Names are chosen to resemble the patch's visual shape. Straight-line (I-polyomino) patches use the digit matching their cell count. Well-known polyomino shapes use their standard letter. Same-canonical-shape pairs use an upper/lower-case pairing so both letters evoke the shared shape, with the cheaper (lower button cost) tile taking the lowercase letter.

Each row lists a **Recommended** letter and two **Alternatives**. The alternatives in each row are independent options; they are not a pre-validated second alphabet — check uniqueness before combining them.

† marks entries that share a canonical shape with another entry.

| ID | Old | Cells | Btn | Inc | Canonical shape                       | Recommended | Alt A | Alt B | Rationale                                        |
|----|-----|-------|-----|-----|---------------------------------------|-------------|-------|-------|--------------------------------------------------|
|  1 |  33 |  2    |  2  |  0  | `XX`                                  | `2`         | `i`   | `─`   | 2-cell I-polyomino (domino)                      |
|  2 |   1 |  3    |  1  |  0  | `XX / X.` †                           | `v`         | `l`   | `L`   | L/J-tromino, cheaper                             |
|  3 |  30 |  3    |  2  |  0  | `XXX`                                 | `3`         | `i`   | `─`   | 3-cell I-polyomino (straight tromino)            |
|  4 |   6 |  3    |  3  |  0  | `XX / X.` †                           | `j`         | `r`   | `J`   | L/J-tromino, costlier; mirror orientation of ID 2 |
|  5 |   8 |  4    |  2  |  0  | `XXX / .X.`                           | `t`         | `T`   | `f`   | T-tetromino                                      |
|  6 |  21 |  4    |  3  |  1  | `XXXX`                                | `4`         | `I`   | `i`   | 4-cell I-polyomino (straight tetromino)          |
|  7 |  23 |  4    |  3  |  1  | `XX. / .XX` †                         | `s`         | `z`   | `Z`   | S/Z-tetromino, cheaper                           |
|  8 |  24 |  4    |  4  |  2  | `XXX / X..` †                         | `L`         | `l`   | `J`   | L/J-tetromino, higher income                     |
|  9 |  31 |  4    |  4  |  1  | `XXX / X..` †                         | `l`         | `j`   | `r`   | L/J-tetromino, lower income; same canon as ID 8  |
| 10 |  15 |  4    |  6  |  2  | `XX / XX`                             | `o`         | `0`   | `q`   | O-tetromino (2×2 square); lowercase o frees O for the octomino |
| 11 |  19 |  4    |  7  |  3  | `XX. / .XX` †                         | `S`         | `Z`   | `z`   | S/Z-tetromino, costlier; same canon as ID 7      |
| 12 |  29 |  5    |  1  |  0  | `XXX / X.X`                           | `u`         | `U`   | `c`   | U-pentomino                                      |
| 13 |  13 |  5    |  2  |  1  | `XXX. / ..XX`                         | `z`         | `n`   | `f`   | N-pentomino; `z` pairs with Z-hexomino (ID 26) by shape family |
| 14 |   9 |  5    |  2  |  0  | `XXX / XX.`                           | `p`         | `P`   | `b`   | P-pentomino                                      |
| 15 |  20 |  5    |  3  |  1  | `XXXX / .X..`                         | `y`         | `Y`   | `q`   | Y-pentomino                                      |
| 16 |   2 |  5    |  5  |  2  | `.X. / XXX / .X.`                     | `x`         | `X`   | `+`   | X-pentomino (small plus/cross)                   |
| 17 |  32 |  5    |  5  |  2  | `XXX / .X. / .X.`                     | `T`         | `t`   | `b`   | T-pentomino                                      |
| 18 |   4 |  5    |  7  |  1  | `XXXXX`                               | `5`         | `I`   | `i`   | 5-cell I-polyomino (straight pentomino)          |
| 19 |  11 |  5    | 10  |  3  | `XX. / .XX / ..X`                     | `w`         | `W`   | `m`   | W-pentomino (staircase)                          |
| 20 |  25 |  5    | 10  |  2  | `XXXX / X...`                         | `J`         | `j`   | `7`   | J-pentomino (5-cell L/J family)                  |
| 21 |  16 |  6    |  0  |  1  | `.X.. / XXXX / .X..`                  | `A`         | `G`   | `g`   | Asymmetric T-hexomino (offset cross)             |
| 22 |  18 |  6    |  1  |  1  | `XXXX / X..X`                         | `U`         | `u`   | `c`   | U-hexomino (open-top U)                          |
| 23 |   7 |  6    |  1  |  0  | `XX. / .X. / .X. / .XX`               | `N`         | `Z`   | `n`   | N/Z-hexomino (diagonal N-shape)                  |
| 24 |   3 |  6    |  2  |  0  | `.X.. / XXXX / ..X.`                  | `k`         | `Z`   | `s`   | Asymmetric zigzag hexomino (unique shape; `k` is unambiguous) |
| 25 |  28 |  6    |  3  |  2  | `XX. / .XX / XX.`                     | `e`         | `E`   | `k`   | Weaving/interlocked hexomino                     |
| 26 |  10 |  6    |  4  |  0  | `XXX. / .XXX`                         | `Z`         | `S`   | `k`   | S/Z-hexomino (elongated S)                       |
| 27 |  12 |  6    |  7  |  2  | `XXXX / .XX.`                         | `m`         | `M`   | `a`   | Arch/bridge hexomino (two bumps = two m-humps)   |
| 28 |  22 |  6    |  7  |  2  | `XXX / .X. / .X. / .X.`              | `1`         | `T`   | `b`   | Tall-stick hexomino (numeral 1 with base)        |
| 29 |   5 |  6    |  8  |  3  | `XX. / XXX / ..X`                     | `d`         | `D`   | `k`   | Step/staircase hexomino                          |
| 30 |  14 |  6    | 10  |  3  | `XXXX / XX..`                         | `q`         | `Q`   | `r`   | Hockey-stick hexomino                            |
| 31 |  27 |  7    |  1  |  1  | `.X. / .X. / XXX / .X. / .X.`        | `X`         | `x`   | `+`   | X-heptomino (large plus/cross)                   |
| 32 |  17 |  7    |  2  |  0  | `XXX / .X. / XXX`                     | `H`         | `h`   | `#`   | H-heptomino                                      |
| 33 |  26 |  8    |  5  |  1  | `.XX. / XXXX / .XX.`                  | `O`         | `8`   | `a`   | I-octomino (serif I-beam, visually a thick O/oval shape) |

The **Recommended** column is a conflict-free set of 33 unique characters:

`1`, `2`, `3`, `4`, `5`, `A`, `d`, `e`, `H`, `J`, `j`, `k`, `l`, `L`, `m`, `N`, `o`, `O`, `p`, `q`, `s`, `S`, `t`, `T`, `u`, `U`, `v`, `w`, `x`, `X`, `y`, `z`, `Z`

## Impact

- No change to game logic or state encoding — `name` is a display/logging aid
- Logging and debug output can reference `patch.name` instead of `patch.id`
- All 33 names are unique ASCII characters (letters or digits); case is significant
- `data/patches.yaml` gains one required field per patch entry; entries are reordered and shapes normalised
- `cpp/generated/patches.hpp` gains one `char` field per struct — zero run-time cost
