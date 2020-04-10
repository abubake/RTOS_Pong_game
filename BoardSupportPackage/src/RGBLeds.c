/*
 * RGBLeds.c
 *
 *  Created on: Jan 20, 2020
 *      Author: baker
 */
#include <driverlib.h>
#include <RGBLeds.h>
#include "helpers.h"
#include "stdio.h"


/* LP3943 ColorSet
 * Status: NOT IMPLEMENTED
 * This function will set the frequencies and PWM duty cycle
 * for each register of the specified unit
 * Parameters: Color of the LED and which driver it is, and the PWM data
 * Returns: void */
static void LP3943_ColorSet(uint32_t unit, uint32_t PWM_DATA);


/* Status: IN USE
 * This function will set each of the LEDs to the desired operating mode.
 * The operating modes are on, off.
 * Parameters: unit is whether we are using R,G, or B LP3943, and LED_DATA is the data being sent/ written
 * Returns: Void*/
void LP3943_LedModeSet(uint32_t unit, uint16_t LED_DATA, uint8_t LS_addy){
    uint8_t SlaveAddr = 0;

    if (unit == BLUE){
    SlaveAddr = 0x60; //slave base address (1st LP3943)
    }
    else if (unit == GREEN){
    SlaveAddr = 0x61; //slave base address (2nd LP3943)
    }
    else {
    SlaveAddr = 0x62; //slave base address (3rd LP3943)
    }
    UCB2I2CSA = SlaveAddr;
    UCB2CTLW0 |= 0x0010; // enable transmitter
    UCB2CTLW0 |= UCTXSTT;
    while((UCB2CTLW0 & 2));
    UCB2TXBUF = LS_addy;
    while(!(UCB2IFG & 2));

/******************************************************************************************************/
    if(LED_DATA == 3){
        LED_DATA = 0b00000101;
    }
    if(LED_DATA == 5){
            LED_DATA = 0b00010001;
        }
    if(LED_DATA == 6){
            LED_DATA = 0b00010100;
        }
    if(LED_DATA == 7){
            LED_DATA = 0b00010101;
        }
    if(LED_DATA == 9){
            LED_DATA = 0b01000001;
        }
    if(LED_DATA == 0x000A){
            LED_DATA = 0b01000100;
        }
    if(LED_DATA == 0x000B){
            LED_DATA = 0b01000101;
        }
    if(LED_DATA == 0x000C){
            LED_DATA = 0b01010000;
        }
    if(LED_DATA == 0x000D){
            LED_DATA = 0b01010001;
        }
    if(LED_DATA == 0x000E){
            LED_DATA = 0b01010100;
        }
    if(LED_DATA == 0x000F){
            LED_DATA = 0b01010101;
        }

    if(LED_DATA == 8){
        LED_DATA = 0b01000000;
    }
    if(LED_DATA == 4){
        LED_DATA = 0b00010000;
    }
    if(LED_DATA == 2){
            LED_DATA = 0b00000100;
        }
/******************************************************************************************************/

UCB2TXBUF = LED_DATA;
while(!(UCB2IFG & 2)); // wait for data to send
UCB2CTLW0 |= UCTXSTP;
while((UCB2CTLW0 & 4)); // wait for STOP to send

}


/* STATUS: IN USE
 * Performs needed initializations to use I2C on UCB2
 * Turns all LEDS off, does a required software reset to stop operation; initializing the slave master, I2C mode, clock synch, etc.
 * Set the clock pre-scaler, determining how much the LEDs will update;
 * Set the P3 pins for I2C mode
 * Re-enables operation after initializations are done (coming out of reset mode)
 * Sets all LEDs to be off initially
 * Parameters: NONE
 * Returns: NONE */
void init_RGBLEDS()
{
    uint16_t UNIT_OFF = 0x0000;
    UCB2CTLW0 = UCSWRST;
    UCB2CTLW0 |= UCSSEL_2 | UCMST | UCMODE_3 | UCSYNC;
    UCB2BRW = 30; // 8 for 3MHz, 30 for max frequency
    P3SEL0 |= 0xC0;
    P3SEL1 &= ~0xC0;

    UCB2CTLW0 &= ~UCSWRST;

    /* Init all LED's as off */
    LP3943_LedModeSet(RED, UNIT_OFF, 0x06);
    LP3943_LedModeSet(GREEN, UNIT_OFF, 0x06);
    LP3943_LedModeSet(BLUE, UNIT_OFF, 0x06);

    LP3943_LedModeSet(RED, UNIT_OFF, 0x07);
    LP3943_LedModeSet(GREEN, UNIT_OFF, 0x07);
    LP3943_LedModeSet(BLUE, UNIT_OFF, 0x07);

    LP3943_LedModeSet(RED, UNIT_OFF, 0x08);
    LP3943_LedModeSet(GREEN, UNIT_OFF, 0x08);
    LP3943_LedModeSet(BLUE, UNIT_OFF, 0x08);

    LP3943_LedModeSet(RED, UNIT_OFF, 0x09);
    LP3943_LedModeSet(GREEN, UNIT_OFF, 0x09);
    LP3943_LedModeSet(BLUE, UNIT_OFF, 0x09);
}

