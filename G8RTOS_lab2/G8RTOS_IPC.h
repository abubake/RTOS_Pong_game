/*
 * G8RTOS_IPC.h
 *
 *  Created on: Jan 10, 2017
 *      Author: Daniel Gonzalez
 */

#ifndef G8RTOS_G8RTOS_IPC_H_
#define G8RTOS_G8RTOS_IPC_H_

#include "G8RTOS.h"
/*********************************************** Error Codes **************************************************************************/
#define FIFOSIZE 16
#define MAX_NUMBER_OF_FIFOS 4
/*********************************************** Error Codes **************************************************************************/

/*********************************************** Public Functions *********************************************************************/
typedef struct FIFO_t{

	int32_t Buffer[FIFOSIZE];
	int32_t *Head;
	int32_t *Tail;

	uint32_t lostData;
	semaphore_t CurrentSize;
	semaphore_t Mutex;

}FIFO_t;

/* Array of FIFOS */
static FIFO_t FIFOs[4];
/*
 * Initializes One to One FIFO Struct
 */
int G8RTOS_InitFIFO(uint32_t FIFOIndex);

/*
 * Reads FIFO
 *  - Waits until CurrentSize semaphore is greater than zero
 *  - Gets data and increments the head ptr (wraps if necessary)
 * Param "FIFOChoice": chooses which buffer we want to read from
 * Returns: uint32_t Data from FIFO */
int32_t readFIFO(int32_t FIFO);

/*
 * Writes to FIFO
 *  Writes data to Tail of the buffer if the buffer is not full
 *  Increments tail (wraps if ncessary)
 *  Param "FIFOChoice": chooses which buffer we want to read from
 *        "Data': Data being put into FIFO
 *  Returns: error code for full buffer if unable to write
 */
int writeFIFO(int32_t FIFO, int32_t data);

/*********************************************** Public Functions *********************************************************************/


#endif /* G8RTOS_G8RTOS_IPC_H_ */
