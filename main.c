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

#include <DriverLib.h>
#include "BSP.h"

#define MAIN_LAB5
#ifdef MAIN_LAB5

int waitingForHost = 0x55;
int hostFlag = 0x55;

test_t winning;

void main(void){
	G8RTOS_Init();

	LCD_Init(false);

	/* For the color randomness */
	srand(time(NULL));

	/* Sets up a semaphore for indicating if the LED resource and the sensor resource are available */
	G8RTOS_InitSemaphore(&USING_SPI, 1);
	G8RTOS_InitSemaphore(&USING_LED_I2C, 1);

	/*Aperioidic Threads*/
	//Button taps
    P4->SEL1 &= ~BIT4;
    P4->SEL0 &= ~BIT4;
    P4->DIR  &= ~BIT4;
    P4->IFG  &= ~BIT4;   //Clears P4.4
    P4->IE   |= BIT4;     //Enables interrupt
    P4->IES  |= BIT4;    //High to low transition
    P4->REN  |= BIT4;    //Pull-up resistor
    P4->OUT  |= BIT4;    //Sets res to pull-up

    P5->SEL1 &= ~BIT2;
    P5->SEL0 &= ~BIT2;
    P5->DIR  &= ~BIT2;
    P5->IFG  &= ~BIT2;   //Clears P5.2
    P5->IE   |= BIT2;    //Enables interrupt
    P5->IES  |= BIT2;    //High to low transition
    P5->REN  |= BIT2;    //Pull-up resistor
    P5->OUT  |= BIT2;    //Sets res to pull-up

    G8RTOS_AddThread(WaitScreen, 1, "Wait_Screen");
    G8RTOS_AddAPeriodicEvent(TOP_BUTTON_TAP, 6, PORT4_IRQn);
    G8RTOS_AddAPeriodicEvent(BOTTOM_BUTTON_TAP, 6, PORT5_IRQn);

	G8RTOS_InitFIFO(JOYSTICKFIFO);

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

