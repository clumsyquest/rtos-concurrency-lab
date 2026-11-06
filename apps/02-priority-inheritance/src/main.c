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
 * Useful APIs: k_mutex_init, k_mutex_lock(&m, K_FOREVER), k_mutex_unlock(&m).
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <zephyr/devicetree.h>
#include "timing.h"

#define LED DT_ALIAS(led0)
#define STACKSIZE 1024
#define LOW_PRIORITY 7
#define MED_PRIORITY 5
#define HIGH_PRIORITY 3


LOG_MODULE_REGISTER(demo02, LOG_LEVEL_INF);

struct gpio_dt_spec led= GPIO_DT_SPEC_GET(LED,gpios);

K_MUTEX_DEFINE(s);

void L(void *p1, void *p2, void *p3){

  while(1){
        k_mutex_lock(&s,K_FOREVER);
        LOG_INF("L running");
        k_busy_wait(20000);
        LOG_INF("L gave s back");
        k_mutex_unlock(&s);
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
        k_mutex_lock(&s,K_FOREVER);
        LOG_INF("H running");
        gpio_pin_set_dt(&led,1);
        k_mutex_unlock(&s);
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
