#!/usr/bin/env python3
"""
Gantt timeline from the lab's serial logs.

Expected log line format (YOU define it in your firmware), e.g.:
    [<cycles>] <THREAD_NAME> <EVENT>
    [123456] HIGH wait_lock
    [123980] LOW  release_lock

Read such lines (from a file or stdin), build per-thread intervals, and
render a Gantt chart so the priority inversion becomes visible to the eye.

Usage (once implemented):
    picocom -b 115200 /dev/ttyACM0 | tee run.log
    python3 tools/gantt.py run.log

TODO
  [ ] parse the log lines into (thread, event, cycles)
  [ ] turn consecutive events into intervals per thread
  [ ] plot with matplotlib (broken_barh is handy for Gantt bars)
"""

def main() -> None:
    # TODO: implement parsing + plotting
    raise SystemExit("gantt.py: not implemented yet - this is your part.")


if __name__ == "__main__":
    main()
