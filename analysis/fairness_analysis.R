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
#   duckdb   — in-process database that reads the NDJSON and extracts fields
#   ggplot2  — plots
#   dplyr    — per-setup aggregation
# All are available in the project's PPM-backed toolchain.
#
# NDJSON is read entirely inside DuckDB: each line is scanned as one VARCHAR and
# the game_summary fields are pulled out with core regexp_extract. This avoids
# the DuckDB json extension (which needs network access to install) and scales
# to tens of millions of records (10M rows in ~5s vs minutes for a row-by-row
# JSON parse in R).

suppressWarnings(suppressMessages({
  pkgload::load_all(quiet = TRUE)
  library(duckdb)
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

# ── Ingest into DuckDB ───────────────────────────────────────────────────────
# Read each NDJSON line as a single VARCHAR (delimiter = a byte that never
# appears in the data, quoting disabled), keep only game_summary records, and
# extract each field with DuckDB's core regexp_extract. The per-game score
# margin (P1 − P0) is derived in SQL.
con <- dbConnect(duckdb::duckdb())
on.exit(dbDisconnect(con, shutdown = TRUE), add = TRUE)

field <- function(key, sign = "-?") {
  sprintf(
    "CAST(regexp_extract(line, %s, 1) AS BIGINT) AS %s",
    dbQuoteString(con, sprintf('"%s":(%s[0-9]+)', key, sign)), key
  )
}
read_lines_sql <- sprintf(
  "read_csv(%s, columns={'line': 'VARCHAR'}, delim=%s, header=false, quote=%s)",
  dbQuoteString(con, normalizePath(input_path)),
  dbQuoteString(con, "\a"),  # bell byte — never present in the NDJSON
  dbQuoteString(con, "")
)
inner_sql <- sprintf(
  "SELECT %s FROM %s WHERE line LIKE %s",
  paste(
    field("setup_id"), field("seed", sign = ""), field("score_p0"),
    field("score_p1"), field("winner"), field("plies", sign = ""),
    sep = ", "
  ),
  read_lines_sql,
  dbQuoteString(con, "%\"event\":\"game_summary\"%")
)
games <- dbGetQuery(con, sprintf(
  "SELECT setup_id, seed, score_p0, score_p1, winner, plies,
          (score_p1 - score_p0) AS margin
   FROM (%s) s
   ORDER BY setup_id, seed",
  inner_sql
))

n_games <- nrow(games)
if (n_games == 0) {
  stop("no games found in batch summary")
}

# ── First-player advantage: win rate + binomial CI, mean margin + CI ─────────
p1_wins <- sum(games$winner == 1L)
wr <- binom.test(p1_wins, n_games, p = 0.5)
p1_win_rate <- unname(wr$estimate)
wr_ci <- wr$conf.int

margin_mean <- mean(games$margin)
margin_tt <- t.test(games$margin, mu = 0)
margin_ci <- margin_tt$conf.int

# ── Variance decomposition: between-setup vs within-setup ────────────────────
# Total variance of the margin splits into the variance of per-setup mean
# margins (between) and the mean of per-setup variances (within). Uses the
# population identity Var(X) = Var(E[X|g]) + E[Var(X|g)] with n-weighting.
by_setup <- games %>%
  group_by(setup_id) %>%
  summarise(
    n_games = n(),
    p1_win_rate = mean(winner == 1L),
    mean_margin = mean(margin),
    var_margin = if (n() > 1) var(margin) else 0,
    .groups = "drop"
  )

grand_mean <- margin_mean
w <- by_setup$n_games / n_games
between_var <- sum(w * (by_setup$mean_margin - grand_mean)^2)
within_var <- sum(w * by_setup$var_margin)
total_var <- between_var + within_var
between_share <- if (total_var > 0) between_var / total_var else 0
within_share <- if (total_var > 0) within_var / total_var else 0

# Per-setup win-rate CIs for the committed table.
setup_ci <- lapply(seq_len(nrow(by_setup)), function(i) {
  bt <- binom.test(
    round(by_setup$p1_win_rate[i] * by_setup$n_games[i]),
    by_setup$n_games[i],
    p = 0.5
  )
  data.frame(wr_lo = bt$conf.int[1], wr_hi = bt$conf.int[2])
})
setup_ci <- do.call(rbind, setup_ci)
by_setup$wr_lo <- setup_ci$wr_lo
by_setup$wr_hi <- setup_ci$wr_hi

# ── Committed per-setup table ────────────────────────────────────────────────
fairness_by_setup <- by_setup %>%
  transmute(
    setup_id,
    n_games,
    p1_win_rate,
    wr_ci_lo = wr_lo,
    wr_ci_hi = wr_hi,
    mean_margin
  ) %>%
  arrange(setup_id)
csv_path <- file.path(output_dir, "fairness_by_setup.csv")
write.csv(fairness_by_setup, csv_path, row.names = FALSE)

# ── Plots ─────────────────────────────────────────────────────────────────────
# Per-setup P1 win rate with CI, sorted; reference line at 0.5.
plot_setup <- by_setup %>% arrange(p1_win_rate) %>% mutate(rank = row_number())
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

# Score-margin distribution across all games; reference line at 0.
p_margin <- ggplot(games, aes(x = margin)) +
  geom_histogram(binwidth = 1, fill = "#2c7fb8", colour = "white") +
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
  sd(by_setup$p1_win_rate)
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
  100 * (p1_win_rate - 0.5), margin_mean, sd(games$margin)
))
cat("wrote:", csv_path, "\n")
cat("wrote:", file.path(output_dir, "fairness_winrate_by_setup.png"), "\n")
cat("wrote:", file.path(output_dir, "fairness_margin_distribution.png"), "\n")
cat("=====================================================\n")
