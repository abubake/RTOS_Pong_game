/*
 * threads.c
 *
 *  Created on: Feb 11, 2020
 *      Author: baker
 */
#include "threads.h"
#include <driverlib.h>
#include "G8RTOS_Semaphores.h"
#include "G8RTOS.h"
#include <BSP.h>
#include <stdlib.h>
#include "helpers.h" //figure out why this was needed vs. just including driver lib to work
#include "LCDLib.h"

#define THREADS_QUIZ

#ifdef THREADS

 /* New variables */
 int16_t accelerometerX = 0;
 int16_t accelerometerY = 0;
 bool lcdTouched = false;
 int32_t xVal = 0;
 int32_t yVal = 0;
 balls_t myBalls[20];
 bool freshDead = false;
 int16_t numBalls = 0;


void Read_Accel(void){
while(1){

	G8RTOS_WaitSemaphore(&SENSOR_READY);
	bmi160_read_accel_x(&accelerometerX);
	G8RTOS_SignalSemaphore(&SENSOR_READY);

	G8RTOS_WaitSemaphore(&SENSOR_READY);
	bmi160_read_accel_y(&accelerometerY);
	G8RTOS_SignalSemaphore(&SENSOR_READY);
	sleep(5);
}
    }

void Wait_For_Tap(void){
		freshDead = false;
		/* read touch coordinates */

		G8RTOS_WaitSemaphore(&SPI_READY);
		xVal = TP_ReadX(); //352
		yVal = TP_ReadY(); //224
		G8RTOS_SignalSemaphore(&SPI_READY);

		if ((xVal != 0)&&(yVal != 0)){

		int32_t temp0 = 0;
		temp0 = xVal*(MAX_SCREEN_X - 1);
		xVal = temp0 / 4096; //3552
		int32_t temp1 = 0;
		temp1 = yVal*(MAX_SCREEN_Y - 1);
		yVal = temp1 / 4096;



		threadId_t ballToKill;
		/* Kills if with a % error of same position as a current ball */
		for (uint32_t i=0; i < 20; i++){
			if((myBalls[i].alive == true)&&(xVal <= myBalls[i].xPos + 10)&&(xVal >= myBalls[i].xPos - 10)&&(yVal <= myBalls[i].yPos + 10)&&(yVal >= myBalls[i].yPos - 10)){
				numBalls--;
				ballToKill = myBalls[i].threadID;
				myBalls[i].alive = false;
				G8RTOS_WaitSemaphore(&SPI_READY);
				LCD_DrawRectangle(myBalls[i].xPos, myBalls[i].xPos+4, myBalls[i].yPos, myBalls[i].yPos + 4, LCD_BLACK);
				G8RTOS_KillThread(ballToKill);
				G8RTOS_SignalSemaphore(&SPI_READY);
				freshDead = true;
				break;
			}
		}

		 /* Create if none were killed or if there are no errors */
		if((freshDead == false)&&(numBalls < 20)&&(xVal != 0)&&(yVal != 0)){
			numBalls++;
			int i;
			//CRITICAL SECTION
			for(i = 0; i < 20; i++){
				if(myBalls[i].alive == false){
					myBalls[i].xPos = xVal;
					myBalls[i].yPos = yVal;
					myBalls[i].alive = true;
					break;
				}
			}
			writeFIFO(BALLFIFO, i);
			G8RTOS_AddThread(Ball_Thread, 100, "BallThread");
			}

		}
		sleep(500); // 500 ms delay to account for screen bouncing
		BITBAND_PERI(P4->IFG, 0) = 0;
		BITBAND_PERI(P4->IE, 0) = 1;
		G8RTOS_KillSelf();
		while(1); // Prevents us from going to the default handler
}


