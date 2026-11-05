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


| Scenario | HIGH task blocking time |
|----------|-------------------------|
| 01 - semaphore (inversion) | 38000 µs_ |
| 02 - mutex (inheritance)   | _TODO µs_ |

Gantt of the inversion: _TODO (docs/inversion.png)_

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

- [ ] 01 - reproduce + measure the inversion (semaphore), toggle LED via DT
- [ ] 02 - fix with a mutex, measure the contrast
- [ ] 03 - deadlock + fix
- [ ] 04 - instrumentation + Gantt
- [ ] ztest assertions in tests/concurrency
- [ ] CI (GitHub Actions): build every app + run twister
- [ ] README write-up with measured numbers and captures

## License

MIT (or your choice).
