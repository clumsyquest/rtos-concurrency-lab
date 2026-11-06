# rtos-concurrency-lab

A hands-on lab, on **Zephyr RTOS** running on an **ARM Cortex-M** board, that
**reproduces, measures and fixes** the classic real-time concurrency hazards:
priority inversion, priority inheritance, and deadlock — then makes them
**visible** (GPIO/LED + a Gantt timeline) and **tested** (ztest in CI).

Everything happens **inside a single microcontroller**: several threads compete
for a shared resource behind a lock.

> Target board: **ST Nucleo-F401RE** (`nucleo_f401re`). Also builds on
> **QEMU** (`qemu_cortex_m3`) for CI without hardware.

## What it exercises (Zephyr pillars)

- **Kernel / RTOS** — threads, scheduler, mutex/semaphore, priority inversion/inheritance, deadlock
- **Device Tree** — a `boards/nucleo_f401re.overlay` exposes the LED/GPIO
- **Driver model** — the LED is driven through the GPIO API from devicetree
- **Kconfig** — per-app `prj.conf`
- **Logging subsystem** — structured logs via `LOG_*`
- **Testing** — `ztest` suite run by **twister** (feeds CI)

Networking, BLE, power management are intentionally out of scope (their own worlds).

## Demos

| # | App | What it shows |
|---|-----|---------------|
| 01 | `apps/01-priority-inversion`   | Unbounded priority inversion with a **semaphore** lock |
| 02 | `apps/02-priority-inheritance` | The fix: a **mutex** (priority inheritance) collapses the blocking time |
| 03 | `apps/03-deadlock`             | A two-mutex **deadlock**, then the fix (lock ordering / timeout) |
| 04 | `apps/04-instrumentation`      | Thread analyzer + logs -> **Gantt visualization** |

## Build, flash, test

```sh
# from the repo root, with your Zephyr/west environment active
west build -b nucleo_f401re apps/01-priority-inversion
west flash --runner openocd          # ST-Link attached to WSL via usbipd
picocom -b 115200 /dev/ttyACM0       # read the console

# no hardware (also what CI runs):
west build -b qemu_cortex_m3 apps/01-priority-inversion -t run

# unit tests via twister:
west twister -T tests/concurrency -p qemu_cortex_m3
```

## Results (fill with YOUR measurements)

### Demo 01 — Priority inversion (semaphore)

Measured on QEMU (`qemu_cortex_m3`) and on hardware (`nucleo_f401re`):

| Target | HIGH task blocking time |
|--------|-------------------------|
| QEMU (`qemu_cortex_m3`) | ~12–22 ms (jittery) |
| Hardware (`nucleo_f401re`) | ~14 ms typical, up to **~38 ms** (deterministic) |

The log sequence `H ready to take s → M running → L gave s back → H running`
shows HIGH being blocked by MED (which never uses the lock) — the signature of
priority inversion. The delay is bounded only by how long MED runs: the
"unbounded" nature of the problem.

Note: on real hardware the blocking time is deterministic to the microsecond
(repeated ~14.41 ms / ~38.13 ms), while QEMU jitters — which is why real-time
behaviour must be validated on target, not only in emulation.


### Demo 02 — Priority inheritance (mutex)

Same 3-task scenario as demo 01, but the lock is a `k_mutex` instead of a binary
semaphore. Zephyr mutexes implement **priority inheritance**.

**Result — the inversion is fixed.** When HIGH blocks on the mutex held by LOW,
the kernel boosts LOW to HIGH's priority, so LOW outranks MED, finishes its
critical section and releases at once. MED can no longer interpose between HIGH
requesting the lock and HIGH acquiring it.

| Scenario | HIGH blocking time | MED interposes? |
|----------|--------------------|-----------------|
| Demo 01 (semaphore)        | ~14–38 ms, unbounded | **yes** |
| Demo 02 (mutex), QEMU      | ~1–3 ms              | no |
| Demo 02 (mutex), hardware  | ~14–33 ms (see note) | no |

Hardware log (nucleo_f401re) — MED never runs between `H ready` and `H running`:

​```text
L running
H ready to take s
L gave s back       # LOW, boosted to HIGH's priority, releases immediately
H running           # no MED in between (compare demo 01)
​```

**Measurement note.** On hardware the absolute `elapsed time` is inflated by
~14 ms: three `LOG_INF` calls sit inside the measured window and synchronous
logging over the 115200-baud UART is slow. QEMU (instant console) shows the true
cost (~1–3 ms). The hardware log is kept as behavioural proof that MED no longer
interposes; the remaining worst case (~33 ms) is LOW's critical section, now
**bounded** — not MED interference. A cleaner measurement moves logging outside
the t1/t2 window.

### Demo 03 — Deadlock (two mutexes) and its fixes

Two tasks, two mutexes, acquired in **crossed order**: A locks m1 then m2,
B locks m2 then m1. A short sleep after the first lock lets both grab their
first mutex, then each waits on the other's — a **circular wait**. A low-priority
heartbeat task keeps printing, proving the system is not crashed: A and B are
logically stuck.

**Deadlock reproduced:**

​```text
A took m1
B took m2
A waiting for m2
B waiting for m1
app: still alive
app: still alive
...            # A and B frozen forever; only the heartbeat runs
​```

A deadlock needs all four **Coffman conditions**: mutual exclusion, hold-and-wait,
no preemption, circular wait. Each fix below breaks one of them.

**Fix 1 — lock ordering (canonical).** Everyone acquires locks in the same global
order (always m1 before m2). No task ever holds m2 while requesting m1, so the
circular wait is impossible. Clean, no churn. This is the fix used in the repo.

**Fix 2 — timeout + backoff (alternative).** Keep the crossed order, but take the
second lock with a timeout (`k_mutex_lock(&m, K_MSEC(1))`). On `-EAGAIN`, release
the lock already held and retry. This breaks hold-and-wait:

​```text
A took m1
B took m2
A waiting for m2
A failed to take m2     # timeout -> A releases m1 and retries
B took m1
...                     # progress continues, never freezes
​```

Trade-off: pure backoff can **livelock** if both tasks retry in lockstep;
real code adds **randomised (jittered) backoff** to desynchronise them.

## Repo layout

```
apps/            one standalone Zephyr app per demo (CMakeLists.txt + prj.conf + src/)
boards/          nucleo_f401re.overlay : device tree overlay (LED/GPIO)
common/          shared helpers (timing.h)
tests/           ztest suite (run by twister, used in CI)
tools/           gantt.py : serial logs -> Gantt chart
docs/            captures (Gantt PNGs, logs, photos)
.github/workflows/  CI (build on qemu_cortex_m3 + nucleo_f401re, run twister)
```

## Roadmap

- [Done ] 01 - reproduce + measure the inversion (semaphore), toggle LED via DT
- [Done ] 02 - fix with a mutex, measure the contrast
- [ ] 03 - deadlock + fix
- [ ] 04 - instrumentation + Gantt
- [ ] ztest assertions in tests/concurrency
- [ ] CI (GitHub Actions): build every app + run twister
- [ ] README write-up with measured numbers and captures

## License

MIT (or your choice).
