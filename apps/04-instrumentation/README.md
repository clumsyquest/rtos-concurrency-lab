# 04 — Instrumentation & Visualization

Instrumentation du scénario d'inversion de priorité à 3 threads (`LOW`, `MEDIUM`, `HIGH`) sous Zephyr RTOS (`qemu_cortex_m3`) avec logs horodatés par cycles, `thread_analyzer` et visualisation via `tools/gantt.py`.

## Contenu

- `src/main.c` : scénario de priorité avec sémaphore (`k_sem`) et émission d'événements au format `[<cycles>] <THREAD> <EVENT>`.
- `prj.conf` : activation du logging, `CONFIG_THREAD_ANALYZER` et `CONFIG_THREAD_RUNTIME_STATS` / `CONFIG_SCHED_THREAD_USAGE`.
- `../../tools/gantt.py` : parse les logs, découpe naturellement la préemption de `LOW` par `MEDIUM`, permet le zoom (`--start`, `--window`) et affiche les stats CPU (`thread_analyzer`).

## Compilation et capture (QEMU)

```bash
west build -p always -b qemu_cortex_m3 -d ~/rtos-concurrency-lab/apps/04-instrumentation/build ~/rtos-concurrency-lab/apps/04-instrumentation
west build -d ~/rtos-concurrency-lab/apps/04-instrumentation/build -t run | tee ~/rtos-concurrency-lab/apps/04-instrumentation/demo04.log
