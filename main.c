// Lab 4, uP2 Fall 2023
// Created: 2023-07-31
// Updated: 2023-08-01
// Lab 4 is intended to introduce you to more advanced RTOS concepts. In this, you will
// - implement blocking, yielding, sleeping
// - Thread priorities, aperiodic & periodic threads
// - IPC using FIFOs
// - Dynamic thread creation & deletion

/************************************Includes***************************************/

#include "G8RTOS/G8RTOS.h"
#include "./MultimodDrivers/multimod.h"
#include <stdint.h>
#include <driverlib/uartstdio.h>
#include "./threads.h"


// Complete the functions below as test threads.
void task0() {
    uint32_t counter0 = 0;
    while(1) {
        int32_t rslt = G8RTOS_WriteFIFO(0, counter0);
        G8RTOS_WaitSemaphore(&sem_UART);
        UARTprintf("Task 0 counter: %d\n", counter0);
        G8RTOS_SignalSemaphore(&sem_UART);
        counter0++;



        sleep(500);
        //SysCtlDelay(delay_000_1_s);
    }
}

void task1() {
    uint32_t counter1 = 0;
    while(1) {
            int32_t rslt = G8RTOS_ReadFIFO(0);
            G8RTOS_WaitSemaphore(&sem_UART);

            UARTprintf("Task 1 counter is at: %d and Task 0 counter (fifo):%d \n", counter1, rslt);
            G8RTOS_SignalSemaphore(&sem_UART);
            counter1++;
            sleep(1000);
            //SysCtlDelay(delay_000_1_s);
        }
}

void task2() {
    uint32_t counter2 = 0;
    while(1) {
            G8RTOS_WaitSemaphore(&sem_UART);
            UARTprintf("Task 2 counter is at: %d\n", counter2);
            G8RTOS_SignalSemaphore(&sem_UART);
            counter2++;
            sleep(1000);
            //SysCtlDelay(delay_000_1_s);
        }
}

void idle() {
    while(1);
}




/************************************MAIN*******************************************/
int main(void)
{
    // Sets clock speed to 80 MHz. You'll need it!
    SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
    G8RTOS_Init();
    multimod_init();

    // Add threads, semaphores, here
    G8RTOS_InitSemaphore(&sem_UART, 1);
    G8RTOS_AddThread(task0, 8, "task 0");
    G8RTOS_AddThread(task1, 0, "task 1");
    //G8RTOS_AddThread(task2, 3, "task 2");
    G8RTOS_AddThread(idle, 255, "idle");

    G8RTOS_InitFIFO(0);


    G8RTOS_Launch();
    while (1);
}

/************************************MAIN*******************************************/
