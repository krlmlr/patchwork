#!/usr/bin/env Rscript
# Fairness analysis for stochastic (uniform-random) self-play batches.
#
# Reads a batch `game_summary` NDJSON file (produced by the C++ batch_driver in
# its default summary mode) and answers two questions about the simplified game
# played by two identical uniform-random agents:
#   1. Fairness: is there a structural first-player advantage? (P1 win rate and
#      mean score margin, each with a confidence interval).
#   2. Setup-dependence: how much of the outcome variance comes from the dealt
#      setup (between-setup) vs the luck of play (within-setup, agent RNG)?
#
# Usage:
#   Rscript analysis/fairness_analysis.R <batch.ndjson> [output_dir]
# Defaults: input  = analysis/output/fairness_batch.ndjson
#           output = analysis/output/
#
# R package dependencies beyond DESCRIPTION (Imports: yaml):
#   duckplyr — dplyr backend on DuckDB; reads NDJSON/Parquet and runs the
#              aggregation in-database
#   ggplot2  — plots
# Both are available in the project's PPM-backed toolchain.
#
# Data flow — as much computation as possible is pushed into DuckDB via duckplyr:
#   * The DuckDB `json` extension is installed and loaded (INSTALL then LOAD).
#   * The NDJSON is converted once to a typed Parquet cache (skipped when the
#     cache is newer than the NDJSON); re-runs read the Parquet directly.
#   * Frames use prudence = "stingy", so nothing is materialised into R
#     implicitly: every aggregate is computed in DuckDB and only the small
#     result tables (1 overall row, one row per setup, one row per margin bin)
#     are pulled back with collect(). The 10M per-game rows never enter R.
#   * Variance is derived from sd() (which duckplyr translates) because var()
#     does not translate and the dd$ escape hatch is absent in duckplyr 1.2.1.

suppressWarnings(suppressMessages({
  pkgload::load_all(quiet = TRUE)
  library(duckplyr)
  library(dplyr)
  library(ggplot2)
}))

args <- commandArgs(trailingOnly = TRUE)
input_path <- if (length(args) >= 1) {
  args[[1]]
} else {
  "analysis/output/fairness_batch.ndjson"
}
output_dir <- if (length(args) >= 2) args[[2]] else "analysis/output"
dir.create(output_dir, showWarnings = FALSE, recursive = TRUE)

if (!file.exists(input_path)) {
  stop(sprintf("batch summary file not found: %s", input_path))
}

# ── DuckDB json extension: unconditional INSTALL, then LOAD ───────────────────
duckplyr::db_exec("INSTALL json;")
duckplyr::db_exec("LOAD json;")

# ── One-time NDJSON -> Parquet conversion (cached) ────────────────────────────
# Read the NDJSON natively, keep only game_summary records, project the columns
# we need plus the derived margin, and write a typed Parquet cache. The cache is
# rebuilt only when it is missing or older than the NDJSON input.
parquet_path <- file.path(
  output_dir,
  paste0(tools::file_path_sans_ext(basename(input_path)), ".parquet")
)
stale <- !file.exists(parquet_path) ||
  file.mtime(parquet_path) < file.mtime(input_path)
if (stale) {
  message("converting NDJSON -> Parquet cache: ", parquet_path)
  read_json_duckdb(
    input_path,
    prudence = "stingy",
    options = list(format = "newline_delimited")
  ) |>
    filter(event == "game_summary") |>
    transmute(
      setup_id,
      seed,
      score_p0,
      score_p1,
      winner,
      plies,
      margin = score_p1 - score_p0
    ) |>
    compute_parquet(parquet_path)
  invisible(NULL)
}

games <- read_parquet_duckdb(parquet_path, prudence = "stingy")

# ── Aggregates computed in DuckDB; only small tables come back to R ───────────
overall <- games |>
  summarise(
    n = n(),
    p1_wins = sum(winner == 1L),
    margin_mean = mean(margin),
    margin_sd = sd(margin)
  ) |>
  collect()

n_games <- overall$n
if (n_games == 0) {
  stop("no games found in batch summary")
}

by_setup <- games |>
  summarise(
    n_games = n(),
    p1_wins = sum(winner == 1L),
    mean_margin = mean(margin),
    sd_margin = sd(margin),
    .by = setup_id
  ) |>
  arrange(setup_id) |>
  collect() |>
  mutate(
    p1_win_rate = p1_wins / n_games,
    var_margin = sd_margin^2
  )

