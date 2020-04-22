#include "msp.h"
#include <driverlib.h>
#include "core_cm4.h"
#include "G8RTOS.h"
#include "LCD.h"
#include "time.h"
#include "stdlib.h"
#include "cc3100_usage.h"
#include "sl_common.h"
#include "Game.h"
#include "G8RTOS_Scheduler.h"
#include "demo_sysctl.h"

#include <DriverLib.h>
#include "BSP.h"

#define MAIN_LAB5
#ifdef Test_LAB5
void main(void){
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;

    //Button taps

    G8RTOS_Init();

    LCD_Init(false);

    //playerType gameRole = GetPlayerRole();

    initCC3100(Host);

    LCD_Text(120, (MAX_SCREEN_Y >> 1) - 10, "Connecting", LCD_WHITE);

    while(1);
}

#endif



#ifdef MAIN_LAB5

void main(void){
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;

    G8RTOS_Init();

    /* Sets up a semaphore for indicating if the LED resource and the sensor resource are available */
    G8RTOS_InitSemaphore(&USING_SPI, 1);
    G8RTOS_InitSemaphore(&USING_LED_I2C, 1);
    G8RTOS_InitSemaphore(&USING_WIFI, 1);

    LCD_Init(false);

    LCD_Text(50, (MAX_SCREEN_Y >> 1) - 20, "Client is top button", LCD_BLUE);
    LCD_Text(50, (MAX_SCREEN_Y >> 1) + 20, "Host is right button", LCD_RED);

    playerType gameRole = GetPlayerRole();
    LCD_Text(150, (MAX_SCREEN_Y >> 1) - 10, "Connecting", LCD_WHITE);
    initCC3100(gameRole);

	/* For the color randomness */
	srand(time(NULL));

	//Add the appropriate starter thread for the chosen role
	if(gameRole == Host){
	    G8RTOS_AddThread(CreateGame, 1, "genesis");
	}
	else if(gameRole == Client){
	    G8RTOS_AddThread(JoinGame, 1, "genesis");
	}

	/* Initializes the Systick to trigger every 1ms and sets the priority for both PendSV and Systick  */
	/* Sets the first thread control block as the current thread, and calls the start_os assembly function */
	G8RTOS_Launch();

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

