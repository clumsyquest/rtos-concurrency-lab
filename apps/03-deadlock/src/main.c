/*
 * Demo 03 - DEADLOCK with two mutexes, then the fix.
 *
 * Two threads, two mutexes (m1, m2):
 *   A: lock m1 -> (delay) -> lock m2
 *   B: lock m2 -> (delay) -> lock m1
 * Interleave them and both block forever -> deadlock.
 *
 * GOAL: show the deadlock (logs stop progressing), then FIX it by enforcing a
 *       global lock ordering (always take m1 before m2), or by using a timeout
 *       on k_mutex_lock and backing off.
 *
 * TODO
 *  [ ] Create m1, m2 and threads A, B with the crossed locking order.
 *  [ ] Add logs before/after each lock so the freeze is visible.
 *  [ ] Then implement the fix (same lock order, or timeout + retry) and show
 *      the system no longer deadlocks.
 *
 * Useful APIs: k_mutex_init, k_mutex_lock(&m, K_MSEC(x)) returns -EAGAIN on
 *              timeout, k_mutex_unlock, k_msleep().
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

int main(void)
{
    printk("Demo 03 - deadlock (two mutexes)\n");
    /* TODO: create m1, m2 and threads A, B here. */
    return 0;
}
