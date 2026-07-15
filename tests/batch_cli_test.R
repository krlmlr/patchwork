#!/usr/bin/env Rscript
# End-to-end checks for the batch_driver executable.
#
# Invoked by Meson with the batch_driver and play_driver executable paths as the
# two trailing arguments. Exercises the engine spec scenarios that are defined at
# the executable level: full-grid coverage, whole-batch reproducibility,
# order/size independence of per-game seeds, replay of a recorded game through
# the single play driver, full-mode structure, summary/full score agreement, and
# error handling for invalid arguments.
#
# Written in R (not Python) to match the project's tooling: R is the analysis and
# code-generation language and is already a CI dependency; Python is not used
# anywhere else in the repo.

suppressWarnings(suppressMessages(library(jsonlite)))

args <- commandArgs(trailingOnly = TRUE)
batch <- args[[1]]
play <- args[[2]]

run <- function(exe, exe_args) {
  out <- tempfile()
  err <- tempfile()
  on.exit(unlink(c(out, err)))
  status <- system2(exe, exe_args, stdout = out, stderr = err)
  list(
    status = status,
    stdout = readLines(out, warn = FALSE),
    stderr = readLines(err, warn = FALSE)
  )
}

records <- function(lines) {
  lines <- lines[nzchar(trimws(lines))]
  lapply(lines, jsonlite::fromJSON)
}

failures <- character(0)
check <- function(cond, msg) {
  if (!isTRUE(cond)) failures <<- c(failures, msg)
}

# 1. Full grid: 3 setups x 100 games = 300 well-formed summary records.
r <- run(batch, c("--setups", "0,1,2", "--games", "100", "--master-seed", "42"))
check(r$status == 0, paste("grid run failed:", paste(r$stderr, collapse = " ")))
recs <- records(r$stdout)
check(length(recs) == 300, sprintf("expected 300 records, got %d", length(recs)))
fields <- c("setup_id", "seed", "score_p0", "score_p1", "winner", "plies")
for (rec in recs) {
  check(identical(rec$event, "game_summary"), "record is not game_summary")
  check(all(fields %in% names(rec)), "missing summary fields")
  check(rec$winner %in% c(0, 1), sprintf("invalid winner %s", rec$winner))
  check(rec$plies > 0, "non-positive plies")
}

# 2. Whole batch is byte-for-byte reproducible.
r2 <- run(batch, c("--setups", "0,1,2", "--games", "100", "--master-seed", "42"))
check(identical(r2$stdout, r$stdout), "batch output is not reproducible")

# 3. Per-game seed/outcome is independent of grid order and size: the first 50
#    games of setup 2 are identical whether run alone or inside a bigger grid.
solo <- run(batch, c("--setups", "2", "--games", "50", "--master-seed", "42"))
big_setup2 <- r$stdout[vapply(recs, function(x) x$setup_id == 2, logical(1))]
check(identical(big_setup2[1:50], solo$stdout[1:50]),
      "per-game seed/outcome depends on batch order or size")

# 4. Replay a recorded (setup_id, seed) through the single play driver.
sample <- recs[[1]]
pr <- run(play, c("--seed", as.character(sample$seed),
                  "--setup", as.character(sample$setup_id)))
check(pr$status == 0, paste("play_driver replay failed:", paste(pr$stderr, collapse = " ")))
pr_recs <- records(pr$stdout)
end <- pr_recs[[length(pr_recs)]]
check(identical(end$event, "game_end"), "replay did not end with game_end")
check(end$score_p0 == sample$score_p0, "replay score_p0 mismatch")
check(end$score_p1 == sample$score_p1, "replay score_p1 mismatch")
check(end$winner == sample$winner, "replay winner mismatch")

# 5. Full mode emits exactly one game_start and one game_end per game.
rf <- run(batch, c("--setups", "0", "--games", "3", "--master-seed", "1", "--full"))
fl <- records(rf$stdout)
events <- vapply(fl, function(x) x$event, character(1))
check(sum(events == "game_start") == 3, "expected 3 game_start")
check(sum(events == "game_end") == 3, "expected 3 game_end")

# 6. Summary scores equal full-mode game_end values for the same game.
s0 <- run(batch, c("--setups", "0", "--games", "1", "--master-seed", "1"))
srec <- records(s0$stdout)[[1]]
f0 <- run(batch, c("--setups", "0", "--games", "1", "--master-seed", "1", "--full"))
f0_recs <- records(f0$stdout)
fend <- f0_recs[[length(f0_recs)]]
for (k in c("score_p0", "score_p1", "winner")) {
  check(srec[[k]] == fend[[k]], sprintf("summary/full %s mismatch", k))
}

# 7. Error handling: missing --master-seed, and a malformed --setups spec.
err_missing <- run(batch, c("--setups", "0", "--games", "1"))
check(err_missing$status != 0 && length(err_missing$stderr) > 0,
      "missing master-seed should error")
err_bad <- run(batch, c("--setups", "9-5", "--games", "1", "--master-seed", "1"))
check(err_bad$status != 0 && length(err_bad$stderr) > 0,
      "descending range should error")

if (length(failures) > 0) {
  for (f in failures) message("FAIL: ", f)
  quit(status = 1)
}
cat("batch_cli_test OK\n")
