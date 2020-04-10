
#ifndef THREADS_H_
#define THREADS_H_

#include <stdbool.h>
#include <stdint.h>
#include <msp.h>
#include "G8RTOS.h"

	semaphore_t LED_READY;
	semaphore_t SENSOR_READY;
	semaphore_t SPI_READY; // semaphore for SPI interface using EUSCIB3 with LCD/ Touchpad
	/* There is also a blocked semaphore pointer */

	typedef enum FIFO {
	    BALLFIFO = 0,
		BALLNUMFIFO = 0
	}fifo_desig;

	typedef struct balls_t{
		int16_t xPos;
		int16_t yPos;
		int16_t speed;
		bool alive;
		threadId_t threadID;
		uint16_t color;
	}balls_t;

	/* For Snake game */
	typedef enum DIRECTION {
		UP = 0,
		RIGHT = 1,
		DOWN = 2,
		LEFT = 3
	}direction_desig;

	typedef struct snake_t{
		int x;
		int y;
		int dir;
	}snake_t;


#define TASKS_QUIZ
#ifdef TASKS

	/* Counter */
  void Read_Accel();

    /* Counter */
  void Wait_For_Tap();

    /* Counter */
  void Ball_Thread();

  void Task4();

  void Idle();

  /* Periodic Tasks */
  void PTask0();

  void PTask1();

  /* A-Periodic Tasks */
  void LCD_Tap();

#endif /* TASKS */

#ifdef TASKS_QUIZ

  void startGame();

  void snakeAdd();

  void snakeSub();

  void idle();

  void Joystick();

  void aperiodicTap();

  /* FUNCTIONS   */
 inline void updateDirection();

#endif /* TASKS_QUIZ */

#endif /* THREADS_H_ */
