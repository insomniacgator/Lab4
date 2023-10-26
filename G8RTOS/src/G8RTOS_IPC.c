// G8RTOS_IPC.c
// Date Created: 2023-07-25
// Date Updated: 2023-07-27
// Defines for FIFO functions for interprocess communication

#include "../G8RTOS_IPC.h"

/************************************Includes***************************************/

#include "../G8RTOS_Semaphores.h"

/******************************Data Type Definitions********************************/

/****************************Data Structure Definitions*****************************/


// The manual says to define this G8RTOS_IPC.h, template error?
//typedef struct G8RTOS_FIFO_t {

//} G8RTOS_FIFO_t;


/***********************************Externs*****************************************/

/********************************Private Variables***********************************/

static G8RTOS_FIFO_t FIFOs[MAX_NUMBER_OF_FIFOS];


/********************************Public Functions***********************************/

// G8RTOS_InitFIFO
// Initializes FIFO, points head & tai to relevant buffer
// memory addresses. Returns - 1 if FIFO full, 0 if no error
// Param uint32_t "FIFO_index": Index of FIFO block
// Return: int32_t
int32_t G8RTOS_InitFIFO(uint32_t FIFO_index) {
    // Check if FIFO index is out of bounds
    if (FIFO_index > 15)
    {
        while(1)
            UARTprintf("FIFO index is out of bounds\n");
    }

    // Init head, tail pointers
    G8RTOS_FIFO_t *fifo = &FIFOs[FIFO_index];
    fifo->head = 0;
    fifo->tail = 0;
    //fifo->test = 4745;

    // Init the mutex, current size
    fifo->mutex = 1;
    fifo->currentSize = 0;
    // Init lost data
    fifo->lostData = 0;


    // why should we check if fifo full this is init?
    return 0;
}

// G8RTOS_ReadFIFO
// Reads data from head pointer of FIFO.
// Param uint32_t "FIFO_index": Index of FIFO block
// Return: int32_t
int32_t G8RTOS_ReadFIFO(uint32_t FIFO_index) {

    // Your code
    // Be mindful of boundary conditions!
    G8RTOS_FIFO_t *fifo = &FIFOs[FIFO_index];
    //check if fifo is being used
    G8RTOS_WaitSemaphore(&fifo->mutex); // acquire mutex

    //check if there is any data and decrease buffer size
    G8RTOS_WaitSemaphore(&fifo->currentSize);
    if (fifo->head > 15) // if head is greater than 15 (max buffer index)
        fifo->head = 0; // reset the head index to 0 (wrap around)
    int32_t read_data = fifo->buffer[fifo->head]; // read data
    fifo->head = fifo->head + 1; // increase head index by 1 after data read.

    G8RTOS_SignalSemaphore(&fifo->mutex); // release mutex

    return read_data;


}

// G8RTOS_WriteFIFO
// Writes data to tail of buffer.
// 0 if no error, -1 if out of bounds, -2 if full
// Param uint32_t "FIFO_index": Index of FIFO block
// Param uint32_t "data": data to be written
// Return: int32_t
int32_t G8RTOS_WriteFIFO(uint32_t FIFO_index, uint32_t data) {
    // Your code
    G8RTOS_FIFO_t *fifo = &FIFOs[FIFO_index];

    //G8RTOS_WaitSemaphore(&fifo->mutex); // acquire mutex

    // check if full
    if ( fifo->currentSize > 15 ) // max buffer size is 16 so if currentSize greater than 15 we full cuh
        return (-2); // return -2 error
    // check if out of bounds
    if ( FIFO_index > MAX_NUMBER_OF_FIFOS - 1 ) // if fifo index is out of bounds
        return (-1); // return -1 error

    // write data to fifo buffer
    fifo->buffer[fifo->tail] = data; // write data at the tail of fifo buffer
    fifo->tail++; // increase tail index by 1
    if (fifo->tail > 15)
        fifo->tail = 0;
    G8RTOS_SignalSemaphore(&fifo->currentSize); // increase currentSize by 1
    //G8RTOS_SignalSemaphore(&fifo->mutex);

    return 0;

}