void Ball_Thread(void){
	/* finds dead ball and makes it alive */

	int ballNumber = readFIFO(BALLFIFO);
	myBalls[ballNumber].threadID = G8RTOS_GetThreadId();

	/* Generating a random color for each ball */
	int randNum = (rand() % 3) + 1;
	if(randNum == 1){
		myBalls[ballNumber].color = LCD_ORANGE;
	}
	else if(randNum == 2){
		myBalls[ballNumber].color = LCD_YELLOW;
	}
	else{
		myBalls[ballNumber].color = LCD_WHITE;
	}

	/* Getting a random speed */
	myBalls[ballNumber].speed = (rand() % 3) + 1;

while(1){
	bool negX = false;
	bool negY = false;

	/* Setting flags to for which direction the ball is to go */
	int Xaccel = accelerometerX;
	if (Xaccel < 0)
	   {
	     Xaccel = abs(Xaccel);
	     negX = true;
	     }
	else
	    {
	      negX = false;
	     }
	 int Yaccel = accelerometerY;

	 if (Yaccel < 0)
	    {
	     Yaccel = abs(Yaccel);
	     negY = true;
	     }
	 else
	     {
	      negY = false;
	      }


	G8RTOS_WaitSemaphore(&SPI_READY);
	LCD_DrawRectangle(myBalls[ballNumber].xPos, myBalls[ballNumber].xPos + 4, myBalls[ballNumber].yPos, myBalls[ballNumber].yPos + 4, LCD_BLACK);
	G8RTOS_SignalSemaphore(&SPI_READY);

	/* Adding weighting factor to ball's position based on speed */

	 if (negX){
	   myBalls[ballNumber].xPos = myBalls[ballNumber].xPos - myBalls[ballNumber].speed;
	 }
	 else{
	   myBalls[ballNumber].xPos = myBalls[ballNumber].xPos + myBalls[ballNumber].speed;
	 }

	 if (!negY){
	   myBalls[ballNumber].yPos = myBalls[ballNumber].yPos - myBalls[ballNumber].speed;
	 }
	 else{
	   myBalls[ballNumber].yPos = myBalls[ballNumber].yPos + myBalls[ballNumber].speed;
	 }

	/* Checking Ball bounds */
	 if (myBalls[ballNumber].xPos < 0){
		 myBalls[ballNumber].xPos = 316;}
	 else if(myBalls[ballNumber].xPos > 316){
		 myBalls[ballNumber].xPos = myBalls[ballNumber].xPos - 316; }

	 if(myBalls[ballNumber].yPos < 0){
		 myBalls[ballNumber].yPos = 236;}
	 else if(myBalls[ballNumber].yPos > 236){
		 myBalls[ballNumber].yPos = myBalls[ballNumber].yPos - 236;}


	G8RTOS_WaitSemaphore(&SPI_READY);
	LCD_DrawRectangle(myBalls[ballNumber].xPos, myBalls[ballNumber].xPos + 4, myBalls[ballNumber].yPos, myBalls[ballNumber].yPos + 4, myBalls[ballNumber].color);
	G8RTOS_SignalSemaphore(&SPI_READY);
	sleep(30);
}
        }


void Idle(void){
	while(1);
}


void LCD_Tap(void){
	BITBAND_PERI(P4->IE, 0) = 0; // disables interrupt on pin 4
	G8RTOS_AddThread(Wait_For_Tap, 50, "WaitForit");
}


#endif /* THREADS */

#ifdef THREADS_QUIZ

/* LCD Info */
bool tapped = false;

/* Joystick Info */
int16_t X_coord;
int16_t Y_coord;
int direction;

int16_t xVal;
int16_t yVal;

/* Snake Info */
snake_t mySnake[256];
int  head = 50;
int  tail = 0;
int index = 0; // How many total are in the buffer

int count = 0;

int32_t snakeSubID = 0;
int32_t snakeAddID = 0;
int32_t JoystickID = 0;
int32_t idleID = 0;

bool wrapped = false;
int doneGrowing = false;
int prev;
int dirFlag = RIGHT;
//int PJoystickID = 0; //Trying to not kill periodic at first

void startGame(){
	/* Kill all other threads */
	/* Returns threadID DNE the first time through since they aren't made yet */
	G8RTOS_KillThread(snakeSubID);
	G8RTOS_KillThread(snakeAddID);
	G8RTOS_KillThread(JoystickID);
	G8RTOS_KillThread(idleID);
	/* I don't kill aperiodic thread */

/* Re-initing the snake */
	for(int i = 0; i < 256; i++){
		mySnake[i].x = 0;
		mySnake[i].y = 0;
	}
	index = 0;
	head = 0;
	tail = 0;

	LCD_Text(110, 100, "Waiting for tap", LCD_WHITE);

	while(1){
		if(tapped == true){
			tapped = false;
			/* Clear screen */
			LCD_Clear(LCD_BLACK);
			/* Create Threads Needed */
			G8RTOS_AddThread(snakeAdd, 172, "add");
			G8RTOS_AddThread(snakeSub, 175, "sub");
			G8RTOS_AddThread(Joystick, 170, "joystick");
			G8RTOS_AddThread(idle, 200, "idle");

			/* Setting up and drawing first snake block*/
			mySnake[0].x = 10;
			mySnake[0].y = 100;

			G8RTOS_WaitSemaphore(&SPI_READY);
			LCD_DrawRectangle(mySnake[50].x, mySnake[50].x + 2, mySnake[50].y, mySnake[50].y + 2, LCD_WHITE);
			G8RTOS_SignalSemaphore(&SPI_READY);

			BITBAND_PERI(P4->IFG, 0) = 0;
			BITBAND_PERI(P4->IE, 0) = 1;
			G8RTOS_KillSelf();
		}
	}
}

