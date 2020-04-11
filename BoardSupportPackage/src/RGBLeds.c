#include <RGBLeds.h>

void init_RGB_LEDS(){


    //Software reset enable

    UCB2CTLW0 = UCSWRST;

    //Initialize I2C master
    // I2C mode, Clock sync, SMCLK source, transmitter
    //meant for a non-multi master system

    UCB2CTLW0 |= (UCMST | UCMODE_3 | UCSYNC | UCSSEL_3 | UCTR);

    //Max clk frequency for the LP3943 is 400kHz
    //Assumes SMCLK source is DCOCLK wihch is default 3 MHz
    // 3Mz/8 ->375 kHz (pretty close to max without having to tune DCOCLK frequency)
    UCB2BRW = 30;

    //Sets the pins as I2C mode

    P3SEL0 |= (BIT7 | BIT6);
    P3SEL1 &= 0;

    //Bitwise anding of all bits except UCSWRT
    //no more reset
    UCB2CTLW0 &= ~UCSWRST;

    //Turn them all off
    LP3943_LedModeSet(BLUE, OFF);
    LP3943_LedModeSet(GREEN, OFF);
    LP3943_LedModeSet(RED, OFF);

}

//PWM and PSC are set
void LP3943_ColorSet(uint8_t unit, uint32_t pwm_data){
    //PWM_DATA format
    //PWM1 _ PSC1 _ PWM0 _ PSC0

    //PWM controls the brightness of the LEDs and
    //PSC controls the blinking of the LEDs

    setSlave(unit);
    uint8_t cur_reg = PSC0;
    for(;cur_reg <= PWM1; cur_reg++){
        LED_write(cur_reg, (uint8_t)pwm_data);
        pwm_data = pwm_data >> 8;
    }
}

//says which color LEDs to put in a certain mode
void LP3943_LedModeSet(uint8_t unit, uint32_t mode_data){
    //LED_DATA: the 32 bits are split into 4 8 bytes where each 2 bits represents an LED state
    //I will SWEEP through each byte, updating 4 LEDs at a time from LED 0->15
    //setSlave(unit);

    setSlave(unit);
    uint8_t cur_reg = LS0;
    for(;cur_reg <= LS3; cur_reg++){
        LED_write(cur_reg, (uint8_t)(mode_data & 0x000000FF));
        mode_data = mode_data >> 8;
    }
}

//displays 16-bit data on the 16 LEDs
void LP3943_DataDisplay(uint8_t unit, uint8_t disp_mode, uint16_t data){
    //takes in data intended to be displayed in binary on the LEDs
    //made data 32 bits

    LP3943_LedModeSet(unit, OFF);   //turn all LEDs off
    //bit manipulation to set to mode the data is intended to be displayed in

    if(disp_mode != OFF){
       uint32_t mode_set = 0;
       uint8_t i = 0;
       for(; i < 16; i++){
            mode_set = mode_set << 2;   //allows setting of each LED bit in LS0 - LS3 (2-bit select)
            if(data & 0x8000){       //probe MSB
               //if then LED is supposed to be lit then copy the disp_mode
               mode_set |= disp_mode;
            }
            data  = data << 1;      //shift data over to probe next bit
        }
        LP3943_LedModeSet(unit, mode_set);
    }
}
