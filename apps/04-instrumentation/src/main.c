/*
 * Demo 04 - INSTRUMENTATION & visualization.
 *
 * Reuse a previous scenario, but focus on OBSERVING it:
 *   - CONFIG_THREAD_ANALYZER prints per-thread stack usage + CPU runtime.
 *   - Logging subsystem (LOG_*) emits structured timestamped events.
 *   - tools/gantt.py turns those logs into a timeline.
 *
 * GOAL: produce the data that makes the inversion VISIBLE (Gantt) and
 *       quantified (CPU time per thread).
 *
 * TODO
 *  [ ] Start your threads (e.g. the Demo 01 scenario).
 *  [ ] Emit one LOG line per scheduling event: "[<cycles>] <THREAD> <event>".
 *  [ ] Capture the serial output, feed it to tools/gantt.py.
 *  [ ] (Optional) call thread_analyzer_print() yourself instead of AUTO.
 *
 * Useful: CONFIG_THREAD_ANALYZER*, k_thread_runtime_stats_get(),
 *         LOG_MODULE_REGISTER, LOG_INF(), k_cycle_get_32().
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(demo04, LOG_LEVEL_INF);

int main(void)
{
    LOG_INF("Demo 04 - instrumentation");
    /* TODO: start threads and emit structured logs here. */
    return 0;
}
