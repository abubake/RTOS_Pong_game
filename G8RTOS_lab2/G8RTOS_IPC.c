/*
 * G8RTOS_IPC.c
 *
 *  Created on: Mar 3, 2020
 *      Author: nicks
 */

#include <hyperloop-iot/G8RTOS_lab2/G8RTOS_IPC.h>
#include <hyperloop-iot/G8RTOS_lab2/G8RTOS_Semaphores.h>
#include <stdint.h>
#include "msp.h"

/*********************************************** Defines ******************************************************************************/

#define FIFOSIZE 16
#define MAX_NUMBER_OF_FIFOS 4

/*********************************************** Defines ******************************************************************************/


/*********************************************** Data Structures Used *****************************************************************/

/*
 * FIFO struct will hold
 *  - buffer
 *  - head
 *  - tail
 *  - lost data
 *  - current size
 *  - mutex
 */

/* Create FIFO struct here */
typedef struct FIFO_t
{
    int32_t Buffer[FIFOSIZE];
    int32_t *Head;
    int32_t *Tail;
    uint32_t LostData;
    semaphore_t CurrentSize;
    semaphore_t Mutex;
}FIFO_t;


/* Array of FIFOS */
static FIFO_t FIFOs[4];


/*********************************************** Data Structures Used *****************************************************************/

/*
 * Initializes FIFO Struct
 */
int G8RTOS_InitFIFO(uint32_t FIFOIndex)
{
    //Check we can add
    if(FIFOIndex >= MAX_NUMBER_OF_FIFOS)
    {
        return 1;
    }

    FIFOs[FIFOIndex].LostData = 0;
    FIFOs[FIFOIndex].Head = &FIFOs[FIFOIndex].Buffer[0];
    FIFOs[FIFOIndex].Tail = &FIFOs[FIFOIndex].Buffer[0];

    G8RTOS_InitSemaphore(&FIFOs[FIFOIndex].CurrentSize, 0);
    G8RTOS_InitSemaphore(&FIFOs[FIFOIndex].Mutex, 1);

    //Was successful
    return 0;
}

/*
 * Reads FIFO
 *  - Waits until CurrentSize semaphore is greater than zero
 *  - Gets data and increments the head ptr (wraps if necessary)
 * Param: "FIFOChoice": chooses which buffer we want to read from
 * Returns: uint32_t Data from FIFO
 */
uint32_t readFIFO(uint32_t FIFOChoice)
{
    //Wait in case being read from another thread
    G8RTOS_WaitSemaphore(&FIFOs[FIFOChoice].Mutex);
    //Wait for Current size to not be 0
    G8RTOS_WaitSemaphore(&FIFOs[FIFOChoice].CurrentSize);

    //Read head value
    uint32_t dataToRead = *FIFOs[FIFOChoice].Head;

    //Increment and wrap head pointer
    FIFOs[FIFOChoice].Head++;
    if(FIFOs[FIFOChoice].Head > &FIFOs[FIFOChoice].Buffer[FIFOSIZE-1])
    {
        FIFOs[FIFOChoice].Head = &FIFOs[FIFOChoice].Buffer[0];
    }

    //Signal Mutex semaphore
    G8RTOS_SignalSemaphore(&FIFOs[FIFOChoice].Mutex);

    return dataToRead;
}

/*
 * Writes to FIFO
 *  Writes data to Tail of the buffer if the buffer is not full
 *  Increments tail (wraps if ncessary)
 *  Param "FIFOChoice": chooses which buffer we want to read from
 *        "Data': Data being put into FIFO
 *  Returns: error code for full buffer if unable to write
 */
int writeFIFO(uint32_t FIFOChoice, uint32_t Data)
{

    if(FIFOs[FIFOChoice].CurrentSize > FIFOSIZE-1)
    {
        //If larger than FIFOSIZE-1 then increment lost data and overwrite
        FIFOs[FIFOChoice].LostData++;
        *FIFOs[FIFOChoice].Tail = Data;

        //Increment tail and wrap
        FIFOs[FIFOChoice].Tail++;
        if(FIFOs[FIFOChoice].Tail > &FIFOs[FIFOChoice].Buffer[FIFOSIZE-1])
        {
            FIFOs[FIFOChoice].Tail = &FIFOs[FIFOChoice].Buffer[0];
        }

        return 1;
    }
    else
    {
        //Else write the data
        *FIFOs[FIFOChoice].Tail = Data;

        //Increment tail and wrap
        FIFOs[FIFOChoice].Tail++;
        if(FIFOs[FIFOChoice].Tail > &FIFOs[FIFOChoice].Buffer[FIFOSIZE-1])
        {
            FIFOs[FIFOChoice].Tail = &FIFOs[FIFOChoice].Buffer[0];
        }

        //Signal CurrentSize to increase and notify other threads of it having data
        G8RTOS_SignalSemaphore(&FIFOs[FIFOChoice].CurrentSize);

        return 0;
    }

}

