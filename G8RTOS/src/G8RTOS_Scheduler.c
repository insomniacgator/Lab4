// G8RTOS_Scheduler.c
// Date Created: 2023-07-25
// Date Updated: 2023-07-27
// Defines for scheduler functions

#include "../G8RTOS_Scheduler.h"

/************************************Includes***************************************/

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "../G8RTOS_CriticalSection.h"

#include <inc/hw_memmap.h>
#include "inc/hw_types.h"
//#include "inc/hw_ints.h"
#include "inc/hw_nvic.h"
#include "driverlib/systick.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"

/********************************Private Variables**********************************/

// Thread Control Blocks - array to hold information for each thread
static tcb_t threadControlBlocks[MAX_THREADS];

// Thread Stacks - array of arrays for individual stacks of each thread
static uint32_t threadStacks[MAX_THREADS][STACKSIZE];

// Periodic Event Threads - array to hold pertinent information for each thread
static ptcb_t pthreadControlBlocks[MAX_PTHREADS];

// Current Number of Threads currently in the scheduler
static uint32_t NumberOfThreads;

// Current Number of Periodic Threads currently in the scheduler
static uint32_t NumberOfPThreads;

static uint32_t threadCounter = 0;

/*******************************Private Functions***********************************/

// Occurs every 1 ms.
static void InitSysTick(void)
{
    // Replace with code from lab 3
    // Disable SysTick during setup
            NVIC_ST_CTRL_R = 0;
        // hint: use SysCtlClockGet() to get the clock speed without having to hardcode it!
        uint32_t sysClock = SysCtlClockGet();

        uint32_t sysTickPeriod = (sysClock / 1000) ; // 1 ms = 1000 Hz

        // Set systick period
        NVIC_ST_RELOAD_R = sysTickPeriod;

        // Set systick interrupt handler (vector 15)
        SysTickIntRegister(SysTick_Handler);
        //IntRegister(15, SysTick_Handler);
        NVIC_PRI3_R |= 0xE0000000;
        // not sure how to set the interrupt handler to SysTick_Handler()

        // Set pendsv handler (vector 14)
        IntRegister(14 , PendSV_Handler);
        //NVIC_PRI3_R |= 0x00E00000;
        // enable systick clk source
        NVIC_ST_CTRL_R |= NVIC_ST_CTRL_CLK_SRC;
        // Enable systick interrupt
        NVIC_ST_CTRL_R |= NVIC_ST_CTRL_INTEN;
        // Enable systick
        NVIC_ST_CTRL_R |= NVIC_ST_CTRL_ENABLE;
}


/********************************Public Variables***********************************/

uint32_t SystemTime;

tcb_t* CurrentlyRunningThread;



/********************************Public Functions***********************************/

// SysTick_Handler
// Increments system time, sets PendSV flag to start scheduler.
// Return: void
void SysTick_Handler() {
    SystemTime++;



    // Traverse the linked-list to find which threads should be awake.
    // Here we will compare sleepCount to SystemTime and wake up threads if it is their time
    //tcb_t *pt = (tcb_t *)&CurrentlyRunningThread->nextTCB;
    tcb_t *pt = (tcb_t *)&threadControlBlocks[0];

    // HERE: to traverse the linked list: should I use condition to stop: next==null? or loop numberOfThreads times?
    for (int16_t i=0; i< NumberOfThreads; i++)
    {
        if ( pt->sleepCount <= SystemTime)
        {
            pt->asleep = 0;
            pt->sleepCount = 0;
        }
        pt = pt->nextTCB;
    }

    // Traverse the periodic linked list to run which functions need to be run.

    // set PendSV flag to start scheduler
    HWREG(NVIC_INT_CTRL) |= NVIC_INT_CTRL_PEND_SV; // am I supposed to trigger the interrupt or just set the flag?
    // I am a bit confused today. Are we context switching every SysTick?
}

