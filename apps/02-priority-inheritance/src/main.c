/*
 * Demo 02 - Fix via PRIORITY INHERITANCE, lock = MUTEX.
 *
 * Same 3-task scenario as Demo 01, but the lock is now a k_mutex.
 * Zephyr mutexes implement priority inheritance: while HIGH waits on the
 * mutex held by LOW, LOW is temporarily boosted to HIGH's priority, finishes
 * fast, releases -> HIGH's blocking time collapses.
 *
 * GOAL: measure HIGH's blocking time again and COMPARE with Demo 01.
 *       The before/after contrast is your proof.
 *
 * TODO
 *  [ ] Copy your Demo 01 structure.
 *  [ ] Replace the k_sem lock by a k_mutex (k_mutex_init / lock / unlock).
 *  [ ] Re-measure HIGH's wait and print it.
 *  [ ] Record both numbers in the README results table.
 *
 * Useful APIs: k_mutex_init, k_mutex_lock(&m, K_FOREVER), k_mutex_unlock(&m).
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

int main(void)
{
    printk("Demo 02 - priority inheritance (mutex lock)\n");
    /* TODO: init your mutex and start your threads here. */
    return 0;
}
