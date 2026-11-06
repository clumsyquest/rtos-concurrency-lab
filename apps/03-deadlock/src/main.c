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
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#define STACKSIZE 1024


K_THREAD_STACK_DEFINE(stackA,STACKSIZE);
K_THREAD_STACK_DEFINE(stackB,STACKSIZE);
K_THREAD_STACK_DEFINE(stackC,STACKSIZE);

struct k_thread threadA,threadB, threadC;
struct k_mutex m1,m2;

void A(void *p1, void *p2, void *p3){

while(1){
        k_mutex_lock(&m1,K_FOREVER);
	printk("A took m1\n");
	k_msleep(10);
        printk("A waiting for m2\n");
	k_mutex_lock(&m2,K_FOREVER);
	printk("A took m2\n");
        k_msleep(10);
	k_mutex_unlock(&m2);
	printk("A freed m2\n");
	k_mutex_unlock(&m1);
	printk("A freed m1\n");
	k_msleep(10);}

}

void B(void *p1, void *p2, void *p3){

while(1){
        k_mutex_lock(&m1,K_FOREVER);
        printk("B took m1\n");
        k_msleep(15);
        printk("B waiting for m2\n");
        k_mutex_lock(&m2,K_FOREVER);
        printk("B took m2\n");
        k_msleep(10);
        k_mutex_unlock(&m2);
        printk("B freed m2\n");
        k_mutex_unlock(&m1);
        printk("B freed m1\n");
        k_msleep(10);}

}


void C(void *p1, void *p2, void *p3){
while(1){
	printk("app: still alive\n");
	k_msleep(10);
}
}



int main(void)
{

    printk("Demo 03 - deadlock (two mutexes)\n");

    k_mutex_init(&m1);
    k_mutex_init(&m2);

    k_thread_create(&threadA,stackA,K_THREAD_STACK_SIZEOF(stackA),A,NULL,NULL,NULL,3,0,K_NO_WAIT);
    k_thread_create(&threadB,stackB,K_THREAD_STACK_SIZEOF(stackB),B,NULL,NULL,NULL,5,0,K_NO_WAIT);
    k_thread_create(&threadC,stackC,K_THREAD_STACK_SIZEOF(stackC),C,NULL,NULL,NULL,7,0,K_NO_WAIT);
    return 0;
}
