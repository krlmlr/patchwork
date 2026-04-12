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
# Returns perimeter (exposed edges), convex/concave corners, and circumference.
#
# Circumference equals perimeter for grid-cell shapes: both count exposed cell
# edges on the boundary. Corners are a vertex-level property: at each grid
# vertex shared by 1 occupied cell → convex corner; by 3 → concave corner;
# by 2 diagonally opposite → 2 convex (saddle). For simply-connected shapes
# the discrete Gauss-Bonnet theorem gives convex − concave = 4.
shape_features <- function(cells_mat) {
  rows <- cells_mat[, 1]
  cols <- cells_mat[, 2]
  bbox_rows <- max(rows) - min(rows) + 1L
  bbox_cols <- max(cols) - min(cols) + 1L
  bbox_area <- bbox_rows * bbox_cols
  n_cells <- nrow(cells_mat)
  density <- n_cells / bbox_area

  # Fast occupancy lookup via character key set.
  occ_keys <- paste(rows, cols, sep = ",")
  is_occ <- function(r, c) paste(r, c, sep = ",") %in% occ_keys

  # Perimeter: count exposed edges (adjacent to non-occupied cell or border).
  deltas <- list(c(-1L, 0L), c(1L, 0L), c(0L, -1L), c(0L, 1L))
  perimeter <- 0L
  for (i in seq_len(n_cells)) {
    r <- rows[i]; c <- cols[i]
    for (d in deltas) {
      if (!is_occ(r + d[1], c + d[2])) perimeter <- perimeter + 1L
    }
  }

  # Circumference = perimeter for grid-cell shapes.
  circumference <- perimeter

  # Corners: inspect every grid vertex in the bounding box (+ 1 border row/col).
  min_r <- min(rows); max_r <- max(rows)
  min_c <- min(cols); max_c <- max(cols)
  convex_corners  <- 0L
  concave_corners <- 0L
  for (vr in min_r:(max_r + 1L)) {
    for (vc in min_c:(max_c + 1L)) {
      tl <- is_occ(vr - 1L, vc - 1L)
      tr <- is_occ(vr - 1L, vc)
      bl <- is_occ(vr, vc - 1L)
      br <- is_occ(vr, vc)
      n <- sum(c(tl, tr, bl, br))
      if (n == 1L) {
        convex_corners <- convex_corners + 1L
      } else if (n == 3L) {
        concave_corners <- concave_corners + 1L
      } else if (n == 2L && ((tl && br && !tr && !bl) || (!tl && !br && tr && bl))) {
        convex_corners <- convex_corners + 2L
      }
    }
  }

  list(
    bbox_rows       = bbox_rows,
    bbox_cols       = bbox_cols,
    bbox_area       = bbox_area,
    density         = density,
    perimeter       = perimeter,
    circumference   = circumference,
    convex_corners  = convex_corners,
    concave_corners = concave_corners,
    corners         = convex_corners + concave_corners
  )
}

# Verify perimeter for a 2-cell horizontal patch "XX" -> expected 6.
test_cells <- parse_cells("XX")
test_feat  <- shape_features(test_cells)
stopifnot(test_feat$perimeter == 6L)
stopifnot(test_feat$circumference == 6L)
stopifnot(test_feat$convex_corners == 4L)
stopifnot(test_feat$concave_corners == 0L)
# Verify Gauss-Bonnet: convex - concave = 4.
stopifnot(test_feat$convex_corners - test_feat$concave_corners == 4L)

