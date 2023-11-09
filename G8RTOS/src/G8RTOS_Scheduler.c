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

static tcb_t *headTCB = 0;
static tcb_t *tailTCB = 0;

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
    // add periodic thread set to system time


    // Traverse the linked-list to find which threads should be awake.
    // Here we will compare sleepCount to SystemTime and wake up threads if it is their time
    //tcb_t *pt = (tcb_t *)&CurrentlyRunningThread->nextTCB;

    //tcb_t *pt = (tcb_t *)&threadControlBlocks[0];
    // we'll change the way we get the head since the array is not in sync w/ the linked list anymore
    tcb_t *pt = headTCB; // get a pointer to the first thread of the linked list

    // HERE: to traverse the linked list: should I use condition to stop: next==null? or loop numberOfThreads times?
    for (int16_t i=0; i< NumberOfThreads; i++)
    {
        if ( pt->sleepCount <= SystemTime) // if sleepCount is less than systime means its time to wake up
        {
            pt->asleep = 0; // wake up
            pt->sleepCount = 0; // reset sleep count to 0
        }
        pt = pt->nextTCB; // if its not time to wake up skip to the next.
    }


    // Traverse the periodic linked list to run which functions need to be run.
    ptcb_t *ppt = (ptcb_t *)&pthreadControlBlocks[0]; // we get the first periodic thread from pthread array
    for (int16_t i=0; i < NumberOfPThreads; i++) // we are gonna loop for as many pthreads that we have in our array
    {
        if ( ppt->executeTime <= SystemTime ) // we check if it is time to execute periodic thread
        {
            ppt->handler(); // if it is time to execute we execute the thread;
            ppt->executeTime = SystemTime + ppt->period; // and we set its new execute time to current SystemTime + period
        }
        ppt = ppt->nextPTCB; // we go to the next pthread in the linked list (I guess we could have also use array and index here)
    }





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


    //tcb_t *pt = (tcb_t *)&threadControlBlocks[0];
    tcb_t *pt = CurrentlyRunningThread;
    tcb_t *highest = 0;

    // HERE: to traverse the linked list: should I use condition to stop: next==null? or loop numberOfThreads times?
    for (int16_t i=0; i< NumberOfThreads; i++)
    //while (headTCB->ThreadID != pt->nextTCB->ThreadID)
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
    //static int32_t thread_id = 0;

    // If number of threads is greater than the maximum number of threads
        if ( NumberOfThreads >= MAX_THREADS )
        {
            // return
            EndCriticalSection(IBit_State);
            return THREAD_LIMIT_REACHED;
        }
        // else
        else
        {
            // We need to modify Add Thread so we can add threads where the dead threads where and not just at the end.
            // we should traverse ThreadControlBlocks array by incrementing index and find dead threads and write there.

            // if no threads we just set our thread at slot 0 in the array
            if ( threadCounter == 0)
            {
                // set thread as HEAD and TAIL
                threadControlBlocks[0].nextTCB = &threadControlBlocks[0];
                threadControlBlocks[0].previousTCB = &threadControlBlocks[0];

                strcpy(threadControlBlocks[0].threadName, name);
                threadControlBlocks[0].priority = priority;
                threadControlBlocks[0].ThreadID = threadCounter;
                headTCB = &threadControlBlocks[0];
                tailTCB = &threadControlBlocks[0];

                // set currently running thread
                threadStacks[0][STACKSIZE - 2] = (uint32_t)threadToAdd; //we set the function pointer into the PC register 2 spots
                // the from bottom of the stack
                threadStacks[0][STACKSIZE - 1] = THUMBBIT; // we set THUMBIT on the PSR word spot (last item, bottom of stack)
                threadControlBlocks[0].stackPointer = &threadStacks[0][STACKSIZE - 16]; // we put the SP up 15 places
                // from the bottom of the stack. (we use 16 because last spot is STACKSIZE-1, ie count starts from 0. also we do not save SP (but into).
                threadControlBlocks[0].isAlive = 1;
                threadControlBlocks[0].asleep = 0;
            }
            // else
            else if ( threadCounter < MAX_THREADS) // we just keep filling that array until before we get to MAX_THREADS
            {
                threadControlBlocks[threadCounter].previousTCB = tailTCB;
                threadControlBlocks[threadCounter].nextTCB = headTCB;
                headTCB->previousTCB = &threadControlBlocks[threadCounter];
                tailTCB->nextTCB = &threadControlBlocks[threadCounter];

                tailTCB = &threadControlBlocks[threadCounter];

                strcpy(threadControlBlocks[threadCounter].threadName, name);
                threadControlBlocks[threadCounter].priority = priority;
                threadControlBlocks[threadCounter].ThreadID = threadCounter;

                threadStacks[threadCounter][STACKSIZE - 2] = (uint32_t)threadToAdd; //we set the function pointer into the PC register 2 spots
                // the from bottom of the stack
                threadStacks[threadCounter][STACKSIZE - 1] = THUMBBIT; // we set THUMBIT on the PSR word spot (last item, bottom of stack)
                threadControlBlocks[threadCounter].stackPointer = &threadStacks[threadCounter][STACKSIZE - 16]; // we put the SP up 15 places
                // from the bottom of the stack. (we use 16 because last spot is STACKSIZE-1, ie count starts from 0. also we do not save SP (but into).
                threadControlBlocks[threadCounter].isAlive = 1;
                threadControlBlocks[threadCounter].asleep = 0;

            }
            else // once that array is full we have to traverse the array looking for a dead thread to replace.
            {
                // however we will add at the end of the linked list however regardless of the place in the TCB array.

                int8_t index = 0;
                // we traverse the TCB array until we find a dead thread, if we cant find a dead thread we return
                while (threadControlBlocks[index].isAlive)
                {
                    index++;
                    if (index >= MAX_THREADS)
                    {
                        EndCriticalSection(IBit_State);
                        return THREAD_LIMIT_REACHED;
                    }
                }
                // if we here it means that we found a dead thread at index
                threadControlBlocks[index].previousTCB = tailTCB; // so point the new thread's previous pointer to the old tail
                threadControlBlocks[index].nextTCB = headTCB; // point the new thread's next pointer to the head

                tailTCB->nextTCB = &threadControlBlocks[index]; //point the old tail's next pointer to the new thread ( new tail )
                headTCB->previousTCB = &threadControlBlocks[index]; // point the head's previous pointer to the new thread ( new tail )

                tailTCB = &threadControlBlocks[index]; // make the new thread the new tail

                strcpy(threadControlBlocks[index].threadName, name); // copy name string into threadName
                threadControlBlocks[index].priority = priority; // set priority
                threadControlBlocks[index].ThreadID = threadCounter; // set threadID from threadCounter

                // reset stack before?
                for (int32_t i=0; i<STACKSIZE; i++)
                {
                    threadStacks[index][i] = 0;
                }

                threadStacks[index][STACKSIZE - 2] = (uint32_t)threadToAdd; //we set the function pointer into the PC register 2 spots
                // the from bottom of the stack
                threadStacks[index][STACKSIZE - 1] = THUMBBIT; // we set THUMBIT on the PSR word spot (last item, bottom of stack)
                threadControlBlocks[index].stackPointer = &threadStacks[index][STACKSIZE - 16]; // we put the SP up 15 places
                // from the bottom of the stack. (we use 16 because last spot is STACKSIZE-1, ie count starts from 0. also we do not save SP (but into).
                threadControlBlocks[index].isAlive = 1;
                threadControlBlocks[index].asleep = 0;

            }
                /*
                Append the new thread to the end of the linked list
                * 1. Number of threads will refer to the newest thread to be added since the current index would be NumberOfThreads-1
                * 2. Set the next thread for the new thread to be the first in the list, so that round-robin will be maintained
                * 3. Set the current thread's nextTCB to be the new thread
                * 4. Set the first thread's previous thread to be the new thread, so that it goes in the right spot in the list
                * 5. Point the previousTCB of the new thread to the current thread so that it moves in the correct order
                */



            NumberOfThreads++;
            threadCounter++;
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
    IBit_State = StartCriticalSection();
    // Check if IRQn is valid
    if ( IRQn >= 255 | IRQn <= 0)
    {
        EndCriticalSection(IBit_State);
        return IRQn_INVALID;
    }
    // Check if priority is valid
    if (priority > 6)
    {
        EndCriticalSection(IBit_State);
        return HWI_PRIORITY_INVALID;
    }
    // Set corresponding index in interrupt vector table to handler.
    uint32_t *vectors = (uint32_t*)HWREG(NVIC_VTABLE);
    vectors[IRQn] = (uint32_t) AthreadToAdd;
    // Set priority.
    IntPrioritySet(IRQn, priority);
    // Enable the interrupt.
    IntEnable(IRQn);
    //IntMasterEnable(); Do I need to add this here?
    // End the critical section.
    EndCriticalSection(IBit_State);

    return NO_ERROR;

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
    if (NumberOfPThreads >= MAX_PTHREADS)
        return THREAD_LIMIT_REACHED;
    // Check if there is no PThread. Initialize and set the first PThread.
    if ( NumberOfPThreads == 0)
    {
        // set same only thread as next and previous
        pthreadControlBlocks[0].nextPTCB = &pthreadControlBlocks[0];
        pthreadControlBlocks[0].previousPTCB = &pthreadControlBlocks[0];
    }
    // else set the last thread next to the this thread and the first thread previous to this thread
    else
    {
        pthreadControlBlocks[NumberOfPThreads - 1].nextPTCB = &pthreadControlBlocks[NumberOfPThreads];
        pthreadControlBlocks[0].previousPTCB = &pthreadControlBlocks[NumberOfPThreads];

        pthreadControlBlocks[NumberOfPThreads].previousPTCB = &pthreadControlBlocks[NumberOfPThreads - 1];
        pthreadControlBlocks[NumberOfPThreads].nextPTCB = &pthreadControlBlocks[0];
    }

    // Subsequent PThreads should be added, inserted similarly to a doubly-linked linked list
        // last PTCB should point to first, last PTCB should point to last.
    // Set function
    pthreadControlBlocks[NumberOfPThreads].handler = PThreadToAdd;
    // Set period
    pthreadControlBlocks[NumberOfPThreads].period = period;
    // Set execute time
    pthreadControlBlocks[NumberOfPThreads].executeTime = execution;
    // Increment number of PThreads
    NumberOfPThreads++;

    return NO_ERROR;
}

