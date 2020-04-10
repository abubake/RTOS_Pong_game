/*
 * G8RTOS_Semaphores.h
 */

#ifndef G8RTOS_SEMAPHORES_H_
#define G8RTOS_SEMAPHORES_H_

/*********************************************** Datatype Definitions *****************************************************************/

/*
 * Semaphore typedef
 */
typedef int32_t semaphore_t;

/*********************************************** Datatype Definitions *****************************************************************/


/*********************************************** Public Functions *********************************************************************/

/* Initializes a semaphore to a given value
 * Param "s": Pointer to semaphore
 * Param "value": Value to initialize semaphore to */
void G8RTOS_InitSemaphore(semaphore_t *s, int32_t value);

/* No longer waits for semaphore
 *  - Decrements semaphore
 *  - Blocks thread if sempahore is unavalible
 * Param "s": Pointer to semaphore to wait on
 * THIS IS A CRITICAL SECTION */
void G8RTOS_WaitSemaphore(semaphore_t *s);

/* Signals the completion of the usage of a semaphore
 *  - Increments the semaphore value by 1
 *  - Unblocks any threads waiting on that semaphore
 * Param "s": Pointer to semaphore to be signaled
 * THIS IS A CRITICAL SECTION */
void G8RTOS_SignalSemaphore(semaphore_t *s);

void G8RTOS_WaitSemaphoreMutex(semaphore_t *s);

void G8RTOS_SignalSemaphoreMutex(semaphore_t *s);

/*********************************************** Public Functions *********************************************************************/


#endif /* G8RTOS_SEMAPHORES_H_ */
