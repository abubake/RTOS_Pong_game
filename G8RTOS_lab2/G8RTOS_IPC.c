/*
 * G8RTOS_IPC.c
 *
 *  Created on: Jan 10, 2017
 *      Author: Daniel Gonzalez
 */
#include <stdint.h>
#include "msp.h"
#include "G8RTOS_IPC.h"
#include "G8RTOS_Semaphores.h"

/*********************************************** Defines ******************************************************************************/


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


/*********************************************** Data Structures Used *****************************************************************/

/*
 * Initializes FIFO Struct
 */
int G8RTOS_InitFIFO(uint32_t FIFOIndex)
{
	if (FIFOIndex > 4){
		return -1;
	}
	else {
				FIFOs[FIFOIndex].Head = &FIFOs[FIFOIndex].Buffer[0];
				FIFOs[FIFOIndex].Tail = &FIFOs[FIFOIndex].Buffer[0];
				G8RTOS_InitSemaphore(&(FIFOs[FIFOIndex].CurrentSize), 0); //this right?
				G8RTOS_InitSemaphore(&(FIFOs[FIFOIndex].Mutex), 1);
				FIFOs[FIFOIndex].lostData = 0;
		return 0;
	}
}

/*
 * Reads FIFO
 *  - Waits until CurrentSize semaphore is greater than zero
 *  - Gets data and increments the head ptr (wraps if necessary)
 * Param: "FIFOChoice": chooses which buffer we want to read from
 * Returns: uint32_t Data from FIFO
 */
int32_t readFIFO(int32_t FIFOChoice)
{
	int32_t data = 0;

    G8RTOS_WaitSemaphore(&(FIFOs[FIFOChoice].CurrentSize));
    G8RTOS_WaitSemaphore(&(FIFOs[FIFOChoice].Mutex)); // The one causing the issue

    data = *(FIFOs[FIFOChoice].Head); // is this value being retrieved correctly?
    (FIFOs[FIFOChoice].Head)++;

    if (FIFOs[FIFOChoice].Head == &FIFOs[FIFOChoice].Buffer[FIFOSIZE-1]){
    	FIFOs[FIFOChoice].Head = &FIFOs[FIFOChoice].Buffer[0];
    }

    G8RTOS_SignalSemaphore(&(FIFOs[FIFOChoice].Mutex));
    return data;
}

/*
 * Writes to FIFO
 *  Writes data to Tail of the buffer if the buffer is not full
 *  Increments tail (wraps if ncessary)
 *  Param "FIFOChoice": chooses which buffer we want to read from
 *        "Data': Data being put into FIFO
 *  Returns: error code for full buffer if unable to write */
int32_t writeFIFO(int32_t FIFOChoice, int32_t Data)
{
if(FIFOs[FIFOChoice].CurrentSize == FIFOSIZE-1){ // was FIFOSIZE -1
	FIFOs[FIFOChoice].lostData++;
	return -1;
}
	*(FIFOs[FIFOChoice].Tail) = Data; // a put
	FIFOs[FIFOChoice].Tail++; //place for next

	if (FIFOs[FIFOChoice].Tail == &FIFOs[FIFOChoice].Buffer[FIFOSIZE-1]){
		FIFOs[FIFOChoice].Tail = &FIFOs[FIFOChoice].Buffer[0];
	}

		G8RTOS_SignalSemaphore(&FIFOs[FIFOChoice].CurrentSize); //should this be the old kind of signal semaphore? (MAY NOT BE APPROPRIATE)
		return 0;
}

