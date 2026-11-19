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
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <zephyr/devicetree.h>
#include "timing.h"  // cycles_to_us(); enable the include dir in CMakeLists */

#define LED DT_ALIAS(led0)
#define STACKSIZE 1024
#define LOW_PRIORITY 7
#define MED_PRIORITY 5
#define HIGH_PRIORITY 3


LOG_MODULE_REGISTER(demo04, LOG_LEVEL_INF);

//struct gpio_dt_spec led= GPIO_DT_SPEC_GET(LED,gpios);

K_SEM_DEFINE(my_sem,1,1);

void thread_low(void *p1, void *p2, void *p3){

  while(1){

        LOG_INF("[%u] LOW START", k_cycle_get_32());
        LOG_INF("[%u] LOW WAIT_SEM", k_cycle_get_32());

        //LOW taking de semaphore
        k_sem_take(&my_sem,K_FOREVER);
        LOG_INF("[%u] LOW GOT_SEM", k_cycle_get_32());

        //critical work during wich HIGH and MEDIUM will arrive
        k_busy_wait(30000);

        //LOW releasing the semaphore
        LOG_INF("[%u] LOW REL_SEM", k_cycle_get_32());
        k_sem_give(&my_sem);

        //LOW finish the rest of his work avec releasing the semaphore
        LOG_INF("[%u] LOW KEEP_WORKING", k_cycle_get_32());
        k_busy_wait(10000);

        LOG_INF("[%u] LOW END", k_cycle_get_32());

        k_sleep(K_MSEC(60));
        }
}

void  thread_high(void *p1, void *p2, void *p3){
/*      int ret;


        ret= gpio_pin_configure_dt(&led,GPIO_OUTPUT_LOW);
        if (ret!=0){
           LOG_INF("error: failed to configure pin->%d",led.pin);
           return;
        }
*/
  while(1){

        /*starts before thread_medium but after thread_low*/
        k_sleep(K_MSEC(10));

        LOG_INF("[%u] HIGH START", k_cycle_get_32());
        uint32_t t1= k_cycle_get_32();

        LOG_INF("[%u] HIGH WAIT_SEM", k_cycle_get_32());
        k_sem_take(&my_sem,K_FOREVER);
        uint32_t t2= k_cycle_get_32();

        LOG_INF("[%u] HIGH GOT_SEM", k_cycle_get_32());
  //      gpio_pin_set_dt(&led,1);
        k_busy_wait(10000);

        LOG_INF("[%u] HIGH REL_SEM", k_cycle_get_32());
        k_sem_give(&my_sem);

    //    gpio_pin_set_dt(&led,0);
        uint32_t delay_us= cycles_to_us(t2-t1);
        LOG_INF("elasped time: %u",delay_us);
        LOG_INF("[%u] HIGH END", k_cycle_get_32());

        k_sleep(K_MSEC(60));
        }
}

void thread_medium(void *p1, void *p2, void *p3){
     while(1){
        k_sleep(K_MSEC(15));

        LOG_INF("[%u] MEDIUM START", k_cycle_get_32());
        k_busy_wait(10000);
        LOG_INF("[%u] MEDIUM END", k_cycle_get_32());

        k_sleep(K_MSEC(60));
        }
}

K_THREAD_DEFINE(LOW_ID,STACKSIZE,thread_low,NULL,NULL,NULL,LOW_PRIORITY,0,0);
K_THREAD_DEFINE(MED_ID,STACKSIZE,thread_medium,NULL,NULL,NULL,MED_PRIORITY,0,0);
K_THREAD_DEFINE(HIGH_ID,STACKSIZE,thread_high,NULL,NULL,NULL,HIGH_PRIORITY,0,0);

int main(void)
{
    LOG_INF("Demo 04 - instrumentation");
    return 0;
}