# Score-margin histogram, binned in DuckDB (integer margins -> a few hundred
# rows) rather than pulling 10M values into R.
margin_hist <- games |>
  summarise(n = n(), .by = margin) |>
  arrange(margin) |>
  collect()

# ── First-player advantage: win rate + binomial CI, mean margin + CI ─────────
wr <- binom.test(overall$p1_wins, n_games, p = 0.5)
p1_win_rate <- unname(wr$estimate)
wr_ci <- wr$conf.int

margin_mean <- overall$margin_mean
margin_sd <- overall$margin_sd
margin_ci <- margin_mean +
  c(-1, 1) * qt(0.975, df = n_games - 1) * margin_sd / sqrt(n_games)

# ── Variance decomposition: between-setup vs within-setup ────────────────────
# Var(margin) = Var(E[margin | setup]) + E[Var(margin | setup)], n-weighted.
w <- by_setup$n_games / n_games
between_var <- sum(w * (by_setup$mean_margin - margin_mean)^2)
within_var <- sum(w * by_setup$var_margin)
total_var <- between_var + within_var
between_share <- if (total_var > 0) between_var / total_var else 0
within_share <- if (total_var > 0) within_var / total_var else 0

# ── Per-setup win-rate CIs (small table, computed in R) ───────────────────────
setup_ci <- lapply(seq_len(nrow(by_setup)), function(i) {
  bt <- binom.test(by_setup$p1_wins[i], by_setup$n_games[i], p = 0.5)
  data.frame(wr_lo = bt$conf.int[1], wr_hi = bt$conf.int[2])
})
setup_ci <- do.call(rbind, setup_ci)
by_setup$wr_lo <- setup_ci$wr_lo
by_setup$wr_hi <- setup_ci$wr_hi

# ── Per-setup seat bias: is the spread real, and how large? ───────────────────
# The grand-average win rate can sit at ~0.5 while individual setups are heavily
# seat-biased, because per-setup first-mover advantages cancel across setups.
setup_wr_sd <- sd(by_setup$p1_win_rate)
fair_se <- sqrt(0.25 / stats::median(by_setup$n_games))
signal_ratio <- if (fair_se > 0) setup_wr_sd / fair_se else NA_real_
real_bias_setups <- sum(by_setup$wr_lo > 0.5 | by_setup$wr_hi < 0.5)
dev <- abs(by_setup$p1_win_rate - 0.5)
frac_5545 <- mean(dev >= 0.05) # 55:45 or worse
frac_5248 <- mean(dev >= 0.02) # 52:48 or worse
max_bias <- max(dev)

# ── Committed per-setup table ────────────────────────────────────────────────
fairness_by_setup <- by_setup |>
  transmute(
    setup_id,
    n_games,
    p1_win_rate,
    wr_ci_lo = wr_lo,
    wr_ci_hi = wr_hi,
    mean_margin
  ) |>
  arrange(setup_id)
csv_path <- file.path(output_dir, "fairness_by_setup.csv")
write.csv(fairness_by_setup, csv_path, row.names = FALSE)

# ── Plots ─────────────────────────────────────────────────────────────────────
# Per-setup P1 win rate with CI, sorted; reference line at 0.5.
plot_setup <- by_setup |> arrange(p1_win_rate) |> mutate(rank = row_number())
p_wr <- ggplot(plot_setup, aes(x = rank, y = p1_win_rate)) +
  geom_hline(yintercept = 0.5, linetype = "dashed", colour = "grey40") +
  geom_errorbar(aes(ymin = wr_lo, ymax = wr_hi), width = 0, colour = "grey70") +
  geom_point(colour = "#2c7fb8", size = 1) +
  labs(
    title = "Player-1 win rate per setup (uniform-random self-play)",
    subtitle = sprintf(
      "%d setups, %d games each; dashed line = fair (0.5)",
      nrow(by_setup),
      by_setup$n_games[1]
    ),
    x = "setup (ranked by win rate)",
    y = "P1 win rate"
  ) +
  ylim(0, 1) +
  theme_minimal()
ggsave(
  file.path(output_dir, "fairness_winrate_by_setup.png"),
  p_wr,
  width = 8,
  height = 5,
  dpi = 120
)

