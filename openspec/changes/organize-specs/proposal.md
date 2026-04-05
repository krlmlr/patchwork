## Why

The `openspec/specs/` directory has grown organically across multiple changes and now contains specs from different domains (infrastructure, game core, game logic, agents) with no grouping or discovery aid. As more specs are added each phase, navigating and understanding the full capability surface becomes harder. Without explicit decision rules, new specs end up with inconsistent names and arbitrary domain assignments, making it harder for both humans and AI agents to reason about the spec surface.

## What Changes

- Add `openspec/specs/README.md` — a human- and AI-readable catalog that:
  - Groups every existing spec under a named domain
  - Provides a one-sentence description of each domain
  - States clear, unambiguous decision rules for assigning a new spec to exactly one domain
  - Lists all in-progress specs (simplified-rules) and their domains
  - Documents the kebab-case naming convention for spec folder names
- No existing spec files are moved or renamed (the openspec tooling relies on stable flat paths).

## Capabilities

### New Capabilities

- `spec-catalog`: An index document at `openspec/specs/README.md` that organises all specs by domain, defines the domain taxonomy with decision rules, and serves as the authoritative guide for naming and placing future specs.

### Modified Capabilities

_(none)_

## Impact

- Documentation only. No code, build, or test changes.
- Every future change MUST consult `openspec/specs/README.md` before choosing a spec name or domain.
- Every future change that adds a spec MUST also update the catalog.