// G8RTOS_Init
// Initializes the RTOS by initializing system time.
// Return: void
void G8RTOS_Init() {
    uint32_t newVTORTable = 0x20000000;
    uint32_t* newTable = (uint32_t*)newVTORTable;
    uint32_t* oldTable = (uint32_t*) 0;

    for (int i = 0; i < 155; i++) {
        newTable[i] = oldTable[i];
    }

    HWREG(NVIC_VTABLE) = newVTORTable; //Why are we replacing the VTABLE here?

    SystemTime = 0;
    NumberOfThreads = 0;
    NumberOfPThreads = 0;
}

// G8RTOS_Launch
// Launches the RTOS.
// Return: error codes, 0 if none
int32_t G8RTOS_Launch() {
    // Replace with code from lab 3
    // Initialize system tick
    InitSysTick();
    // Set currently running thread to the first control block
    CurrentlyRunningThread = &threadControlBlocks[0];
    // Set interrupt priorities
       // Pendsv
       // Systick
    NVIC_PRI3_R |= 0xE0E00000;

        // Set SysTick priority (adjust as needed)
        //NVIC_SetPriority(SysTick_IRQn, configKERNEL_INTERRUPT_PRIORITY + 1);
    // Call G8RTOS_Start()
        G8RTOS_Start();
    return 0;
}

// G8RTOS_Scheduler
// Chooses next thread in the TCB. This time uses priority scheduling.
// Return: void
void G8RTOS_Scheduler() {
    // Using priority, determine the most eligible thread to run that
    // is not blocked or asleep. Set current thread to this thread's TCB.


    tcb_t *pt = (tcb_t *)&threadControlBlocks[0];
    tcb_t *highest = 0;

    // HERE: to traverse the linked list: should I use condition to stop: next==null? or loop numberOfThreads times?
    for (int16_t i=0; i< NumberOfThreads; i++)
    {
        if ( !pt->blocked && !pt->asleep )
        {
            if (!highest)
                highest = pt;
            else if (highest->priority > pt->priority)
            {
                highest = pt;
            }

        }
        pt = pt->nextTCB;
    }

    CurrentlyRunningThread = highest;


}

// G8RTOS_AddThread
// Adds a thread. This is now in a critical section to support dynamic threads.
// It also now should initalize priority and account for live or dead threads.
// Param void* "threadToAdd": pointer to thread function address
// Param uint8_t "priority": priority from 0, 255.
// Param char* "name": character array containing the thread name.
// Return: sched_ErrCode_t
sched_ErrCode_t G8RTOS_AddThread(void (*threadToAdd)(void), uint8_t priority, char *name) {
    // Your code here

    // This should be in a critical section! <---- But why?
    IBit_State = StartCriticalSection();

    // If number of threads is greater than the maximum number of threads
        if ( NumberOfThreads > MAX_THREADS )
            // return
            return THREAD_LIMIT_REACHED;
        // else
        else
        {


            // if no threads
            if ( NumberOfThreads == 0)
            {
                // set thread as HEAD and TAIL
                threadControlBlocks[0].nextTCB = &threadControlBlocks[0];
                threadControlBlocks[0].previousTCB = &threadControlBlocks[0];
                //threadControlBlocks[0].threadName = name;
                strcpy(threadControlBlocks[0].threadName, name);
                threadControlBlocks[0].priority = priority;

                // set currently running thread
            }
            // else
            else
            {
                threadControlBlocks[NumberOfThreads - 1].nextTCB = &threadControlBlocks[NumberOfThreads]; // we wrap around for round-robin scheduling
                threadControlBlocks[0].previousTCB = &threadControlBlocks[NumberOfThreads];
                threadControlBlocks[NumberOfThreads].previousTCB = &threadControlBlocks[NumberOfThreads - 1];
                threadControlBlocks[NumberOfThreads].nextTCB = &threadControlBlocks[0];
                //threadControlBlocks[NumberOfThreads].threadName = name;
                strcpy(threadControlBlocks[NumberOfThreads].threadName, name);
                threadControlBlocks[NumberOfThreads].priority = priority;
            }
                /*
                Append the new thread to the end of the linked list
                * 1. Number of threads will refer to the newest thread to be added since the current index would be NumberOfThreads-1
                * 2. Set the next thread for the new thread to be the first in the list, so that round-robin will be maintained
                * 3. Set the current thread's nextTCB to be the new thread
                * 4. Set the first thread's previous thread to be the new thread, so that it goes in the right spot in the list
                * 5. Point the previousTCB of the new thread to the current thread so that it moves in the correct order
                */

            threadStacks[NumberOfThreads][STACKSIZE - 2] = (uint32_t)threadToAdd; //we set the function pointer into the PC register 2 spots
            // the from bottom of the stack
            threadStacks[NumberOfThreads][STACKSIZE - 1] = THUMBBIT; // we set THUMBIT on the PSR word spot (last item, bottom of stack)
            threadControlBlocks[NumberOfThreads].stackPointer = &threadStacks[NumberOfThreads][STACKSIZE - 16]; // we put the SP up 15 places
            // from the bottom of the stack. (we use 16 because last spot is STACKSIZE-1, ie count starts from 0. also we do not save SP (but into).

            NumberOfThreads++;
            EndCriticalSection(IBit_State);
            return NO_ERROR;
        }




}

