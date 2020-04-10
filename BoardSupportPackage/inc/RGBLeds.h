/*
 * RGBLeds.h
 *
 *  Created on: Jan 20, 2020
 *      Author: baker
 */

#ifndef BOARDSUPPORTPACKAGE_INC_RGBLEDS_H_
#define BOARDSUPPORTPACKAGE_INC_RGBLEDS_H_

/* enumeration for RGB LEDs */
typedef enum device {
    BLUE = 0,
    GREEN = 1,
    RED = 2
}unit_desig;


/* LP3943 ColorSet
 * This function will set the frequencies and PWM duty cycle
 * for each register of the specified unit
 *  */
static void LP3943_ColorSet(uint32_t unit, uint32_t PWM_DATA);


/* LP3943_LedModeSet
 * This function will set each of the LEDs to the desired operating mode.
 * The operating modes are on, off, PWM_1 and PWM_2.
 *  */
void LP3943_LedModeSet(uint32_t unit, uint16_t LED_DATA, uint8_t LS_addy);


/*
 * Performs needed initializations for RGB LEDs
 */
void init_RGBLEDS();

void REDLED_OFF();

void GREENLED_OFF();

void BLUELED_OFF();

/*Purpose: Displays values of multiple sensors from the TI Sensors booster pack for msp432 to the daughter board LEDs
 * to give a physical representation of their values
 * Parameters: Takes in accelerometer x direction data, light sensor data, and the z axis of the gyroscope's data
 * Returns: Void. Everything is handled inside the function.
 */

inline void displayLight(uint16_t lightData);

inline void displayAcceleration(int16_t acceleration);

inline void displayGyro(int16_t gyro);

/* displays custom daughter-board LED output based off of farenheit temperature ranges, after converting celcius 16 bit value ex.(2643) data to farenheit
 *Parameters: Raw temperature data
 *Returns: Void */
void LEDTemperatureDisplay(int32_t tempest);

/*  */
void LEDGyroXCoordDisplay(int32_t xCoord);


#endif /* BOARDSUPPORTPACKAGE_INC_RGBLEDS_H_ */
