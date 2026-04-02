## 1. Update `data/patches.yaml`

- [ ] 1.1 Add a `name` field to each of the 33 patch entries, placed immediately after `id`, using the assignments from the proposal:
  - id 1 → `l`, id 2 → `x`, id 3 → `n`, id 4 → `5`, id 5 → `z`
  - id 6 → `j`, id 7 → `N`, id 8 → `t`, id 9 → `P`, id 10 → `Z`
  - id 11 → `w`, id 12 → `m`, id 13 → `F`, id 14 → `q`, id 15 → `O`
  - id 16 → `T`, id 17 → `H`, id 18 → `U`, id 19 → `S`, id 20 → `y`
  - id 21 → `4`, id 22 → `1`, id 23 → `s`, id 24 → `L`, id 25 → `J`
  - id 26 → `I`, id 27 → `X`, id 28 → `Y`, id 29 → `u`, id 30 → `3`
  - id 31 → `r`, id 32 → `b`, id 33 → `2`
- [ ] 1.2 Verify that all 33 `name` values are unique and each is a single ASCII letter or digit

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
- [ ] 4.2 Add a new scenario: **All patch names are unique single characters** — when all patch `name` fields are read, each is a single ASCII letter or digit and no two patches share the same name
- [ ] 4.3 Add a new scenario: **Generated header exposes patch name** — when a `PatchData` entry is accessed, its `name` field contains the single character specified in the catalog
