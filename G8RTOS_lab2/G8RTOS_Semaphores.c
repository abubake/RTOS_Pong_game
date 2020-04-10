/*
 * G8RTOS_Semaphores.c
 */
/*********************************************** Dependencies and Externs *************************************************************/

#include "msp.h"
#include "G8RTOS.h"

/*********************************************** Dependencies and Externs *************************************************************/



/*********************************************** Public Functions *********************************************************************/

/* Initializes a semaphore to a given value
 * Param "s": Pointer to semaphore
 * Param "value": Value to initialize semaphore to
 * X holds the PRIMASK value
 * THIS IS A CRITICAL SECTION */
void G8RTOS_InitSemaphore(semaphore_t *s, int32_t value)
{
	uint32_t x; // used to have this as global
	x = StartCriticalSection();
	*s = value;
	EndCriticalSection(x);
}

/* No longer waits for semaphore
 *  - Decrements semaphore
 *  - Blocks thread if sempahore is unavalible
 * Param "s": Pointer to semaphore to wait on
 * THIS IS A CRITICAL SECTION */
void G8RTOS_WaitSemaphore(semaphore_t *s)
{
	uint32_t x;
	x = StartCriticalSection();
	(*s) = (*s) - 1;
	if ((*s) < 0){
		CurrentlyRunningThread->blocked = s; // the "blocked" semaphore points to the blocked thread
		EndCriticalSection(x);
		StartContextSwitch(); // runs the next thread
	}
	EndCriticalSection(x);
}


/* Signals the completion of the usage of a semaphore
 *  - Increments the semaphore value by 1
 *  - Unblocks any threads waiting on that semaphore
 * Param "s": Pointer to semaphore to be signaled
 * THIS IS A CRITICAL SECTION */
void G8RTOS_SignalSemaphore(semaphore_t *s)
{
	uint32_t x;
	tcb_t *tempPt;
	x = StartCriticalSection();
	(*s)++;
	if ((*s) <= 0){
		tempPt = CurrentlyRunningThread->Next;

		while(tempPt->blocked != s){
			tempPt = tempPt->Next;
		}
		(tempPt->blocked) = 0; //unblock this
	}
	EndCriticalSection(x);
}

/* Waits for a semaphore to be available (value greater than 0)
 * 	- Decrements semaphore when available
 * 	- Spinlocks to wait for semaphore
 * 	- The resource is held and owned
 * Param "s": Pointer to semaphore to wait on
 * THIS IS A CRITICAL SECTION */
void G8RTOS_WaitSemaphoreMutex(semaphore_t *s)
{
	uint32_t x;
	x = StartCriticalSection();

	while ((*s) == 0){
		EndCriticalSection(x);
		x = StartCriticalSection();
	}
	(*s) = (*s) - 1;
	EndCriticalSection(x);
}

/* Signals the completion of the usage of a semaphore
 * 	- Increments the semaphore value by 1
 * 	- The resource is now available
 * Param "s": Pointer to semaphore to be signaled
 * THIS IS A CRITICAL SECTION */
void G8RTOS_SignalSemaphoreMutex(semaphore_t *s)
{
	uint32_t x;
	x = StartCriticalSection();
	(*s)++;
	EndCriticalSection(x);
}
/**************************************************************************************************************************************/


