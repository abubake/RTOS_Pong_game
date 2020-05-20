/*
 * RGB_I2C_Driver.c
 *
 *  Created on: Jan 21, 2020
 *      Author: steve
 */
#include <hyperloop-iot/BoardSupportPackage/DriverLib/RGB_I2C_Driver.h>

//send data via
void LED_write(uint8_t reg, uint8_t data){
    __disable_irq();
    //start and wait for slave addr to be completely sent
    UCB2CTLW0 |= UCTXSTT;
    while(UCB2CTLW0 & UCTXSTT);

    //what register am I writing to
    UCB2TXBUF = reg;

    //wait for it to be completely be shifted out
    while(!(UCB2IFG & UCTXIFG0));

    //what data am i writing out
    UCB2TXBUF = data;

    ////wait for it to be completely be shifted out
    while(!(UCB2IFG & UCTXIFG0));

    //stop
    UCB2CTLW0 |= UCTXSTP;

    //wait for stop to be finalized
    while(UCB2CTLW0 & UCTXSTP);
    __enable_irq();
}

void setSlave(uint8_t slave){
    UCB2I2CSA = slave;
}