/* The following 4 functions turn off all LEDs of a specified color for all 4 LED drivers */
void REDLED_OFF(){
    uint16_t UNIT_OFF = 0x0000;
    LP3943_LedModeSet(RED, UNIT_OFF, 0x06);
    LP3943_LedModeSet(RED, UNIT_OFF, 0x07);
    LP3943_LedModeSet(RED, UNIT_OFF, 0x08);
    LP3943_LedModeSet(RED, UNIT_OFF, 0x09);
}

void GREENLED_OFF(){
    uint16_t UNIT_OFF = 0x0000;
    LP3943_LedModeSet(GREEN, UNIT_OFF, 0x06);
    LP3943_LedModeSet(GREEN, UNIT_OFF, 0x07);
    LP3943_LedModeSet(GREEN, UNIT_OFF, 0x08);
    LP3943_LedModeSet(GREEN, UNIT_OFF, 0x09);
}

void BLUELED_OFF(){
    uint16_t UNIT_OFF = 0x0000;
    LP3943_LedModeSet(BLUE, UNIT_OFF, 0x06);
    LP3943_LedModeSet(BLUE, UNIT_OFF, 0x07);
    LP3943_LedModeSet(BLUE, UNIT_OFF, 0x08);
    LP3943_LedModeSet(BLUE, UNIT_OFF, 0x09);
}


inline void displayLight(uint16_t lightData){
	if (lightData > 0){
		if (lightData > 26000){
			  if (lightData > 42000){
				  if((lightData > 42000)&&(lightData < 45500)){
					  number_display(65532, GREEN);
				  }
					  else if((lightData < 49000)&&(lightData > 45500)){
					  number_display(65534, GREEN);
					  }
					  else if((lightData > 49000)){
				  	  number_display(65535, GREEN);
					  }
			  }


			  else {
				  if((lightData < 42000)&&(lightData > 38500)){
				  		number_display(65528, GREEN);
				  }
				  		else if((lightData < 42000)&&(lightData > 35500)){
				  		number_display(65520, GREEN);
				  		}
				  		else if((lightData > 31500)&&(lightData < 35500)){
				  		number_display(65504, GREEN);
				  		}
				  		else if((lightData < 31500)&&(lightData > 28000)){
				  		number_display(65472, GREEN);
				  		}
				  		else if((lightData > 26000)&&(lightData < 28000)){
				  		number_display(65408, GREEN);
				  		}
			  }
		}
		else{ /* For cases below the halfway threshold */

			if (lightData > 14000){
				if((lightData > 14000)&&(lightData < 17500)){
					number_display(63488, GREEN);
				}
					else if((lightData > 17500)&&(lightData < 21000)){
					number_display(64512, GREEN);
					}
					else if((lightData > 21000)&&(lightData < 24500)){
					number_display(65024, GREEN);
					}
					else if((lightData > 24500)&&(lightData < 26000)){
					number_display(65280, GREEN);
					}
			}
			else{
				if((lightData > 10500)&&(lightData < 14000)){
					number_display(61440, GREEN); //0xF000
				}
					else if((lightData > 7000)&&(lightData < 10500)){
					number_display(57344, GREEN); //0xE000
					}
					else if((lightData > 3500)&&(lightData < 7000)){
					number_display(49152, GREEN); //0xC000
					}
					else if((lightData < 3500)){
					number_display(32768, GREEN); //0x8000
					}
			}


		}

		}
}

inline void displayAcceleration(int16_t acceleration){
	int16_t data = acceleration;
	uint16_t color = RED;
	if(data > 0){
				if((data > 14000)){
					number_display(65280, color);
				}
					else if((data > 12000)&&(data < 14000)){
					number_display(32512, color);
					}
					else if((data > 10000)&&(data < 12000)){
					number_display(16128, color);
					}
					else if((data > 8000)&&(data < 10000)){
					number_display(7936, color);
					}
					else if((data > 6000)&&(data < 8000)){
					number_display(3840, color);
					}
					else if((data > 4000)&&(data < 6000)){
					number_display(1792, color);
					}
					else if((data > 2000)&&(data < 4000)){
					number_display(768, color);
					}
					else if((data > 0)&&(data < 2000)){
					number_display(256, color);
					}
	}
	else{
				/*the negatives */
				if((data < -14000)){
					number_display(255, color);
				}
					else if((data < -12000)&&(data > -14000)){
					number_display(254, color);
					}
					else if((data < -10000)&&(data > -12000)){
					number_display(252, color);
					}
					else if((data < -8000)&&(data > -10000)){
					number_display(248, color);
					}
					else if((data < -6000)&&(data > -8000)){
					number_display(240, color);
					}
					else if((data < -4000)&&(data > -6000)){
					number_display(224, color);
					}
					else if((data < -2000)&&(data > -4000)){
					number_display(192, color);
					}
					else if((data < 0)&&(data > -2000)){
					number_display(128, color);
					}
			}
}


