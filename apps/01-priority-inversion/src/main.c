/*
 * Demo 01 - Priority inversion (unbounded), lock = SEMAPHORE (no inheritance).
 *
 * GOAL: reproduce AND measure the classic 3-task priority inversion, and make
 * it VISIBLE on a real GPIO/LED described in the device tree.
 *
 * Actors (you implement them):
 *   LOW  (lowest prio)  : takes the shared lock, does a LONG job, releases it.
 *   MED  (middle prio)  : does NOT touch the lock, just burns CPU.
 *   HIGH (highest prio) : after a delay, tries to take the lock and MEASURES
 *                         how long it was blocked. Toggles the LED while running.
 *
 * The lock is a binary k_sem (NO priority inheritance) -> MED preempts LOW while
 * LOW holds the lock -> HIGH is blocked indirectly by MED. Report that time.
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


LOG_MODULE_REGISTER(demo01, LOG_LEVEL_INF);

struct gpio_dt_spec led= GPIO_DT_SPEC_GET(LED,gpios);

K_SEM_DEFINE(s,1,1);

void L(void *p1, void *p2, void *p3){

  while(1){
	k_sem_take(&s,K_FOREVER);
	LOG_INF("L running");
	k_busy_wait(20000);
	LOG_INF("L gave s back");
	k_sem_give(&s);
	k_busy_wait(10000);
	LOG_INF("L sleeping");
	k_msleep(10);}
}

void  H(void *p1, void *p2, void *p3){
	int ret;


        ret= gpio_pin_configure_dt(&led,GPIO_OUTPUT_LOW);
	if (ret!=0){
	   LOG_INF("error: failed to configure pin->%d",led.pin);
	   return;
        }

  while(1){
	uint32_t t1= k_cycle_get_32();
	LOG_INF("H ready to take s");
	k_sem_take(&s,K_FOREVER);
	LOG_INF("H running");
	gpio_pin_set_dt(&led,1);
	k_sem_give(&s);
	LOG_INF("H gave s back");
	uint32_t t2= k_cycle_get_32();
	gpio_pin_set_dt(&led,0);
	uint32_t delay_us= cycles_to_us(t2-t1);
	LOG_INF("elasped time: %u",delay_us);
	LOG_INF("H sleeping");
	k_msleep(10);
	}
}

void M(void *p1, void *p2, void *p3){
     while(1){
	LOG_INF("M runnig");
	k_busy_wait(10000);
	LOG_INF("M sleeping");
	k_msleep(10);}
}

K_THREAD_DEFINE(LOW_ID,STACKSIZE,L,NULL,NULL,NULL,LOW_PRIORITY,0,0);
K_THREAD_DEFINE(MED_ID,STACKSIZE,M,NULL,NULL,NULL,MED_PRIORITY,0,5);
K_THREAD_DEFINE(HIGH_ID,STACKSIZE,H,NULL,NULL,NULL,HIGH_PRIORITY,0,2);


int main(void)
{
    LOG_INF("Demo 01 - priority inversion (semaphore lock)");
    return 0;
}
