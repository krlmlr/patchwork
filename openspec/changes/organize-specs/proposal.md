## Why

The `openspec/specs/` directory has grown organically across multiple changes and now contains specs from different domains (infrastructure, game core, game logic, agents) with no grouping or discovery aid. As more specs are added each phase, navigating and understanding the full capability surface becomes harder. An index establishes a shared vocabulary of domains so future spec names are consistent and predictable.

## What Changes

- Add `openspec/specs/README.md` — a human- and AI-readable catalog that groups every existing spec under a named domain, explains the domain taxonomy, and defines the naming convention for future specs.
- No existing spec files are modified; no code changes required.

## Capabilities

### New Capabilities

- `spec-catalog`: An index document at `openspec/specs/README.md` that organises all specs by domain, documents the domain taxonomy, and defines the naming convention for future specs.

### Modified Capabilities

_(none)_

## Impact

- Documentation only. No code, build, or test changes.
- Future changes should consult `openspec/specs/README.md` before choosing a spec name or domain.
