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
#include <driverlib/interrupt.h>
#include "./threads.h"

extern uint32_t SystemTime;


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
    }
}

void task1() {
    uint32_t counter1 = 0;
    while(1) {
            int32_t rslt = G8RTOS_ReadFIFO(0);
            G8RTOS_WaitSemaphore(&sem_UART);

            UARTprintf("Task 1 counter is at: %d, task 0 counter (fifo):%d \n", counter1, rslt);
            G8RTOS_SignalSemaphore(&sem_UART);
            counter1++;
            sleep(1000);
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
        }
}

void aperiodic_task() {
    GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_0);
    static uint32_t aperiodic_count = 0;
    UARTprintf("Aperiodic counter is at: %d\n", aperiodic_count);

    aperiodic_count++;
}

void periodic_task() {
    static uint32_t periodic_count = 0;
    G8RTOS_WaitSemaphore(&sem_UART);
    UARTprintf("This is a periodic counter is at: %d\n", periodic_count);

    G8RTOS_SignalSemaphore(&sem_UART);
    periodic_count++;
}

void idle() {
    while(1);
}




/************************************MAIN*******************************************/
int main(void)
{
    // Sets clock speed to 80 MHz. You'll need it!
    SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);


    // Init button+interrupt stuff
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    //SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R5; // here we can use either one;
    // Enable relevant port for launchpad switches
    //GPIO_PORTF_DEN_R |= 0x00000001; // digital enable PF0
    //GPIO_PORTF_DEN_R |= 0x00000010; // digital enable PF4
    // set up pull up resistors
    //GPIO_PORTF_PUR_R |= 0x00000010;
    // Configure SW1 input
    //GPIO_PORTF_DIR_R |= ~0x00000011; // set pins 0 and 4 as inputs

    //GPIO_PORTF_LOCK_R = GPIO_LOCK_KEY;
        //GPIO_PORTF_CR_R |= GPIO_PIN_0;

        // Enable port F for switches
        //GPIO_PORTF_DEN_R |= 0x00000001; // digital enable PF0
        //GPIO_PORTF_DEN_R |= 0x00000010; // digital enable PF4

        GPIO_PORTE_DEN_R |= 0x00000010; // digital enable PF4

        // set up pull up resistors (do I even need this?)
        //GPIO_PORTF_PUR_R |= 0x00000011;
        GPIO_PORTE_PUR_R |= 0x00000010;


        // Use SW1 & SW2, configure as inputs.
        //GPIO_PORTF_DIR_R &= ~0x00000011; // set pins 0 and 4 as inputs //I think this is wrong


    G8RTOS_Init();
    multimod_init();


    //GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_FALLING_EDGE);
    //GPIOIntEnable(GPIO_PORTF_BASE, GPIO_INT_PIN_0);



    // Add threads, semaphores, here
    G8RTOS_InitSemaphore(&sem_UART, 1);
    G8RTOS_InitSemaphore(&sem_SPIA, 1);
    G8RTOS_InitSemaphore(&sem_I2CA, 1);
    G8RTOS_InitSemaphore(&sem_PCA9555_Debounce, 1);
    G8RTOS_InitSemaphore(&sem_Joystick_Debounce, 1);
    G8RTOS_InitSemaphore(&sem_KillCube, 1);

    //G8RTOS_AddThread(task0, 8, "task 0");
    //G8RTOS_AddThread(task1, 2, "task 1");
    G8RTOS_AddThread(Read_Buttons, 3, "read_buttons");
    G8RTOS_AddThread(Read_JoystickPress, 3, "read_joystickpress");
    G8RTOS_AddThread(CamMove_Thread, 3, "cammove_thread");
    G8RTOS_AddThread(idle, 255, "idle");

    //G8RTOS_Add_APeriodicEvent(aperiodic_task, 4, 46);
    G8RTOS_Add_APeriodicEvent(GPIOE_Handler, 4, 20);
    G8RTOS_Add_APeriodicEvent(GPIOD_Handler, 4, 19);

    //G8RTOS_Add_PeriodicEvent(periodic_task, 100, SystemTime + 100);
    G8RTOS_Add_PeriodicEvent(Print_WorldCoords, 100, SystemTime);
    G8RTOS_Add_PeriodicEvent(Get_Joystick, 100, SystemTime + 1);


    G8RTOS_InitFIFO(SPAWNCOOR_FIFO);
    G8RTOS_InitFIFO(JOYSTICK_FIFO);


    G8RTOS_Launch();

    IntMasterEnable();
    while (1);
}

/************************************MAIN*******************************************/
