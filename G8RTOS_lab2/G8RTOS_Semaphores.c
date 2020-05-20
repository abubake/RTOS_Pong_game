/*
 * G8RTOS_Semaphores.c
 */

/*********************************************** Dependencies and Externs *************************************************************/

#include <hyperloop-iot/G8RTOS_lab2/G8RTOS_CriticalSection.h>
#include <hyperloop-iot/G8RTOS_lab2/G8RTOS_Scheduler.h>
#include <hyperloop-iot/G8RTOS_lab2/G8RTOS_Semaphores.h>
#include <hyperloop-iot/G8RTOS_lab2/G8RTOS_Structures.h>
#include "msp.h"

/*********************************************** Dependencies and Externs *************************************************************/


/*********************************************** Public Functions *********************************************************************/

/*
 * Initializes a semaphore to a given value
 * Param "s": Pointer to semaphore
 * Param "value": Value to initialize semaphore to
 * THIS IS A CRITICAL SECTION
 */
void G8RTOS_InitSemaphore(semaphore_t* s, int32_t value)
{
    int32_t status;
    status = StartCriticalSection();
    *s = value;
    EndCriticalSection(status);
}

/*
 * Waits for a semaphore to be available (value greater than 0)
 * 	- Decrements semaphore when available
 * 	- Spinlocks to wait for semaphore
 * Param "s": Pointer to semaphore to wait on
 * THIS IS A CRITICAL SECTION
 */
void G8RTOS_WaitSemaphore(semaphore_t* s)
{
    int32_t status;
    status = StartCriticalSection();
    *s = *s - 1;
	if(*s < 0){
	    CurrentlyRunningThread->blocked = s;
	    EndCriticalSection(status);
	    cntxt_switch();
	}
	EndCriticalSection(status);
}

/*
 * Signals the completion of the usage of a semaphore
 * 	- Increments the semaphore value by 1
 * Param "s": Pointer to semaphore to be signalled
 * THIS IS A CRITICAL SECTION
 */
void G8RTOS_SignalSemaphore(semaphore_t* s)
{
    int32_t status;
    status = StartCriticalSection();
	*s = *s + 1;
	if(*s <= 0){    //if it is not blocked
	    tcb_t* is_blocked_to_s = CurrentlyRunningThread->next;
	    while(is_blocked_to_s->blocked != s){
	        is_blocked_to_s = is_blocked_to_s -> next;
	    }
	    is_blocked_to_s->blocked = 0;
	}
	EndCriticalSection(status);
}

/*********************************************** Public Functions *********************************************************************/


