/*
 * helpers.c
 *
 *  Created on: Jan 17, 2020
 *      Author: baker
 */
#include <driverlib.h>
#include <RGBLeds.h>
#include <stdio.h>
#include <stdlib.h>
#include <G8RTOS.h>
/////////////////////////////////////////////////////////////////////////////////////

/* Newton method square root. Finds an approximate square root without using floating point values (i.e. an integer square root value
 * Parameters: n, the number you would like to take the square root of
 * Returns: the value square rooted as an integer value */
uint16_t newtonSquareRoot(uint16_t n){
	uint16_t squaredVal;
	uint16_t temp0 = 0;
	uint16_t temp = (n + 1)/2;
	while(abs(temp - temp0) < 1){
	temp0 = temp; //preserves temp value, acts as x(k)
	temp = (temp + (n/temp))/2; //acts as x(k+1)
	}
	squaredVal = temp;
	return squaredVal;
}

int Modulus255(uint16_t val){
        val = (val % 255);
        return val;
    }

uint16_t Fletcher16( uint8_t *data, int count)
    {
        uint16_t sum1 = 0;
        uint16_t sum2 = 0;
        int index = 0;

        for ( index = 0; index < count; ++index )
        {
            sum1 = (sum1 + data[index]) % 255;
            sum2 = (sum1 + sum2) % 255;
        }
        return (sum2 << 8) | sum1;
    }

void delayMs(int n) {
    int i, j;

    for (j = 0; j < n; j++)
        for (i = 750; i > 0; i--);      /* Delay */
}

void number_display(uint16_t number, uint32_t unit){

    uint16_t temp = number; //preserves the value of the number
    //changed from 3 to 3 + 11
    uint16_t number_b0 = number & 0x0001;
    uint16_t number_b1 = number & 0x0002;
    uint16_t number_b2 = number & 0x0004;
    uint16_t number_b3 = number & 0x0008;

    uint16_t number_b4 = number & 0x0003;
    uint16_t number_b5 = number & 0x0005;
    uint16_t number_b6 = number & 0x0006;
    uint16_t number_b7 = number & 0x0007;
    uint16_t number_b8 = number & 0x0009;
    uint16_t number_b9 = number & 0x000A;
    uint16_t number_b10 = number & 0x000B;
    uint16_t number_b11 = number & 0x000C;
    uint16_t number_b12 = number & 0x000D;
    uint16_t number_b13 = number & 0x000E;
    uint16_t number_b14 = number & 0x000F;
    uint8_t k = 0;

    for (uint8_t j = 0; j < 4; j++){
//added 11 to for loop count (4 + 11)
    if (number_b0 != 0){
        LP3943_LedModeSet(unit, number_b0, 0x06+j);
    }
    if(number_b1 != 0){
        LP3943_LedModeSet(unit, number_b1, 0x06+j);
    }
    if(number_b2 != 0){
        LP3943_LedModeSet(unit, number_b2, 0x06+j);
    }
    if(number_b3 != 0){
        LP3943_LedModeSet(unit, number_b3, 0x06+j);
    }
    /////////////////////////////////////////////////////////////////////
    if(number_b4 != 0){
            LP3943_LedModeSet(unit, number_b4, 0x06+j);
        }
    if(number_b5 != 0){
            LP3943_LedModeSet(unit, number_b5, 0x06+j);
        }
    if(number_b6 != 0){
            LP3943_LedModeSet(unit, number_b6, 0x06+j);
        }
    if(number_b7 != 0){
            LP3943_LedModeSet(unit, number_b7, 0x06+j);
        }
    if(number_b8 != 0){
            LP3943_LedModeSet(unit, number_b8, 0x06+j);
        }
    if(number_b9 != 0){
            LP3943_LedModeSet(unit, number_b9, 0x06+j);
        }
    if(number_b10 != 0){
            LP3943_LedModeSet(unit, number_b10, 0x06+j);
        }
    if(number_b11 != 0){
            LP3943_LedModeSet(unit, number_b11, 0x06+j);
        }
    if(number_b12 != 0){
            LP3943_LedModeSet(unit, number_b12, 0x06+j);
        }
    if(number_b13 != 0){
            LP3943_LedModeSet(unit, number_b13, 0x06+j);
        }
    if(number_b14 != 0){
            LP3943_LedModeSet(unit, number_b14, 0x06+j);
        }
    else{
        LP3943_LedModeSet(unit, 0x0000, 0x06+j);
    }

    temp = (temp >> 4);

     number_b0 = temp & 0x0001;
     number_b1 = temp & 0x0002;
     number_b2 = temp & 0x0004;
     number_b3 = temp & 0x0008;

     number_b4 = temp & 0x0003;
     number_b5 = temp & 0x0005;
     number_b6 = temp & 0x0006;
     number_b7 = temp & 0x0007;
     number_b8 = temp & 0x0009;
     number_b9 = temp & 0x000A;
     number_b10 = temp & 0x000B;
     number_b11 = temp & 0x000C;
     number_b12 = temp & 0x000D;
     number_b13 = temp & 0x000E;
     number_b14 = temp & 0x000F;

     k++;
    }

}
