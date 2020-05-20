/*
 * G8RTOS.h
 */

#ifndef G8RTOS_H_
#define G8RTOS_H_

/******************************Sizes and Limits***********************************/
#define MAX_THREADS 23
#define MAX_PERIODICS 6
#define STACKSIZE 512
#define OSSYS_PRIORITY 2
#define OSINT_PRIORITY 7
#define MAX_FIFOS 6
#define FIFO_BUFFER_SIZE 16
#define MAX_NAME_LENGTH 16
/******************************Sizes and Limits***********************************/

#include <stdint.h>
#include <hyperloop-iot/G8RTOS_lab2/G8RTOS_Semaphores.h>
#include <hyperloop-iot/G8RTOS_lab2/G8RTOS_Scheduler.h>

#endif /* G8RTOS_H_ */