// G8RTOS_KillThread
// Param uint32_t "threadID": ID of thread to kill
// Return: sched_ErrCode_t
sched_ErrCode_t G8RTOS_KillThread(threadID_t threadID) {
    // Start critical section
    IBit_State = StartCriticalSection();
    // Check if there is only one thread, return if so
    if (NumberOfThreads <= 1)
    {
        EndCriticalSection(IBit_State);
        return CANNOT_KILL_LAST_THREAD;
    }
    // Traverse linked list, find thread to kill
    uint8_t index = 0;
    tcb_t *thread = headTCB;
    // while threadID does not match any of the threads IDs
    while ( !thread->ThreadID == threadID )
    {
        // keep traversing the linked list
        index++;
        thread = thread->nextTCB;
        if (index >= NumberOfThreads)
        {
            EndCriticalSection(IBit_State);
            return THREAD_DOES_NOT_EXIST;
        }
    }
    // if we are here we have a thread that matched threadID so we will kill it

    // we remove the thread from the linked list
    thread->nextTCB->previousTCB = thread->previousTCB;
    thread->previousTCB->nextTCB = thread->nextTCB;

    // what if we killed the head or the tail??
    if (thread == headTCB)
        headTCB = thread->nextTCB;
    if (thread == tailTCB)
        tailTCB = thread->previousTCB;

    // we set alive is 0 (kill it)
    thread->isAlive = 0;
    // if thread is blocked we signal the semaphore to release it
    if (thread->blocked)
        G8RTOS_SignalSemaphore(thread->blocked);
    NumberOfThreads--; // decrement number of threads

        // Update the next tcb and prev tcb pointers if found
            // mark as not alive, release the semaphore it is blocked on
        // Otherwise, thread does not exist.

    EndCriticalSection(IBit_State);

    return NO_ERROR;


}

