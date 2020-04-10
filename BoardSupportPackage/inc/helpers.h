/*
 * helpers.h
 *
 *  Created on: Jan 17, 2020
 *      Author: baker
 */
#include <RGBLeds.h>

#ifndef HELPERS_H_
#define HELPERS_H_

/* Newton method square root. Finds an approximate square root without using floating point values (i.e. an integer square root value
 * Parameters: n, the number you would like to take the square root of
 * Returns: the value square rooted as an integer value */
uint16_t newtonSquareRoot(uint16_t n);

int Modulus255(uint16_t val);

uint16_t Fletcher16( uint8_t *data, int count);

void delayMs(int n);

/*Purpose: Displays a specific number in hex to the 16 LEDs of the daughter board for a color of your choosing */
/*Parameters: number to be displayed, color RED, GREEN, or BLUE  */
/*Returns: Void  */
void number_display(uint16_t number, uint32_t unit);

void months_until(uint16_t val1);

void days_until(uint16_t val2);

void hours_until(uint16_t val3);

void minutes_until(uint16_t val4);

#endif /* HELPERS_H_ */
