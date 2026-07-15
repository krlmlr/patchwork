#!/usr/bin/env python3
"""End-to-end checks for the batch_driver executable.

Invoked by Meson with the batch_driver and play_driver executable paths as
argv[1] and argv[2]. Exercises the engine spec scenarios that are defined at the
executable level: full-grid coverage, whole-batch reproducibility, order/size
independence of per-game seeds, replay of a recorded game through the single
play driver, full-mode structure, summary/full score agreement, and error
handling for invalid arguments.
"""
import json
import subprocess
import sys


def run(cmd):
    return subprocess.run(cmd, capture_output=True, text=True)


def records(stdout):
    return [json.loads(line) for line in stdout.splitlines() if line.strip()]


def main():
    batch, play = sys.argv[1], sys.argv[2]
    failures = []

    def check(cond, msg):
        if not cond:
            failures.append(msg)

    # 1. Full grid: 3 setups x 100 games = 300 summary records, all well-formed.
    r = run([batch, "--setups", "0,1,2", "--games", "100", "--master-seed", "42"])
    check(r.returncode == 0, f"grid run failed: {r.stderr}")
    recs = records(r.stdout)
    check(len(recs) == 300, f"expected 300 records, got {len(recs)}")
    fields = ("setup_id", "seed", "score_p0", "score_p1", "winner", "plies")
    for rec in recs:
        check(rec.get("event") == "game_summary", "record is not game_summary")
        check(all(k in rec for k in fields), f"missing fields in {rec}")
        check(rec["winner"] in (0, 1), f"invalid winner {rec.get('winner')}")
        check(rec["plies"] > 0, f"non-positive plies {rec.get('plies')}")

    # 2. Whole batch is byte-for-byte reproducible.
    r2 = run([batch, "--setups", "0,1,2", "--games", "100", "--master-seed", "42"])
    check(r2.stdout == r.stdout, "batch output is not reproducible")

    # 3. Per-game seed/outcome is independent of grid order and size: the first
    #    50 games of setup 2 must be identical whether run alone or in a bigger
    #    grid, and the seeds must match a direct derivation-free comparison.
    solo = run([batch, "--setups", "2", "--games", "50", "--master-seed", "42"])
    solo_recs = records(solo.stdout)
    big_setup2 = [rec for rec in recs if rec["setup_id"] == 2]
    for a, b in zip(big_setup2[:50], solo_recs):
        check(a == b, f"order/size dependence: {a} != {b}")

    # 4. Replay a recorded (setup_id, seed) through the single play driver.
    sample = recs[0]
    pr = run([play, "--seed", str(sample["seed"]), "--setup", str(sample["setup_id"])])
    check(pr.returncode == 0, f"play_driver replay failed: {pr.stderr}")
    end = records(pr.stdout)[-1]
    check(end.get("event") == "game_end", "replay did not end with game_end")
    check(end["score_p0"] == sample["score_p0"], "replay score_p0 mismatch")
    check(end["score_p1"] == sample["score_p1"], "replay score_p1 mismatch")
    check(end["winner"] == sample["winner"], "replay winner mismatch")

    # 5. Full mode emits exactly one game_start and one game_end per game.
    rf = run([batch, "--setups", "0", "--games", "3", "--master-seed", "1", "--full"])
    fl = records(rf.stdout)
    starts = [x for x in fl if x["event"] == "game_start"]
    ends = [x for x in fl if x["event"] == "game_end"]
    check(len(starts) == 3, f"expected 3 game_start, got {len(starts)}")
    check(len(ends) == 3, f"expected 3 game_end, got {len(ends)}")

    # 6. Summary scores equal full-mode game_end values for the same game.
    s0 = run([batch, "--setups", "0", "--games", "1", "--master-seed", "1"])
    srec = records(s0.stdout)[0]
    f0 = run([batch, "--setups", "0", "--games", "1", "--master-seed", "1", "--full"])
    fend = records(f0.stdout)[-1]
    for k in ("score_p0", "score_p1", "winner"):
        check(srec[k] == fend[k], f"summary/full {k} mismatch")

    # 7. Error handling: missing --master-seed, and a malformed --setups spec.
    err = run([batch, "--setups", "0", "--games", "1"])
    check(err.returncode != 0 and err.stderr, "missing master-seed should error")
    bad = run([batch, "--setups", "9-5", "--games", "1", "--master-seed", "1"])
    check(bad.returncode != 0 and bad.stderr, "descending range should error")

    if failures:
        for f in failures:
            print("FAIL:", f, file=sys.stderr)
        sys.exit(1)
    print("batch_cli_test OK")


if __name__ == "__main__":
    main()
