#!/usr/bin/env Rscript
# Generate src/generated/patches.hpp from data/patches.yaml
#
# Usage: Rscript codegen/generate_patches.R
#   Run from the project root directory.

library(yaml)

catalog <- yaml::read_yaml("data/patches.yaml")
stopifnot(length(catalog) == 33)

# ---------------------------------------------------------------------------
# Spec assertions: validate every requirement from openspec/specs/patch-catalog/spec.md
# ---------------------------------------------------------------------------

# Scenario: Each patch has required fields
# Each entry must have id (integer 1-33), name (single-char string [A-Za-z0-9]),
# buttons/time/income (integers), and shape (string).
for (p in catalog) {
  stopifnot(
    is.integer(p$id) || is.numeric(p$id),
    is.character(p$name),
    nchar(p$name) == 1L,
    grepl("^[A-Za-z0-9]$", p$name),
    is.integer(p$buttons) || is.numeric(p$buttons),
    is.integer(p$time) || is.numeric(p$time),
    is.integer(p$income) || is.numeric(p$income),
    is.character(p$shape)
  )
}

# Scenario: IDs are 1-33 assigned sequentially
ids <- vapply(catalog, function(p) as.integer(p$id), integer(1))
stopifnot(identical(ids, 1L:33L))

# Scenario: All patch names are unique single characters
names_vec <- vapply(catalog, function(p) p$name, character(1))
stopifnot(length(unique(names_vec)) == 33L)

# Scenario: Catalog entries are sorted by (cell count ASC, buttons ASC, income DESC)
count_x <- function(shape_str) {
  nchar(gsub("[^X]", "", shape_str))
}
cell_counts <- vapply(catalog, function(p) count_x(p$shape), integer(1))
buttons_vec <- vapply(catalog, function(p) as.integer(p$buttons), integer(1))
income_vec  <- vapply(catalog, function(p) as.integer(p$income),  integer(1))
sort_key    <- data.frame(cells = cell_counts, buttons = buttons_vec, neg_income = -income_vec)
for (i in seq_len(nrow(sort_key) - 1L)) {
  a <- sort_key[i, ]
  b <- sort_key[i + 1L, ]
  stopifnot(
    a$cells < b$cells ||
    (a$cells == b$cells && a$buttons < b$buttons) ||
    (a$cells == b$cells && a$buttons == b$buttons && a$neg_income <= b$neg_income)
  )
}

# Helpers for canonical-shape check
# Canonical form: the orientation (among all 8 = 4 rotations x 2 reflections) that
# yields the lexicographically smallest sequence of sorted normalised (row, col)
# cell coordinates.  This key is compared as a character string "r,c;r,c;..." so
# (0,0;0,1) < (0,0;1,0) because "0,0;0,1" < "0,0;1,0", meaning wide/horizontal
# representations are preferred over tall/vertical ones.

# Rotate 90 degrees clockwise: (r, c) -> (c, -r) then renormalise
rotate90 <- function(cells) {
  cbind(cells[, 2], -cells[, 1])
}

reflect_h <- function(cells) {
  cbind(cells[, 1], -cells[, 2])
}

normalise_cells <- function(cells) {
  cells[, 1] <- cells[, 1] - min(cells[, 1])
  cells[, 2] <- cells[, 2] - min(cells[, 2])
  cells[order(cells[, 1], cells[, 2]), ]
}

# Canonical key: sorted cell coordinates as "r,c;r,c;..."
cell_key <- function(cells) {
  nc <- normalise_cells(cells)
  nc <- nc[order(nc[, 1], nc[, 2]), ]
  paste(apply(nc, 1, paste, collapse = ","), collapse = ";")
}

cells_to_rows <- function(cells) {
  max_r <- max(cells[, 1])
  max_c <- max(cells[, 2])
  sapply(0:max_r, function(r) {
    paste(sapply(0:max_c, function(c) {
      if (any(cells[, 1] == r & cells[, 2] == c)) "X" else "."
    }), collapse = "")
  })
}

# Parse a shape string into a normalised cell matrix
parse_cells <- function(shape_str) {
  rows <- strsplit(trimws(shape_str, which = "right"), "\n")[[1]]
  cells_list <- list()
  for (r in seq_along(rows)) {
    chars <- strsplit(rows[[r]], "")[[1]]
    for (cc in seq_along(chars)) {
      if (chars[[cc]] == "X") {
        cells_list <- c(cells_list, list(c(r - 1L, cc - 1L)))
      }
    }
  }
  matrix(unlist(cells_list), ncol = 2, byrow = TRUE)
}