inline void displayGyro(int16_t gyro){
	int16_t data = gyro;
	uint16_t color = BLUE;
	if(data > 0){
				if((data > 7000)){
					number_display(65280, color);
				}
					else if((data > 6000)&&(data < 7000)){
					number_display(32512, color);
					}
					else if((data > 5000)&&(data < 6000)){
					number_display(16128, color);
					}
					else if((data > 4000)&&(data < 5000)){
					number_display(7936, color);
					}
					else if((data > 3000)&&(data < 4000)){
					number_display(3840, color);
					}
					else if((data > 2000)&&(data < 3000)){
					number_display(1792, color);
					}
					else if((data > 1000)&&(data < 2000)){
					number_display(768, color);
					}
					else if((data > 0)&&(data < 1000)){
					number_display(256, color);
					}
	}
	else{
				/*the negatives */
				if((data < -7000)){
					number_display(255, color);
				}
					else if((data < -6000)&&(data > -7000)){
					number_display(254, color);
					}
					else if((data < -5000)&&(data > -6000)){
					number_display(252, color);
					}
					else if((data < -4000)&&(data > -5000)){
					number_display(248, color);
					}
					else if((data < -3000)&&(data > -4000)){
					number_display(240, color);
					}
					else if((data < -2000)&&(data > -3000)){
					number_display(224, color);
					}
					else if((data < -1000)&&(data > -2000)){
					number_display(192, color);
					}
					else if((data < 0)&&(data > -1000)){
					number_display(128, color);
					}
			}
}

/* displays custom daughter-board LED output based off of farenheit temperature ranges, after converting celcius 16 bit value ex.(2643) data to farenheit
 *Parameters: Raw temperature data
 *Returns: Void */
void LEDTemperatureDisplay(int32_t tempest){

	if (tempest >= 75){
		if ((tempest >= 75) && (tempest < 78)){
			number_display(0x001F, RED);
			number_display(0x00E0, BLUE);
		}
		else if ((tempest >= 78) && (tempest < 81)){
			number_display(0x003F, RED); //issue with 0x005F being displayed instead of 0x003F for the RED, resulting in wrong pattern
			number_display(0x00C0, BLUE);
		}
		else if ((tempest >= 81) && (tempest < 84)){
			number_display(0x007F, RED);
			number_display(0x0080, BLUE);
		}
		else if ((tempest >= 84) && (tempest < 90)){ // weird case: Occasionally would report unusually high number, so added bound of 90
			number_display(0x00FF, RED);
		}
	}
	else if(tempest < 75){
		if ((tempest >= 72) && (tempest < 75)){
			number_display(0x000F, RED);
			number_display(0x00F0, BLUE);
		}
		else if ((tempest >= 69) && (tempest < 72)){
			number_display(0x0007, RED); //c is 12
			number_display(0x00F8, BLUE);
		}
		else if ((tempest >= 66) && (tempest < 69)){
			number_display(0x0003, RED);
			number_display(0x00FC, BLUE);
		}
		else if ((tempest >= 63) && (tempest < 66)){
			number_display(0x0001, RED);
			number_display(0x00FE, BLUE);
		}
		else if ((tempest >= 60) && (tempest < 63)){
			number_display(0x00FF, BLUE);
		}
	}

}

void LEDGyroXCoordDisplay(int32_t xCoord){
	if (xCoord > 6000){
		number_display(0xF000, GREEN);
	}
	else if((xCoord < 6000) && (xCoord > 4000) ){
		number_display(0x7000, GREEN);
	}
	else if ((xCoord < 4000) && (xCoord > 2000)){
		number_display(0x3000, GREEN);
	}
	else if ((xCoord < 2000) && (xCoord > 500)){
		number_display(0x1000, GREEN);
	}
	else if ((xCoord < 500) && (xCoord > -500)){
		number_display(0x0000, GREEN);
	}
	else if ((xCoord < -500) && (xCoord > -2000)){
		number_display(0x0800, GREEN);
	}
	else if ((xCoord < -2000) && (xCoord > -4000)){
		number_display(0x0C00, GREEN);
	}
	else if ((xCoord < -4000) && (xCoord > -6000)){
		number_display(0x0E00, GREEN);
	}
	else if ((xCoord < -6000) && (xCoord > -8000)){
		number_display(0x0F00, GREEN);
	}
}
