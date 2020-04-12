#include "msp.h"
#include <driverlib.h>
#include "G8RTOS.h"
#include "threads.h"
//#include "LCDLib.h"
#include "time.h"
#include "stdlib.h"
#include "Game.h"

#define LAB5_PADDLE

//Moves a paddle left and right at bottom of the screen
#ifdef LAB5_PADDLE
void main(void)
{
    /* Initializes all the hardware resources on the board and sets the amount of threads and system time to 0 */
    G8RTOS_Init();

    LCD_Init(false);

    /* Sets up a semaphore for indicating if the LCD resource is available */
    G8RTOS_InitSemaphore(&SPI_READY, 1);

    G8RTOS_InitFIFO(0);
    G8RTOS_InitFIFO(1);
    G8RTOS_InitFIFO(2);

    /* Adds each task individually to the system */
    G8RTOS_AddThread(JoystickRead, 150, "ReadJoy");
    G8RTOS_AddThread(DrawPaddle, 151, "DrawPaddle");
    G8RTOS_AddThread(Idle, 200, "Idle");

    LCD_DrawRectangle(MAX_SCREEN_X/2 - PADDLE_LEN_D2, MAX_SCREEN_X/2 + PADDLE_LEN_D2, MAX_SCREEN_Y - PADDLE_WID, MAX_SCREEN_Y, LCD_BLUE);

    G8RTOS_Launch();
}



#endif




#ifdef LAB4_MAIN

//If I pushed this, you can read it

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
	G8RTOS_InitFIFO(0);
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

#endif /* LAB4_MAIN */

#ifdef LABQUIZ_MAIN

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
	G8RTOS_AddThread(startGame, 10, "gameStart"); // Highest priority
	G8RTOS_AddAPeriodicEvent(aperiodicTap, 3,  PORT4_IRQn);

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

#endif /* LABQUIZ_MAIN */

