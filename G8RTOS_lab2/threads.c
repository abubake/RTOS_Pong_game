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
#include "LCD.h"

#define THREADS

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
