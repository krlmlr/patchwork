# Tile analysis: shape features, gain models, summary tables, and plots.
#
# Additional R packages required beyond DESCRIPTION:
#   - ggplot2   (plots)
#   - dplyr     (data manipulation)
#   - ggrepel   (non-overlapping labels in scatter plot)
#
# Run from the repository root:
#   Rscript analysis/tile_analysis.R

library(ggplot2)
library(dplyr)
library(ggrepel)

pkgload::load_all(quiet = TRUE)

# ---------------------------------------------------------------------------
# 1. Load patch catalog
# ---------------------------------------------------------------------------

catalog <- yaml::read_yaml("data/patches.yaml")
stopifnot(length(catalog) == 33)

# ---------------------------------------------------------------------------
# 2. Shape feature extraction
# ---------------------------------------------------------------------------

# Compute shape features from a cell-coordinate matrix returned by parse_cells().
shape_features <- function(cells_mat) {
  rows <- cells_mat[, 1]
  cols <- cells_mat[, 2]
  bbox_rows <- max(rows) - min(rows) + 1L
  bbox_cols <- max(cols) - min(cols) + 1L
  bbox_area <- bbox_rows * bbox_cols
  n_cells <- nrow(cells_mat)
  density <- n_cells / bbox_area

  # Perimeter: count exposed edges (adjacent to non-occupied cell or border).
  cell_set <- lapply(seq_len(n_cells), function(i) c(rows[i], cols[i]))
  occupied <- function(r, c) any(sapply(cell_set, function(p) p[1] == r && p[2] == c))
  deltas <- list(c(-1L, 0L), c(1L, 0L), c(0L, -1L), c(0L, 1L))
  perimeter <- 0L
  for (i in seq_len(n_cells)) {
    r <- rows[i]; c <- cols[i]
    for (d in deltas) {
      if (!occupied(r + d[1], c + d[2])) perimeter <- perimeter + 1L
    }
  }

  list(
    bbox_rows = bbox_rows,
    bbox_cols = bbox_cols,
    bbox_area = bbox_area,
    density   = density,
    perimeter = perimeter
  )
}

# Verify perimeter for a 2-cell horizontal patch "XX" -> expected 6.
test_cells <- parse_cells("XX")
test_feat  <- shape_features(test_cells)
stopifnot(test_feat$perimeter == 6L)

# Extract shape features for all patches.
patch_df <- do.call(rbind, lapply(catalog, function(p) {
  cells_mat <- parse_cells(p$shape)
  feat <- shape_features(cells_mat)
  n_cells <- nrow(cells_mat)
  # Verify cells match count_x().
  stopifnot(n_cells == count_x(p$shape))
  data.frame(
    id           = as.integer(p$id),
    name         = p$name,
    button_cost  = as.integer(p$buttons),
    time_cost    = as.integer(p$time),
    button_income = as.integer(p$income),
    cells        = n_cells,
    bbox_rows    = feat$bbox_rows,
    bbox_cols    = feat$bbox_cols,
    bbox_area    = feat$bbox_area,
    density      = feat$density,
    perimeter    = feat$perimeter,
    stringsAsFactors = FALSE
  )
}))

# ---------------------------------------------------------------------------
# 3. Patch gain model
# ---------------------------------------------------------------------------

# Placement gain = 2 * cells - button_cost (may be negative).
patch_df$placement_gain <- 2L * patch_df$cells - patch_df$button_cost

# Payout spaces on the time track.
payout_spaces <- c(5L, 11L, 17L, 23L, 29L, 35L, 41L, 47L, 53L)

# Count payout spaces strictly ahead of pos.
reachable_payouts <- function(pos) {
  sum(payout_spaces > pos)
}

# Verify required values.
stopifnot(reachable_payouts(0)  == 9L)
stopifnot(reachable_payouts(18) == 6L)
stopifnot(reachable_payouts(36) == 3L)
stopifnot(reachable_payouts(53) == 0L)

# Compute gain_per_time for a single patch row at a given position.
# Returns NA if time_cost == 0.
gain_per_time_at <- function(placement_gain, button_income, time_cost, pos) {
  if (time_cost == 0L) return(NA_real_)
  (placement_gain + button_income * reachable_payouts(pos)) / time_cost
}

