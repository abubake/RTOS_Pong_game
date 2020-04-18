#include "msp.h"
#include <driverlib.h>
#include "G8RTOS.h"
#include "LCD.h"
#include "time.h"
#include "stdlib.h"
#include "cc3100_usage.h"
#include "sl_common.h"
#include "Game.h"
#include "G8RTOS_Scheduler.h"

#define MAIN_LAB5
#ifdef MAIN_LAB5

int waitingForHost;
int hostFlag;

void main(void){


	 P4DIR &= ~BIT4;
	 P4IFG &= ~BIT4; // clear interrupt flag bit
	 P4IE |= BIT4; // enable interrupt on pin 4
	 P4IES |= BIT4; //enables low to high transition
	 P4REN |= BIT4; // pull up resistor
	 P4OUT |= BIT4; // sets resistor to pull up

	    /*Freed from loop once choice is made */
	    while(waitingForHost){
	    	if(hostFlag){
	    	/* Setting this to be the host WIFI */
	    		initCC3100(Host);
	    	}
	    }
	/* Initializes all the hardware resources on the board and sets the amount of threads and system time to 0 */
	G8RTOS_Init();


	LCD_Init(true);

	/* For the color randomness */
	srand(time(NULL));

 /* Sets up a semaphore for indicating if the LED resource and the sensor resource are available */
	G8RTOS_InitSemaphore(&USING_SPI, 1);

	/* Adds each task individually to the system */
	G8RTOS_AddThread(CreateGame, 150, "CreateGame"); //NEEDS real PRI and maybe better nam
	//G8RTOS_AddAPeriodicEvent(LCD_Tap, 3,  PORT4_IRQn);

	G8RTOS_InitFIFO(JOYSTICKFIFO);

	/* Initializes the Systick to trigger every 1ms and sets the priority for both PendSV and Systick  */
	/* Sets the first thread control block as the current thread, and calls the start_os assembly function */
	G8RTOS_Launch();
}

void PORT4_IRQHandler(void){
        P4IE &= ~BIT4; // disables interrupt on pin 4
        //P4IFG &= ~BIT4; // must clear IFG flag to operate
        hostFlag = 1; //We exit the while loop and we are set as the host
        waitingForHost = 1;
        //P4IE |= BIT4; // no need to enable interrupt on pin 4 since we don't need it again
    }



#endif /* MAIN_LAB5 */


#ifdef MAIN_LAB4

void main(void)
	{
	/* Initializes all the hardware resources on the board and sets the amount of threads and system time to 0 */
	G8RTOS_Init();

	LCD_Init(true);
	/* For the color randomness */
	srand(time(NULL));

 /* Sets up a semaphore for indicating if the LED resource and the sensor resource are available */
	G8RTOS_InitSemaphore(&SENSOR_READY, 1);
	G8RTOS_InitSemaphore(&SPI_READY, 1);

	/* Adds each task individually to the system */
	G8RTOS_AddThread(Read_Accel, 150, "ReadAccel");
	G8RTOS_AddThread(Idle, 200, "Idle");

	G8RTOS_AddAPeriodicEvent(LCD_Tap, 3,  PORT4_IRQn);

	G8RTOS_InitFIFO(BALLFIFO);
	G8RTOS_InitFIFO(BALLNUMFIFO);

	/* Initializes the Systick to trigger every 1ms and sets the priority for both PendSV and Systick  */
	/* Sets the first thread control block as the current thread, and calls the start_os assembly function */
	G8RTOS_Launch();
}

void PORT4_IRQHandler(void){
        P4IE &= ~BIT0; // disables interrupt on pin 4
        P4IFG &= ~BIT0; // must clear IFG flag to operate
        P4IE |= BIT0; // enable interrupt on pin 4
    }

#endif /* MAIN_LAB4 */

