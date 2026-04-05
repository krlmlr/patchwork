## Why

Game concepts (patches, time track, quilt board, button income, etc.) are referenced inconsistently across source code, data files, specs, and AI agent instructions. A single authoritative glossary establishes a shared vocabulary for humans writing rules, contributors writing code, and AI agents proposing or implementing changes.

## What Changes

- Add `data/glossary.yaml` — machine-readable glossary (YAML, one entry per term with `term`, `definition`, `aliases`, and optional `see_also`)
- Add `docs/glossary.md` — human-readable glossary rendered from the YAML source (or maintained in parallel)
- Update `README.md` to link to the glossary and document its location in the project structure table
- Update `openspec/config.yaml` to reference the glossary as part of project context so AI agents consult it
- Update `.github/prompts/` agent prompt files to mention the glossary

## Capabilities

### New Capabilities

- `game-glossary`: Canonical, human- and machine-readable glossary of all Patchwork game concepts and engine terminology. Covers: patch (tile), patch circle, time track, quilt board, button income, button balance, leather patch, 7×7 bonus, player state, game state, move types (advance, buy), phases, and other domain terms used in code and specs.

### Modified Capabilities

- `patch-catalog`: The patch catalog spec references patches by their game-rule properties; the glossary will provide canonical definitions for terms like "button cost," "time cost," and "income" that appear there.

## Impact

- New files: `data/glossary.yaml`, `docs/glossary.md`
- Modified files: `README.md`, `openspec/config.yaml`, `.github/prompts/opsx-apply.prompt.md` (or a shared context file referenced by all agent prompts)
- No C++ code or build system changes required
- No breaking changes