// G8RTOS_KillSelf
// Kills currently running thread.
// Return: sched_ErrCode_t
sched_ErrCode_t G8RTOS_KillSelf() {
    // your code
    IBit_State = StartCriticalSection();

    // Check if there is only one thread
    if (NumberOfThreads <= 1)
    {
        EndCriticalSection(IBit_State);
            return CANNOT_KILL_LAST_THREAD;
    }


    // Else, mark this thread as not alive.

    tcb_t *thread = CurrentlyRunningThread;

    // we remove the thread from the linked list
    thread->nextTCB->previousTCB = thread->previousTCB;
    thread->previousTCB->nextTCB = thread->nextTCB;

    // what if we killed the head or the tail??
    if (thread == headTCB)
        headTCB = thread->nextTCB;
    if (thread == tailTCB)
        tailTCB = thread->previousTCB;

    // we set alive is 0 (kill it)
    thread->isAlive = 0;
    // if thread is blocked we signal the semaphore to release it
    if (thread->blocked)
        G8RTOS_SignalSemaphore(thread->blocked);
    NumberOfThreads--; // decrement number of threads

        // Update the next tcb and prev tcb pointers if found
            // mark as not alive, release the semaphore it is blocked on
        // Otherwise, thread does not exist.

    EndCriticalSection(IBit_State);

    return NO_ERROR;

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
