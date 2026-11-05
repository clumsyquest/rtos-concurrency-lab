/*
 * Small timing helper shared by the demos.
 * Only an arithmetic convenience: WHERE you measure is up to you.
 */
#ifndef RTOS_LAB_TIMING_H
#define RTOS_LAB_TIMING_H

#include <zephyr/kernel.h>

/* Convert a cycle delta (from k_cycle_get_32) into microseconds. */
static inline uint32_t cycles_to_us(uint32_t cycles)
{
    return (uint32_t)(((uint64_t)cycles * 1000000U)
                      / sys_clock_hw_cycles_per_sec());
}

#endif /* RTOS_LAB_TIMING_H */