# Return the canonical grid string for a shape
canonical_shape <- function(shape_str) {
  cells <- parse_cells(shape_str)
  variants <- vector("list", 8L)
  cur <- cells
  for (k in 1:4) {
    variants[[2 * k - 1]] <- normalise_cells(cur)
    variants[[2 * k]]     <- normalise_cells(reflect_h(cur))
    cur <- normalise_cells(rotate90(cur))
  }
  keys <- vapply(variants, cell_key, character(1))
  best <- variants[[which(keys == min(keys))[1]]]
  paste(cells_to_rows(best), collapse = "\n")
}

# Scenario: Patch shapes are in canonical form
for (p in catalog) {
  stored <- trimws(p$shape, which = "right")
  # normalise trailing newline from YAML block scalar
  stored_trimmed <- paste(strsplit(stored, "\n")[[1]], collapse = "\n")
  canon <- canonical_shape(stored_trimmed)
  stopifnot(stored_trimmed == canon)
}

# Scenario: Shape parses to correct cell count (X count equals reported num_cells)
# (num_cells is computed below during HPP generation; here we just verify X counts > 0)
for (p in catalog) {
  x_count <- count_x(p$shape)
  stopifnot(x_count >= 2L)  # smallest patch is 2 cells
}

# ---------------------------------------------------------------------------
# End spec assertions
# ---------------------------------------------------------------------------

# Parse ASCII art shape into list of (row, col) offsets
parse_shape <- function(shape_str) {
  rows <- strsplit(trimws(shape_str, which = "right"), "\n")[[1]]
  # Verify rectangular
  lengths <- nchar(rows)
  stopifnot(length(unique(lengths)) == 1)

  cells <- list()
  for (r in seq_along(rows)) {
    chars <- strsplit(rows[r], "")[[1]]
    for (c in seq_along(chars)) {
      if (chars[c] == "X") {
        cells <- c(cells, list(c(r - 1L, c - 1L)))
      }
    }
  }
  cells
}

# Build output
lines <- character()
add <- function(...) lines <<- c(lines, paste0(...))

add("// AUTO-GENERATED by codegen/generate_patches.R — DO NOT EDIT")
add("// Source: data/patches.yaml")
add("")
add("#ifndef PATCHWORK_GENERATED_PATCHES_HPP")
add("#define PATCHWORK_GENERATED_PATCHES_HPP")
add("")
add("#include <array>")
add("#include <cstdint>")
add("")
add("namespace patchwork {")
add("")
add("struct CellOffset {")
add("    std::int8_t row;")
add("    std::int8_t col;")
add("};")
add("")
add("struct PatchData {")
add("    int id;")
add("    char name;")
add("    int buttons;")
add("    int time;")
add("    int income;")
add("    int num_cells;")
add("    std::array<CellOffset, 8> cells;  // max 8 cells per patch; unused entries are {0,0}")
add("};")
add("")
add("inline constexpr std::array<PatchData, 33> kPatches = {{")

for (i in seq_along(catalog)) {
  p <- catalog[[i]]
  cells <- parse_shape(p$shape)
  n <- length(cells)

  # Format cells array, pad to 8
  cell_strs <- vapply(cells, function(c) sprintf("{%d, %d}", c[1], c[2]), character(1))
  while (length(cell_strs) < 8) {
    cell_strs <- c(cell_strs, "{0, 0}")
  }
  cells_init <- paste(cell_strs, collapse = ", ")

  comma <- if (i < length(catalog)) "," else ""
  add(sprintf("    {%d, '%s', %d, %d, %d, %d, {{%s}}}%s",
              p$id, p$name, p$buttons, p$time, p$income, n, cells_init, comma))
}

add("}};")
add("")
add("static_assert(kPatches.size() == 33, \"Expected exactly 33 patches\");")
add("")
add("}  // namespace patchwork")
add("")
add("#endif  // PATCHWORK_GENERATED_PATCHES_HPP")

output_path <- "src/generated/patches.hpp"
dir.create(dirname(output_path), showWarnings = FALSE, recursive = TRUE)
writeLines(lines, output_path)
cat("Generated", output_path, "\n")
