## Context

The project has two standalone R scripts in `codegen/` (`generate_patches.R`, `generate_setups.R`). Each script starts with a `library(yaml)` call and contains all logic inline — parsing helpers, spec assertions, and code emission — with no shared infrastructure. Future roadmap phases (Tile Analysis, game logging) will add more R code; without shared structure this will lead to duplication and fragility.

The goal is to adopt minimal R package structure: a `DESCRIPTION` file, an `R/` directory for shared functions, and thin entry-point scripts in `codegen/` that call `pkgload::load_all()`. This does not require installation via `R CMD INSTALL` and does not need to pass `R CMD check`.

## Goals / Non-Goals

**Goals:**
- Add `DESCRIPTION` and `NAMESPACE` so `pkgload::load_all()` works from the project root
- Move reusable R functions out of `codegen/` scripts and into `R/` package files
- Refactor `codegen/` scripts to be thin entry points that call `pkgload::load_all()` then invoke package functions
- Keep generated C++ output byte-for-byte identical (no behavioral change)
- Lay the groundwork for `testthat`-based R unit tests

**Non-Goals:**
- Passing `R CMD check` or publishing to CRAN
- Installing the package via `R CMD INSTALL`
- Changing any data files or generated C++ headers
- Adding R tests in this change (tests are deferred to individual feature phases per project convention)

## Decisions

### Decision: Package root at project root (not a subdirectory)

Place `DESCRIPTION` and `R/` at the repository root rather than in a subdirectory (e.g. `r/` or `pkg/`).

**Rationale:** `pkgload::load_all()` without arguments loads the package in the current working directory. Scripts in `codegen/` that call `pkgload::load_all()` will work correctly when run from the project root (via `Rscript codegen/script.R`), matching the existing convention. A subdirectory would require passing a path argument and complicates the mise task runner.

**Alternative considered:** Placing the package under `r/`. Rejected because it requires changing all call sites and adds no value for a non-installable package.

### Decision: NAMESPACE managed by roxygen2

Use `roxygen2` to generate `NAMESPACE`, with `@export` tags on functions that should be part of the public API.

**Rationale:** roxygen2 is a standard R documentation and namespace tool. It co-locates export declarations with function definitions, making it easy to see what is and isn't exported. `DESCRIPTION` declares `Config/roxygen2/version` so that tools know to manage `NAMESPACE` via roxygen2.

**Alternative considered:** A hand-maintained `exportPattern("^[^\\.]")`. Accepted initially, then superseded by roxygen2 since it is fine as a project dependency.

### Decision: Split R functions by concern into separate files

Create `R/patches.R` for patch-related helpers and `R/setups.R` for setup-related helpers.

**Rationale:** Mirrors the existing script split (`generate_patches.R`, `generate_setups.R`) and keeps files focused. Shared low-level utilities (if any) can go into `R/utils.R`.

### Decision: codegen scripts become thin entry-point wrappers

Each `codegen/` script retains only: `pkgload::load_all(quiet = TRUE)`, argument/path setup, and a single top-level function call (e.g. `generate_patches("cpp/generated/patches.hpp")`).

**Rationale:** Keeps scripts readable and eliminates code that would otherwise be untestable. The function bodies live in `R/` where they can eventually be unit-tested.

## Risks / Trade-offs

- **`pkgload` must be available** → Risk: developer environments without `pkgload` installed will fail. Mitigation: `pkgload` is a standard devtools ecosystem package; it is already declared in `.mise.toml` or can be added to `DESCRIPTION`'s `Suggests` field so `renv`/`pak` can install it.
- **Package name conflicts** → Risk: naming the package `patchwork` conflicts with the popular ggplot2 extension of the same name. Mitigation: name the package `patchworkengine` (or another distinct name) in `DESCRIPTION`.
- **Working directory sensitivity** → Risk: scripts called from a non-root directory will fail to find the package. Mitigation: document the convention (always run from project root) and the mise tasks already enforce this.
