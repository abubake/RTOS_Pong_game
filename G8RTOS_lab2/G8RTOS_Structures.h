/*
 * G8RTOS_Structure.h
 *
 *  Created on: Jan 12, 2017
 *      Author: Raz Aloni
 */

#ifndef G8RTOS_STRUCTURES_H_
#define G8RTOS_STRUCTURES_H_

#include "G8RTOS.h"

/*********************************************** Data Structure Definitions ***********************************************************/

/*
 *  Thread Control Block:
 *      - Every thread has a Thread Control Block
 *      - The Thread Control Block holds information about the Thread Such as the Stack Pointer, Priority Level, and Blocked Status
 *      - For Lab 2 the TCB will only hold the Stack Pointer, next TCB and the previous TCB (for Round Robin Scheduling)
 */

/* Create tcb struct here */
typedef struct tcb_t{

	int32_t *Stack_pointer;
    struct tcb_t *Next;
    struct tcb_t *Previous;
    semaphore_t *blocked;

    bool isAlive;
    threadId_t threadID;
    char threadName[MAX_NAME_LENGTH];
    uint16_t priority;

    bool asleep;
    uint32_t sleepCount;
}tcb_t;


/*
 *  Periodic Thread Control Block:
 *      - Holds a function pointer that points to the periodic thread to be executed
 *      - Has a period in us
 *      - Holds Current time
 *      - Contains pointer to the next periodic event - linked list
 */
typedef struct ptcb_t{

		void(*Handler)(void); //function pointer to periodic event handler
		uint32_t Period;
		uint32_t executionTime; // same thing as burst length

		uint32_t currentTime; // not intially asked for in lab 3 manual

		struct ptcb_t *Next;
		struct ptcb_t *Previous;

}ptcb_t;


/*********************************************** Data Structure Definitions ***********************************************************/


/*********************************************** Public Variables *********************************************************************/

tcb_t * CurrentlyRunningThread;


/*********************************************** Public Variables *********************************************************************/




#endif /* G8RTOS_STRUCTURES_H_ */
