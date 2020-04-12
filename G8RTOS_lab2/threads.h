
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


#define TASKS_LAB4
#ifdef TASKS_LAB5

/* COMMON THREADS */

	/* Should hold arrays of previous players and ball positions
	• Draw and/or update balls (you’ll need a way to tell whether to draw a new ball,
	 or update its position (i.e. if a new ball has just been created – hence the alive attribute in the Ball_t struct.
	• Update players
	• Sleep for 20ms (reasonable refresh rate) */
	void DrawObjects();

	/* Responsible for updating the LED array with current scores */
	void MoveLEDS();


/* HOST THREADS */

	/* Begins the game */
	void CreateGame();

	/* Adds move ball threads based of certain criteria */
	void GenerateBall();

	/* Moves a ball */
	void MoveBall();

	/* Host reading of Joystick data */
	void ReadJoystickHost();

	/* Fills and sends a UDP packet to the client
	 * Checks if game is done */
	void SendDataToClient();

	/* Receive data continually */
	void RecieveDataFromClient();

	/* Ends the host game */
	void EndOfGameHost();


/* CLIENT THREADS */

	/* Sends player info to the host */
	void JoinGame();

	/* Reads Joystick and adds displacement */
	void ReadJoystickClient();

	/* Sends the player info to the host */
	void SendDataToHost();

	/* Receives the data from the host of game state */
	void RecieveDataFromHost();

	/* Restarts game and waits for host to start new game */
	void EndOfGameClient();

#endif /* TASKS_LAB5 */



#ifdef TASKS_LAB4

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

#endif /* TASKS_LAB4 */


#endif /* THREADS_H_ */