void snakeAdd(){
	snakeAddID = G8RTOS_GetThreadId();
while(1){
	head++;
	index++;
	if((head > 256)&&(index <= 256)){
		head = 0; //(so that it will be at pos. 0);
	}
	else if(index > 256){ //Subtract so that there is 0 change;
		doneGrowing = true;
	}

if(doneGrowing == false){
	/* STOPS SNAKE FROM GOING BACK ON ITSELF, making it go the previous direction instead */
	prev = head - 1;
	if(head == 0){
		prev = 256;
	}

	if (((mySnake[prev].dir == LEFT) && (direction == RIGHT))||((mySnake[prev].dir == RIGHT) && (direction == LEFT))){// if previous node went left and you want to go right
		direction = mySnake[prev].dir;
	}
	else if (((mySnake[prev].dir == UP) && (direction == DOWN))||((mySnake[prev].dir == DOWN) && (direction == UP))){
		direction = mySnake[prev].dir;
	}

	/* NEED TO HANDLE IF  IT HITS ITSELF */
	/* Updates the current head's data and draws the new node accordingly based on "direction" */
	if(direction == RIGHT){ //RIGHT
		if(mySnake[prev].x == 318){
			mySnake[head].x = 2;
			mySnake[head].y = mySnake[prev].y;
			mySnake[head].dir = RIGHT;
		}
		else{
			mySnake[head].x = mySnake[prev].x + 2;
			mySnake[head].y = mySnake[prev].y;
			mySnake[head].dir = RIGHT;
		}
		}
	else if(direction == LEFT){ //LEFT
		if(mySnake[prev].x == 2){//IF X TOO FAR LEFT
			mySnake[head].x = 318;
			mySnake[head].y = mySnake[prev].y;
			mySnake[head].dir = LEFT;
		}
		else{//REGULAR CASE
			mySnake[head].x = mySnake[prev].x - 2;
			mySnake[head].y = mySnake[prev].y;
			mySnake[head].dir = LEFT;
		}
		}
	else if(direction == UP){ //UP
		if(mySnake[prev].y == 238){
			mySnake[head].x = mySnake[prev].x;
			mySnake[head].y = 2;
			mySnake[head].dir = UP;
		}
		else{
			mySnake[head].x = mySnake[prev].x;
			mySnake[head].y = mySnake[prev].y + 2;
			mySnake[head].dir = UP;
		}
		}
	else if(direction== DOWN){ //DOWN
		if(mySnake[prev].y == 2){
			mySnake[head].x = mySnake[prev].x;
			mySnake[head].y = 238;
			mySnake[head].dir = DOWN;
		}
		else{
			mySnake[head].x = mySnake[prev].x;
			mySnake[head].y = mySnake[prev].y - 2;
			mySnake[head].dir = DOWN;
		}
		}

	/*Crash detection */
	/*
	for(uint16_t i = 0; i < 256; i++){
		if((mySnake[i].y != 0)&&(mySnake[i].x != 0)&&(mySnake[head].x == mySnake[i].x)&&(mySnake[head].y == mySnake[i].y)){
			LCD_Clear(LCD_BLACK);
			G8RTOS_AddThread(startGame, 10, "gameStart");
		}
	}
	*/

	G8RTOS_WaitSemaphore(&SPI_READY);
	LCD_DrawRectangle(mySnake[head].x, mySnake[head].x + 2, mySnake[head].y, mySnake[head].y + 2, LCD_WHITE);
	G8RTOS_SignalSemaphore(&SPI_READY);
	count++;
	}//end of if
sleep(30);
}
}

void snakeSub(){
	snakeSubID = G8RTOS_GetThreadId();
while(1){
	/* Erase old tail, increment tail */
	if(doneGrowing == false){
	G8RTOS_WaitSemaphore(&SPI_READY);
	LCD_DrawRectangle(mySnake[tail].x, mySnake[tail].x + 2, mySnake[tail].y, mySnake[tail].y + 2, LCD_BLACK);
	G8RTOS_SignalSemaphore(&SPI_READY);

	if(count == 2){
		count = 0;
		tail++;
		index--;
			if(tail > 256){
				tail = 0;
				}
		}
	}//end of if
	sleep(60);
}
}

void idle(){
	idleID = G8RTOS_GetThreadId();
while(1);
}

void Joystick(){
	JoystickID = G8RTOS_GetThreadId();

while(1){
	GetJoystickCoordinates(&X_coord, &Y_coord);

	if(X_coord > 1000){
		direction = RIGHT;
	}
	else if(X_coord < -1000){
		direction = LEFT;
	}
	else if(Y_coord > 1000){
		direction = UP;
	}
	else if(Y_coord < -1000){
		direction = DOWN;
	}
	sleep(5);
}

}

void aperiodicTap(){
	BITBAND_PERI(P4->IE, 0) = 0; // disables interrupt on pin 4
	tapped = true;
	/* Need to reenable somewhere */
}

/* FUNCTIONS */
inline void updateDirection(){
							if(direction == RIGHT){ //RIGHT
							mySnake[head].x = mySnake[head-1].x + 2;
							mySnake[head].y = mySnake[head-1].y;
							mySnake[head].dir = RIGHT;
							}
							else if(direction == LEFT){ //LEFT
							mySnake[head].x = mySnake[head-1].x - 2;
							mySnake[head].y = mySnake[head-1].y;
							mySnake[head].dir = LEFT;
							}
							else if(direction == UP){ //UP
							mySnake[head].x = mySnake[head-1].x;
							mySnake[head].y = mySnake[head-1].y + 2;
							mySnake[head].dir = UP;
							}
							else if(direction== DOWN){ //DOWN
							mySnake[head].x = mySnake[head-1].x;
							mySnake[head].y = mySnake[head-1].y - 2;
							mySnake[head].dir = DOWN;
							}
}


#endif /* THREADS_QUIZ */