// G8RTOS_Add_APeriodicEvent


// Param void* "AthreadToAdd": pointer to thread function address
// Param int32_t "IRQn": Interrupt request number that references the vector table. [0..155].
// Return: sched_ErrCode_t
sched_ErrCode_t G8RTOS_Add_APeriodicEvent(void (*AthreadToAdd)(void), uint8_t priority, int32_t IRQn) {
    // Disable interrupts
    // Check if IRQn is valid
    // Check if priority is valid
    // Set corresponding index in interrupt vector table to handler.
    // Set priority.
    // Enable the interrupt.
    // End the critical section.

}

// G8RTOS_Add_PeriodicEvent
// Adds periodic threads to G8RTOS Scheduler
// Function will initialize a periodic event struct to represent event.
// The struct will be added to a linked list of periodic events
// Param void* "PThreadToAdd": void-void function for P thread handler
// Param uint32_t "period": period of P thread to add
// Param uint32_t "execution": When to execute the periodic thread
// Return: sched_ErrCode_t
sched_ErrCode_t G8RTOS_Add_PeriodicEvent(void (*PThreadToAdd)(void), uint32_t period, uint32_t execution) {
    // your code
    // Make sure that the number of PThreads is not greater than max PThreads.
        // Check if there is no PThread. Initialize and set the first PThread.
        // Subsequent PThreads should be added, inserted similarly to a doubly-linked linked list
            // last PTCB should point to first, last PTCB should point to last.
        // Set function
        // Set period
        // Set execute time
        // Increment number of PThreads

    return NO_ERROR;
}

// G8RTOS_KillThread
// Param uint32_t "threadID": ID of thread to kill
// Return: sched_ErrCode_t
sched_ErrCode_t G8RTOS_KillThread(threadID_t threadID) {
    // Start critical section
    // Check if there is only one thread, return if so
    // Traverse linked list, find thread to kill
        // Update the next tcb and prev tcb pointers if found
            // mark as not alive, release the semaphore it is blocked on
        // Otherwise, thread does not exist.


}

// G8RTOS_KillSelf
// Kills currently running thread.
// Return: sched_ErrCode_t
sched_ErrCode_t G8RTOS_KillSelf() {
    // your code

    // Check if there is only one thread
    // Else, mark this thread as not alive.

}

// sleep
// Puts current thread to sleep
// Param uint32_t "durationMS": how many systicks to sleep for
void sleep(uint32_t durationMS) {
    // Update time to sleep to
    // Set thread as asleep
    CurrentlyRunningThread->sleepCount = durationMS + SystemTime;
    CurrentlyRunningThread->asleep = 1;

    // and then trigger context switch
    HWREG(NVIC_INT_CTRL) |= NVIC_INT_CTRL_PEND_SV;



}

// G8RTOS_GetThreadID
// Gets current thread ID.
// Return: threadID_t
threadID_t G8RTOS_GetThreadID(void) {
    return CurrentlyRunningThread->ThreadID;        //Returns the thread ID
}

// G8RTOS_GetNumberOfThreads
// Gets number of threads.
// Return: uint32_t
uint32_t G8RTOS_GetNumberOfThreads(void) {
    return NumberOfThreads;         //Returns the number of threads
}
