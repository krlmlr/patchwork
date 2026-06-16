## 1. Code fix: single source of truth for setups

- [ ] 1.1 Re-point `make_setup(id)` in `cpp/game_setups.hpp` to return `GameSetup(kGameSetups[id mod kNumGameSetups])`; remove the `kPatches` rotation logic and handle negative `id` via floored modulo
- [ ] 1.2 Add a Catch2 test asserting `make_setup(id)` produces the same circle as `kGameSetups[id mod kNumGameSetups]` for several ids (incl. one ≥ `kNumGameSetups`), and that the last character is `'2'`
- [ ] 1.3 Build and run the test suite (`mise run build && mise run test`) to confirm the driver/TUI now agree on circles

## 2. Docs fix: install invocation

- [ ] 2.1 Change `README.md` install line from `sudo bash scripts/install-tools.sh` to `bash scripts/install-tools.sh`
- [ ] 2.2 Change `BUILD.md` install line the same way

## 3. Glossary enhancement (`docs/glossary.md`)

- [ ] 3.1 Correct the **bonus tile** entry to distinguish the simplified-rules 56-occupied-cell proxy from the final-game 49-cell 7×7 area (deferred)
- [ ] 3.2 Add a **free spaces** entry (empty quilt cells; the score penalty term)
- [ ] 3.3 Add a **button-income space** entry (a.k.a. payout space; the time-track positions 5,11,17,23,29,35,41,47,53)

## 4. Spec catalog governance

- [ ] 4.1 Align the maintenance rule at the top of `openspec/specs/README.md` with the (modified) infrastructure "Catalog is kept up to date" requirement so the two never diverge

## 5. Verify and apply spec deltas

- [ ] 5.1 Run `openspec validate consolidate-spec-consistency` and resolve any delta-format issues (especially the TUI `RENAMED` operation; fall back to MODIFIED-only if needed)
- [ ] 5.2 Run `air format` / `mise run lint` as applicable and ensure markdown lint passes
- [ ] 5.3 Archive the change so the seven domain spec deltas are folded into `openspec/specs/` (`game-logic`, `engine`, `game-core`, `agents`, `tui`, `data`, `infrastructure`)
