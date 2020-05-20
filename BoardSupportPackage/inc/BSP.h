/*
 * BSP.h
 *
 *  Created on: Dec 30, 2016
 *      Author: Raz Aloni
 */

#ifndef BSP_H_
#define BSP_H_

/* Includes */

#include <hyperloop-iot/BoardSupportPackage/inc/bme280_support.h>
#include <hyperloop-iot/BoardSupportPackage/inc/bmi160_support.h>
#include <hyperloop-iot/BoardSupportPackage/inc/BackChannelUart.h>
#include <hyperloop-iot/BoardSupportPackage/inc/ClockSys.h>
#include <hyperloop-iot/BoardSupportPackage/inc/Joystick.h>
#include <hyperloop-iot/BoardSupportPackage/inc/opt3001.h>
#include <hyperloop-iot/BoardSupportPackage/inc/RGB_I2C_Driver.h>
#include <hyperloop-iot/BoardSupportPackage/inc/RGBLeds.h>
#include <hyperloop-iot/BoardSupportPackage/inc/tmp007.h>
#include <stdint.h>


/********************************** Public Functions **************************************/

/* Initializes the entire board */
extern void BSP_InitBoard();

/********************************** Public Functions **************************************/

#endif /* BSP_H_ */
