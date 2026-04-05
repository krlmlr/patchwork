## Why

Game concepts (patches, time track, quilt board, button income, etc.) are referenced inconsistently across source code, specs, and AI agent instructions. A single authoritative glossary establishes a shared vocabulary for humans writing rules, contributors writing code, and AI agents proposing or implementing changes.

## What Changes

- Add `docs/glossary.md` — canonical, human-readable glossary of all game and engine terminology; the initial list is defined in the design and will be expanded as new concepts are introduced
- Update `README.md` to link to the glossary and document its location in the project structure table
- Update `openspec/config.yaml` to reference the glossary as part of project context so AI agents consult it
- Update `.github/prompts/` agent prompt files to mention the glossary

## Capabilities

### New Capabilities

- `game-glossary`: Canonical Markdown glossary of all Patchwork game concepts and engine terminology, covering current and forward-looking terms. Grouped by category (game rules, engine, data). The design document proposes the initial term list.

### Modified Capabilities

## Impact

- New file: `docs/glossary.md`
- Modified files: `README.md`, `openspec/config.yaml`, `.github/prompts/opsx-apply.prompt.md` (or a shared context file referenced by all agent prompts)
- No C++ code or build system changes required
- No breaking changes