# Extract shape features for all patches.
patch_df <- do.call(rbind, lapply(catalog, function(p) {
  cells_mat <- parse_cells(p$shape)
  feat <- shape_features(cells_mat)
  n_cells <- nrow(cells_mat)
  # Verify cells match count_x().
  stopifnot(n_cells == count_x(p$shape))
  # Verify circumference equals perimeter.
  stopifnot(feat$circumference == feat$perimeter)
  # Verify Gauss-Bonnet for simply-connected shapes.
  stopifnot(feat$convex_corners - feat$concave_corners == 4L)
  data.frame(
    id              = as.integer(p$id),
    name            = p$name,
    button_cost     = as.integer(p$buttons),
    time_cost       = as.integer(p$time),
    button_income   = as.integer(p$income),
    cells           = n_cells,
    bbox_rows       = feat$bbox_rows,
    bbox_cols       = feat$bbox_cols,
    bbox_area       = feat$bbox_area,
    density         = feat$density,
    perimeter       = feat$perimeter,
    circumference   = feat$circumference,
    convex_corners  = feat$convex_corners,
    concave_corners = feat$concave_corners,
    corners         = feat$corners,
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

# Total gain (not normalised by time): placement_gain + income * reachable_payouts.
total_gain_at <- function(placement_gain, button_income, pos) {
  placement_gain + button_income * reachable_payouts(pos)
}

# Build per-patch total gain across all positions.
total_gain_list <- lapply(seq_len(nrow(patch_df)), function(i) {
  row <- patch_df[i, ]
  tg <- vapply(positions, function(pos) {
    total_gain_at(row$placement_gain, row$button_income, pos)
  }, numeric(1))
  data.frame(
    id         = row$id,
    name       = row$name,
    button_income = row$button_income,
    pos        = positions,
    total_gain = tg,
    stringsAsFactors = FALSE
  )
})
total_gain_df <- do.call(rbind, total_gain_list)

# Position bands defined by payout spaces: within each band, reachable_payouts
# is constant. Representative position for each band is its lower bound.
band_breaks <- c(0L, payout_spaces)
band_labels <- c("0–4", "5–10", "11–16", "17–22", "23–28",
                 "29–34", "35–40", "41–46", "47–52", "53")
band_rep_pos <- c(0L, 5L, 11L, 17L, 23L, 29L, 35L, 41L, 47L, 53L)

# Build gain_per_time at each representative position for all patches.
band_gpt_list <- lapply(seq_len(nrow(patch_df)), function(i) {
  row <- patch_df[i, ]
  data.frame(
    id            = row$id,
    name          = row$name,
    button_income = row$button_income,
    band          = factor(band_labels, levels = band_labels),
    rep_pos       = band_rep_pos,
    gain_per_time = vapply(band_rep_pos, function(pos) {
      gain_per_time_at(row$placement_gain, row$button_income, row$time_cost, pos)
    }, numeric(1)),
    total_gain    = vapply(band_rep_pos, function(pos) {
      total_gain_at(row$placement_gain, row$button_income, pos)
    }, numeric(1)),
    stringsAsFactors = FALSE
  )
})
band_df <- do.call(rbind, band_gpt_list)

# ---------------------------------------------------------------------------
# 4. Summary tables
# ---------------------------------------------------------------------------

dir.create("analysis/output", showWarnings = FALSE, recursive = TRUE)

# 4.1 / 4.2  tile_summary.csv
summary_df <- patch_df[, c(
  "id", "name", "button_cost", "time_cost", "button_income",
  "cells", "placement_gain",
  "bbox_rows", "bbox_cols", "density", "perimeter",
  "circumference", "convex_corners", "concave_corners", "corners",
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

# Income factor labels for fill colouring.
patch_df$income_f <- factor(patch_df$button_income)
band_df$income_f  <- factor(band_df$button_income)

# 5.1  gain_per_time.png — faceted bar chart by position band, fill by income.
# Sort patches by gain_per_time at pos 0, descending.
patch_order_gpt <- patch_df$name[order(-patch_df$gain_per_time_pos0, na.last = TRUE)]
band_df$name_f <- factor(band_df$name, levels = patch_order_gpt)

p1 <- ggplot(band_df[!is.na(band_df$gain_per_time), ],
             aes(x = name_f, y = gain_per_time, fill = income_f)) +
  geom_col() +
  geom_hline(yintercept = 1.0, linetype = "dashed", colour = "black") +
  scale_y_continuous(breaks = scales::breaks_width(1)) +
  facet_wrap(~ band, nrow = 2, scales = "free_y") +
  labs(title = "Patch gain per time cost by position band",
       x = "Patch", y = "gain / time", fill = "Income") +
  theme_minimal() +
  theme(axis.text.x = element_text(size = 6, angle = 90, hjust = 1, vjust = 0.5))
ggsave("analysis/output/gain_per_time.png", p1, width = 14, height = 8, dpi = 150)

# 5.2  gain_curves.png — line plot for income patches with text labels.
income_patches <- unique(gain_curves_df$name[gain_curves_df$id %in%
                          patch_df$id[patch_df$button_income > 0]])
curves_subset <- gain_curves_df[gain_curves_df$name %in% income_patches, ]

# Add income for color mapping.
curves_subset <- merge(curves_subset,
                       patch_df[, c("id", "button_income")],
                       by = "id")
curves_subset$income_f <- factor(curves_subset$button_income)

# Label points at integer y values and every position.
p2 <- ggplot(curves_subset, aes(x = pos, y = gain_per_time,
                                colour = name, group = name)) +
  geom_line(alpha = 0.8) +
  geom_text_repel(aes(label = name), size = 2.5, max.overlaps = 100,
                  segment.alpha = 0.3, show.legend = FALSE, seed = 42L) +
  geom_hline(yintercept = 1.0, linetype = "dashed", colour = "black") +
  scale_y_continuous(breaks = scales::breaks_width(1)) +
  labs(title = "Gain per time cost over time-track positions (income patches)",
       x = "Time-track position", y = "gain / time", colour = "Patch") +
  theme_minimal()
ggsave("analysis/output/gain_curves.png", p2, width = 12, height = 7, dpi = 150)

# 5.3  gain_heatmap.png — heatmap with axes flipped: patches on x, positions on y.
patch_order <- patch_df$name[order(-patch_df$gain_per_time_pos0, na.last = TRUE)]
gain_curves_df$name_f <- factor(gain_curves_df$name, levels = patch_order)

p3 <- ggplot(gain_curves_df, aes(x = name_f, y = pos, fill = gain_per_time)) +
  geom_tile() +
  scale_fill_gradient2(low = "#D32F2F", mid = "#FFFDE7", high = "#1976D2",
                       midpoint = 1.0, name = "gain/time") +
  labs(title = "Patch gain per time cost: all patches × all positions",
       x = "Patch", y = "Time-track position (0–53)") +
  theme_minimal() +
  theme(axis.text.x = element_text(size = 7, angle = 90, hjust = 1, vjust = 0.5))
ggsave("analysis/output/gain_heatmap.png", p3, width = 10, height = 8, dpi = 150)

# 5.4  shape_density.png — scatter density vs. cells with labels.
p4 <- ggplot(patch_df, aes(x = cells, y = density, label = name)) +
  geom_point(colour = "#1976D2") +
  geom_text_repel(size = 3, max.overlaps = 50, seed = 42L) +
  labs(title = "Shape density vs. cell count",
       x = "Cell count", y = "Density (cells / bbox area)") +
  theme_minimal()
ggsave("analysis/output/shape_density.png", p4, width = 8, height = 6, dpi = 150)

# 5.5  total_gain.png — faceted bar chart of total gain by position band,
#      fill by income, sorted by total_gain at pos 0 descending.
patch_order_tg <- patch_df$name[order(-(2L * patch_df$cells - patch_df$button_cost +
                                        patch_df$button_income * 9L), na.last = TRUE)]
band_df$name_tg <- factor(band_df$name, levels = patch_order_tg)

p5 <- ggplot(band_df, aes(x = name_tg, y = total_gain, fill = income_f)) +
  geom_col() +
  geom_hline(yintercept = 0, linetype = "dashed", colour = "black") +
  scale_y_continuous(breaks = scales::breaks_width(1)) +
  facet_wrap(~ band, nrow = 2, scales = "free_y") +
  labs(title = "Total patch gain by position band",
       x = "Patch", y = "Total gain", fill = "Income") +
  theme_minimal() +
  theme(axis.text.x = element_text(size = 6, angle = 90, hjust = 1, vjust = 0.5))
ggsave("analysis/output/total_gain.png", p5, width = 14, height = 8, dpi = 150)

# 5.6  shape_corners.png — scatter of corners (convex, concave) vs. cells.
corners_long <- rbind(
  data.frame(name = patch_df$name, cells = patch_df$cells,
             type = "convex", count = patch_df$convex_corners,
             stringsAsFactors = FALSE),
  data.frame(name = patch_df$name, cells = patch_df$cells,
             type = "concave", count = patch_df$concave_corners,
             stringsAsFactors = FALSE)
)
p6 <- ggplot(corners_long, aes(x = cells, y = count, colour = type, label = name)) +
  geom_point(alpha = 0.7) +
  geom_text(vjust = -0.8, size = 2.5, show.legend = FALSE) +
  facet_wrap(~ type, scales = "free_y") +
  labs(title = "Corner count vs. cell count",
       subtitle = "convex \u2212 concave = 4 for all simply-connected shapes (discrete Gauss-Bonnet)",
       x = "Cell count", y = "Number of corners", colour = "Corner type") +
  scale_colour_manual(values = c(convex = "#1976D2", concave = "#D32F2F")) +
  theme_minimal()
ggsave("analysis/output/shape_corners.png", p6, width = 12, height = 6, dpi = 150)

# 5.7  circumference_vs_cells.png — circumference and perimeter vs. cells.
# Since circumference = perimeter for grid shapes, show both on one scatter
# to confirm the identity, along with corners.
p7 <- ggplot(patch_df, aes(x = cells, y = perimeter, label = name)) +
  geom_point(aes(size = corners), colour = "#1976D2", alpha = 0.7) +
  geom_text_repel(size = 2.5, max.overlaps = 50, seed = 42L) +
  labs(title = "Perimeter (= circumference) vs. cell count",
       subtitle = "Point size = total corners (convex + concave)",
       x = "Cell count",
       y = "Perimeter / Circumference (exposed edges)") +
  theme_minimal()
ggsave("analysis/output/circumference_vs_cells.png", p7, width = 9, height = 6, dpi = 150)

# 5.8  Confirm all PNG files are non-empty.
png_files <- c(
  "analysis/output/gain_per_time.png",
  "analysis/output/gain_curves.png",
  "analysis/output/gain_heatmap.png",
  "analysis/output/shape_density.png",
  "analysis/output/total_gain.png",
  "analysis/output/shape_corners.png",
  "analysis/output/circumference_vs_cells.png"
)
for (f in png_files) {
  stopifnot(file.exists(f) && file.info(f)$size > 0L)
}

cat("tile_analysis.R completed successfully.\n")
cat("Outputs written to analysis/output/:\n")
cat("  tile_summary.csv       (", nrow(summary_df), "rows )\n")
cat("  advance_breakeven.csv  (", nrow(breakeven_df), "rows )\n")
for (f in png_files) cat("  ", basename(f), " (", file.info(f)$size, "bytes )\n")
