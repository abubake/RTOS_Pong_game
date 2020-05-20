#ifndef BOARDSUPPORTPACKAGE_DRIVERLIB_RGBLEDS_H_
#define BOARDSUPPORTPACKAGE_DRIVERLIB_RGBLEDS_H_

#include <hyperloop-iot/BoardSupportPackage/DriverLib/RGB_I2C_Driver.h>

/*
 * Initialize control of board RGB LEDs
 */
void init_RGB_LEDS();

/*
 * LP3943 ColorSet
 * Sets frequencies and PWM duty cycle for each
 * register of the specified unit.
 */
void LP3943_ColorSet(uint8_t unit, uint32_t pwm_data);

/*
 * LP3943 LedModeSet
 * This function will set each of the LEDs
 * to the desired operating mode.
 * ON OFF PWM1 PWM2
 */
void LP3943_LedModeSet(uint8_t unit, uint32_t mode_data);

void LP3943_DataDisplay(uint8_t unit, uint8_t disp_mode, uint16_t data);
#endif /* BOARDSUPPORTPACKAGE_DRIVERLIB_RGBLEDS_H_ */