# Score-margin distribution from the DuckDB-binned counts.
p_margin <- ggplot(margin_hist, aes(x = margin, y = n)) +
  geom_col(fill = "#2c7fb8", width = 1) +
  geom_vline(xintercept = 0, linetype = "dashed", colour = "grey40") +
  geom_vline(xintercept = margin_mean, colour = "#d95f0e") +
  labs(
    title = "Score margin (P1 - P0) distribution",
    subtitle = sprintf(
      "%d games; dashed = fair (0), orange = mean (%.3f)",
      n_games,
      margin_mean
    ),
    x = "score margin (P1 - P0)",
    y = "games"
  ) +
  theme_minimal()
ggsave(
  file.path(output_dir, "fairness_margin_distribution.png"),
  p_margin,
  width = 8,
  height = 5,
  dpi = 120
)

# ── Verdict ───────────────────────────────────────────────────────────────────
# Fair if the win-rate CI covers 0.5 AND the margin CI covers 0.
# Unfair if both intervals exclude their fair value and agree in sign.
# Inconclusive otherwise.
wr_covers_fair <- wr_ci[1] <= 0.5 && wr_ci[2] >= 0.5
margin_covers_fair <- margin_ci[1] <= 0 && margin_ci[2] >= 0
verdict <- if (wr_covers_fair && margin_covers_fair) {
  "FAIR"
} else if (
  !wr_covers_fair &&
    !margin_covers_fair &&
    sign(p1_win_rate - 0.5) == sign(margin_mean)
) {
  "UNFAIR"
} else {
  "INCONCLUSIVE"
}

# ── Report ────────────────────────────────────────────────────────────────────
cat("\n================  FAIRNESS ANALYSIS  ================\n")
cat(sprintf(
  "games:            %d across %d setups (%d per setup)\n",
  n_games,
  nrow(by_setup),
  by_setup$n_games[1]
))
cat(sprintf(
  "P1 win rate:      %.4f   95%% CI [%.4f, %.4f]\n",
  p1_win_rate,
  wr_ci[1],
  wr_ci[2]
))
cat(sprintf(
  "mean margin P1-P0:%.4f   95%% CI [%.4f, %.4f]\n",
  margin_mean,
  margin_ci[1],
  margin_ci[2]
))
cat(sprintf(
  "variance shares:  between-setup %.1f%%  |  within-setup %.1f%%\n",
  100 * between_share,
  100 * within_share
))
cat(sprintf(
  "  (between-setup var = %.4f, within-setup var = %.4f, total = %.4f)\n",
  between_var,
  within_var,
  total_var
))
cat(sprintf(
  "per-setup win rate: min %.3f  max %.3f  sd %.4f\n",
  min(by_setup$p1_win_rate),
  max(by_setup$p1_win_rate),
  setup_wr_sd
))
cat(sprintf(
  "per-setup seat bias: spread is %.1fx sampling noise (obs sd %.4f vs fair se %.4f)\n",
  signal_ratio,
  setup_wr_sd,
  fair_se
))
cat(sprintf(
  "  setups with real bias (95%% CI excludes 0.5): %d/%d; %.0f%% >= 55:45, %.0f%% >= 52:48; worst %.0f:%.0f\n",
  real_bias_setups,
  nrow(by_setup),
  100 * frac_5545,
  100 * frac_5248,
  100 * (0.5 + max_bias),
  100 * (0.5 - max_bias)
))
cat(sprintf("\nVERDICT: %s\n", verdict))
cat(sprintf(
  "  win-rate CI %s 0.5; margin CI %s 0\n",
  if (wr_covers_fair) "covers" else "excludes",
  if (margin_covers_fair) "covers" else "excludes"
))
# Effect size vs statistical significance: at very large n a practically
# negligible bias still becomes significant, so report the magnitude too.
cat(sprintf(
  "  effect size: P1 win rate %+.3f pp from 50%%; mean margin %+.4f pts (margin sd %.1f)\n",
  100 * (p1_win_rate - 0.5),
  margin_mean,
  margin_sd
))
cat("wrote:", csv_path, "\n")
cat("wrote:", parquet_path, "(Parquet cache)\n")
cat("wrote:", file.path(output_dir, "fairness_winrate_by_setup.png"), "\n")
cat("wrote:", file.path(output_dir, "fairness_margin_distribution.png"), "\n")
cat("=====================================================\n")