positions <- 0L:53L

# Build per-patch gain curves (all 33 patches × 54 positions).
gain_curves_list <- lapply(seq_len(nrow(patch_df)), function(i) {
  row <- patch_df[i, ]
  gpt <- vapply(positions, function(pos) {
    gain_per_time_at(row$placement_gain, row$button_income, row$time_cost, pos)
  }, numeric(1))
  # Verify non-increasing (patch gain can only decrease or stay flat as pos rises).
  finite_idx <- is.finite(gpt)
  if (any(finite_idx)) {
    diffs <- diff(gpt[finite_idx])
    stopifnot(all(diffs <= 0))
  }
  data.frame(
    id            = row$id,
    name          = row$name,
    pos           = positions,
    gain_per_time = gpt,
    stringsAsFactors = FALSE
  )
})
gain_curves_df <- do.call(rbind, gain_curves_list)

# Compute advance_breakeven_pos: latest pos where gain_per_time >= 1.0.
advance_breakeven_pos <- vapply(seq_len(nrow(patch_df)), function(i) {
  row <- patch_df[i, ]
  gpt <- vapply(positions, function(pos) {
    gain_per_time_at(row$placement_gain, row$button_income, row$time_cost, pos)
  }, numeric(1))
  eligible <- which(is.finite(gpt) & gpt >= 1.0)
  if (length(eligible) == 0L) NA_integer_ else as.integer(positions[max(eligible)])
}, integer(1))
patch_df$advance_breakeven_pos <- advance_breakeven_pos

# Add gain_per_time at key positions.
patch_df$gain_per_time_pos0  <- mapply(gain_per_time_at,
  patch_df$placement_gain, patch_df$button_income, patch_df$time_cost, 0L)
patch_df$gain_per_time_pos18 <- mapply(gain_per_time_at,
  patch_df$placement_gain, patch_df$button_income, patch_df$time_cost, 18L)
patch_df$gain_per_time_pos36 <- mapply(gain_per_time_at,
  patch_df$placement_gain, patch_df$button_income, patch_df$time_cost, 36L)

# ---------------------------------------------------------------------------
# 4. Summary tables
# ---------------------------------------------------------------------------

dir.create("analysis/output", showWarnings = FALSE, recursive = TRUE)

# 4.1 / 4.2  tile_summary.csv
summary_df <- patch_df[, c(
  "id", "name", "button_cost", "time_cost", "button_income",
  "cells", "placement_gain",
  "bbox_rows", "bbox_cols", "density", "perimeter",
  "gain_per_time_pos0", "gain_per_time_pos18", "gain_per_time_pos36",
  "advance_breakeven_pos"
)]
stopifnot(nrow(summary_df) == 33L)
write.csv(summary_df, "analysis/output/tile_summary.csv", row.names = FALSE)

# 4.3 / 4.4  advance_breakeven.csv
breakeven_df <- patch_df[, c(
  "id", "name", "button_cost", "time_cost", "button_income",
  "placement_gain", "gain_per_time_pos0", "advance_breakeven_pos"
)]
# Sort by advance_breakeven_pos descending, NA last.
breakeven_df <- breakeven_df[order(-ifelse(is.na(breakeven_df$advance_breakeven_pos),
                                           -Inf,
                                           breakeven_df$advance_breakeven_pos)), ]
write.csv(breakeven_df, "analysis/output/advance_breakeven.csv", row.names = FALSE)

# 4.5  Spot-check three patches against patches.yaml.
for (check_id in c(1L, 10L, 33L)) {
  yaml_p <- catalog[[check_id]]
  df_p   <- patch_df[patch_df$id == check_id, ]
  stopifnot(df_p$button_cost   == as.integer(yaml_p$buttons))
  stopifnot(df_p$time_cost     == as.integer(yaml_p$time))
  stopifnot(df_p$button_income == as.integer(yaml_p$income))
  # advance_breakeven_pos must be NA or in 0-53.
  abp <- df_p$advance_breakeven_pos
  stopifnot(is.na(abp) || (abp >= 0L && abp <= 53L))
}

# ---------------------------------------------------------------------------
# 5. Plots
# ---------------------------------------------------------------------------

