/*
 * ztest suite for the lab (runs on qemu_cortex_m3 via twister -> CI).
 *
 * Keep tests host/QEMU-friendly: test pure helpers and lock logic you can
 * exercise deterministically (e.g. cycles_to_us(), a lock-ordering helper).
 *
 * Run locally:
 *   west twister -T tests/concurrency -p qemu_cortex_m3
 *
 * TODO
 *  [ ] Replace the placeholder with real assertions.
 *  [ ] Add tests that would catch a regression in your lock/ordering code.
 */

#include <zephyr/ztest.h>
#include "timing.h"

ZTEST_SUITE(concurrency, NULL, NULL, NULL, NULL, NULL);

ZTEST(concurrency, test_cycles_to_us_zero)
{
    /* TODO: real tests. Placeholder so the suite builds and runs in CI. */
    zassert_equal(cycles_to_us(0), 0u, "0 cycles must be 0 us");
}
