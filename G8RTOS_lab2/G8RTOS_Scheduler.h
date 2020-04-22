/*
 * G8RTOS_Scheduler.h
 */

#ifndef G8RTOS_SCHEDULER_H_
#define G8RTOS_SCHEDULER_H_
#include <pong-lab/G8RTOS_lab2/G8RTOS_Structures.h>
#include "msp.h"

#define _ICSR (*((volatile unsigned int*)(0xe000ed04)))
#define ICSR_PENDSVSET (1 << 28)

/*********************************************** Public Variables *********************************************************************/
/* Holds the current time for the whole System */
extern uint32_t SystemTime;

typedef enum sched_ErrCode_t{
    NO_ERROR = 0,
    THREAD_LIMIT_REACHED =  -1,
    NO_THREADS_SCHEDULED = -2,
    THREADS_INCORRECTLY_ALIVE =  -3,
    THREAD_DOES_NOT_EXIST =  -4,
    CANNOT_KILL_LAST_THREAD = -5,
    IRQn_INVALID = -6,
    HWI_PRIORITY_INVALID = -7
}sched_ErrCode_t;
/*********************************************** Public Variables *********************************************************************/


/*********************************************** Public Functions *********************************************************************/

/*
 * Initializes variables and hardware for G8RTOS usage
 */
void G8RTOS_Init();

/*
 * Starts G8RTOS Scheduler
 * 	- Initializes Systick Timer
 * 	- Sets Context to first thread
 * Returns: Error Code for starting scheduler. This will only return if the scheduler fails
 */
int32_t G8RTOS_Launch();

/*
 * Adds threads to G8RTOS Scheduler
 * 	- Checks if there are stil available threads to insert to scheduler
 * 	- Initializes the thread control block for the provided thread
 * 	- Initializes the stack for the provided thread
 * 	- Sets up the next and previous tcb pointers in a round robin fashion
 * Param "threadToAdd": Void-Void Function to add as preemptable main thread
 * Returns: Error code for adding threads
 */
sched_ErrCode_t G8RTOS_AddThread(void (*threadToAdd)(void),uint8_t,char*);

int32_t G8RTOS_AddPeriodic(void (*periodicToAdd)(void), uint32_t period);

sched_ErrCode_t G8RTOS_AddAPeriodicEvent(void (*AthreadToAdd)(void),uint8_t priority,IRQn_Type IRQn);

<<<<<<< HEAD

//sched_ErrCode_t G8RTOS_AddAPeriodicEvent(void (*AthreadToAdd)(void), uint16_t priority, IRQn_Type IRQn);


/* Puts the current thread into a sleep state.
 *  param durationMS: Duration of sleep time in ms */
void sleep(uint32_t durationMS);


/* Returns the currently running thread's thread ID */
=======
>>>>>>> WIFI_Bones
threadId_t G8RTOS_GetThreadId();

sched_ErrCode_t G8RTOS_KillThread(threadId_t threadId);

sched_ErrCode_t G8RTOS_KillSelf();

/*********************************************** Public Functions *********************************************************************/
//blocks thread from running
void cntxt_switch();
void sleep(uint32_t duration);
uint32_t getLCM();
#endif /* G8RTOS_SCHEDULER_H_ */