# 5.1  gain_per_time.png — bar chart at pos 0, sorted descending.
plot_df <- summary_df[order(-summary_df$gain_per_time_pos0, na.last = TRUE), ]
plot_df$name_f <- factor(plot_df$name, levels = plot_df$name)
plot_df$category <- ifelse(!is.na(plot_df$gain_per_time_pos0) & plot_df$gain_per_time_pos0 >= 1.0,
                           "strong buy (≥ 1.0)", "subthreshold (< 1.0)")

p1 <- ggplot(plot_df[!is.na(plot_df$gain_per_time_pos0), ],
             aes(x = name_f, y = gain_per_time_pos0, fill = category)) +
  geom_col() +
  geom_hline(yintercept = 1.0, linetype = "dashed", colour = "black") +
  scale_fill_manual(values = c("strong buy (≥ 1.0)" = "#2196F3",
                               "subthreshold (< 1.0)" = "#BDBDBD")) +
  labs(title = "Patch gain per time cost at position 0",
       x = "Patch", y = "gain_per_time (pos 0)", fill = NULL) +
  theme_minimal() +
  theme(axis.text.x = element_text(size = 7))
ggsave("analysis/output/gain_per_time.png", p1, width = 10, height = 5, dpi = 150)

# 5.2  gain_curves.png — line plot for income patches, ref line at 1.0.
income_patches <- unique(gain_curves_df$name[gain_curves_df$id %in%
                          patch_df$id[patch_df$button_income > 0]])
curves_subset <- gain_curves_df[gain_curves_df$name %in% income_patches, ]

p2 <- ggplot(curves_subset, aes(x = pos, y = gain_per_time, colour = name, group = name)) +
  geom_line(alpha = 0.8) +
  geom_hline(yintercept = 1.0, linetype = "dashed", colour = "black") +
  labs(title = "Gain per time cost over time-track positions (income patches)",
       x = "Time-track position", y = "gain_per_time", colour = "Patch") +
  theme_minimal()
ggsave("analysis/output/gain_curves.png", p2, width = 10, height = 6, dpi = 150)

# 5.3  gain_heatmap.png — heatmap all 33 patches × 54 positions.
# Order patches by gain_per_time at pos 0 descending.
patch_order <- patch_df$name[order(-patch_df$gain_per_time_pos0, na.last = TRUE)]
gain_curves_df$name_f <- factor(gain_curves_df$name, levels = rev(patch_order))

p3 <- ggplot(gain_curves_df, aes(x = pos, y = name_f, fill = gain_per_time)) +
  geom_tile() +
  scale_fill_gradient2(low = "#D32F2F", mid = "#FFFDE7", high = "#1976D2",
                       midpoint = 1.0, name = "gain/time") +
  labs(title = "Patch gain per time cost: all patches × all positions",
       x = "Time-track position (0–53)", y = "Patch") +
  theme_minimal() +
  theme(axis.text.y = element_text(size = 7))
ggsave("analysis/output/gain_heatmap.png", p3, width = 12, height = 7, dpi = 150)

# 5.4  shape_density.png — scatter density vs. cells with labels.
# Set seed for ggrepel label placement reproducibility.
set.seed(42L)
p4 <- ggplot(patch_df, aes(x = cells, y = density, label = name)) +
  geom_point(colour = "#1976D2") +
  geom_text_repel(size = 3, max.overlaps = 50) +
  labs(title = "Shape density vs. cell count",
       x = "Cell count", y = "Density (cells / bbox area)") +
  theme_minimal()
ggsave("analysis/output/shape_density.png", p4, width = 8, height = 6, dpi = 150)

# 5.5  Confirm all four PNG files are non-empty.
png_files <- c(
  "analysis/output/gain_per_time.png",
  "analysis/output/gain_curves.png",
  "analysis/output/gain_heatmap.png",
  "analysis/output/shape_density.png"
)
for (f in png_files) {
  stopifnot(file.exists(f) && file.info(f)$size > 0L)
}

cat("tile_analysis.R completed successfully.\n")
cat("Outputs written to analysis/output/:\n")
cat("  tile_summary.csv       (", nrow(summary_df), "rows )\n")
cat("  advance_breakeven.csv  (", nrow(breakeven_df), "rows )\n")
for (f in png_files) cat("  ", basename(f), " (", file.info(f)$size, "bytes )\n")
