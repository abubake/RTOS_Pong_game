/*
 * RGB_I2C_Driver.h
 *
 *  Created on: Jan 21, 2020
 *      Author: steve
 */

#ifndef RGB_I2C_DRIVER_H_
#define RGB_I2C_DRIVER_H_

#include <driverlib.h>

enum device
{
    //0,1,2 -> 0x60,..1,..2 so if addresses were changed for some reason, not much code needs to be changed
    BLUE = 0x60,
    RED = 0x62,
    GREEN = 0x61,
};

enum regs
{
    PSC0 = 2,
    PWM0 = 3,
    PSC1 = 4,
    PWM1 = 5,
    LS0 = 6,
    LS1 = 7,
    LS2 = 8,
    LS3 = 9
};

enum modes
{
    //0,1,2 -> 0x60,..1,..2 so if addresses were changed for some reason, not much code needs to be changed
    OFF = 0,
    ON = 1,
    DIM0 = 2,
    DIM1 = 3
};

void setSlave(uint8_t slave);
void LED_write(uint8_t reg, uint8_t data);


#endif /* RGB_I2C_DRIVER_H_ */
