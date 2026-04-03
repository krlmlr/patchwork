## 1. Re-order, renumber, and canonicalise `data/patches.yaml`

- [ ] 1.1 Sort all 33 patch entries by (cell count ASC, buttons ASC, income DESC) and renumber `id` fields 1–33 to match the new order
- [ ] 1.2 Normalise each `shape` value to its canonical form (lexicographically minimal grid string across all 8 orientations); the three same-shape pairs that need new canonical shapes are:
  - New IDs 2 & 4 (old 1 & 6): both get `XX\nX.`
  - New IDs 7 & 11 (old 23 & 19): both get `XX.\n.XX`
  - New IDs 8 & 9 (old 24 & 31): both get `XXX\nX..`
- [ ] 1.3 Add a `name` field immediately after `id` to each entry using the recommended assignments from the proposal:
  - id 1 → `2`, id 2 → `v`, id 3 → `3`, id 4 → `j`, id 5 → `t`
  - id 6 → `4`, id 7 → `s`, id 8 → `L`, id 9 → `l`, id 10 → `o`
  - id 11 → `S`, id 12 → `u`, id 13 → `z`, id 14 → `p`, id 15 → `y`
  - id 16 → `x`, id 17 → `T`, id 18 → `5`, id 19 → `w`, id 20 → `J`
  - id 21 → `A`, id 22 → `U`, id 23 → `N`, id 24 → `k`, id 25 → `e`
  - id 26 → `Z`, id 27 → `m`, id 28 → `1`, id 29 → `d`, id 30 → `q`
  - id 31 → `X`, id 32 → `H`, id 33 → `O`
- [ ] 1.4 Verify that all 33 `name` values are unique and each is a single ASCII letter or digit

## 2. Update `codegen/generate_patches.R`

- [ ] 2.1 Read the `name` field from each patch entry in the YAML
- [ ] 2.2 Add `char name` as the second field in the `PatchData` struct definition emitted into the header (after `int id`)
- [ ] 2.3 Emit the `name` value as a character literal (e.g. `'w'`) in each `PatchData` initialiser in the `kPatches` array

## 3. Regenerate `src/generated/patches.hpp`

- [ ] 3.1 Run `Rscript codegen/generate_patches.R` to regenerate the header
- [ ] 3.2 Verify the generated file compiles without errors (`meson setup build && meson compile -C build`)
- [ ] 3.3 Verify all existing tests still pass (`meson test -C build`)

## 4. Update `openspec/specs/patch-catalog/spec.md`

- [ ] 4.1 Add `name` to the list of required fields in the "Each patch has required fields" scenario
- [ ] 4.2 Add scenario: **All patch names are unique single characters** — when all patch `name` fields are read, each is a single ASCII letter or digit and no two patches share the same value
- [ ] 4.3 Add scenario: **Generated header exposes patch name** — when a `PatchData` entry is accessed, its `name` field contains the single character specified in the corresponding catalog entry
- [ ] 4.4 Add scenario: **Catalog entries are in canonical-shape order** — when the catalog is read, entries appear in ascending order of (cell count, button cost, −income) and each `shape` value is the canonical (lexicographically minimal) form for that tile
