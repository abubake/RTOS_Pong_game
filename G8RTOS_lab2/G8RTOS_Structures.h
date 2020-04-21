/*
 * G8RTOS_Structure.h
 *
 *  Created on: Jan 12, 2017
 *      Author: Raz Aloni
 */

#ifndef G8RTOS_STRUCTURES_H_
#define G8RTOS_STRUCTURES_H_

#include <pong-lab/G8RTOS_lab2/G8RTOS.h>
#include <pong-lab/G8RTOS_lab2/G8RTOS_Semaphores.h>
#include <stdbool.h>

/*
 * threadID typedef
 */
typedef uint32_t threadId_t;

/*********************************************** Data Structure Definitions ***********************************************************/

/*
 *  Thread Control Block:
 *      - Every thread has a Thread Control Block
 *      - The Thread Control Block holds information about the Thread Such as the Stack Pointer, Priority Level, and Blocked Status
 *      - For Lab 2 the TCB will only hold the Stack Pointer, next TCB and the previous TCB (for Round Robin Scheduling)
 */

/*
 *  Periodic Thread:
 *      - Every periodic thread has a Periodic Block
 *      - The Thread Control Block holds information about the Periodic Thread Such as the Execution Time, Current Time etc
 */

/*
 *  FIFO:
 *      - Used for async comms between threads (inter-process comm)
 */

/*********************************************** Data Structure Definitions ***********************************************************/
typedef struct tcb_t{
    int32_t* sp;
    struct tcb_t* next;
    struct tcb_t* prev;

    semaphore_t* blocked;   //1 = blocked & 0=open

    uint8_t asleep; //1 = asleep & 0=awake
    uint32_t sleep_cnt; //duration of sleep

    uint8_t priority;
    bool isAlive;
    threadId_t threadID;
    char threadName[MAX_NAME_LENGTH];

}tcb_t;

typedef struct pt_t{
    void(*handler)(void);    //function pointer

    uint32_t period;   //how often it occurs
    uint32_t exec_time; //burst length
    uint32_t cur_time;  //current time

    //to next and prev
    struct pt_t* next;
    struct pt_t* prev;
}pt_t;

typedef struct fifo_t{
    int32_t buf[FIFO_BUFFER_SIZE];    //buffer size

    //to head and tail
    int32_t* head;
    int32_t* tail;

    uint32_t lost_cnt;   //lost data count

    //semaphores
    semaphore_t size;
    semaphore_t mutex; //may not need this if a bit-band
}fifo_t;

/*********************************************** Public Variables *********************************************************************/

tcb_t* CurrentlyRunningThread;
tcb_t* threads[MAX_THREADS];

/*********************************************** Public Variables *********************************************************************/




#endif /* G8RTOS_STRUCTURES_H_ */
