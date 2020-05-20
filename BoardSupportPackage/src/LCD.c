/*
 * LCDLib.c
 *
 *  Created on: Mar 2, 2017
 *      Author: Danny
 */

#include <hyperloop-iot/BoardSupportPackage/DriverLib/driverlib.h>
#include <hyperloop-iot/BoardSupportPackage/inc/AsciiLib.h>
#include <hyperloop-iot/BoardSupportPackage/inc/LCD.h>
#include "msp.h"

/************************************  Private Functions  *******************************************/

/*
 * Delay x ms
 */
static void Delay(unsigned long interval)
{
    while(interval > 0)
    {
        __delay_cycles(48000);
        interval--;
    }
}

/*******************************************************************************
 * Function Name  : LCD_initSPI
 * Description    : Configures LCD Control lines
 * Input          : None
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
void LCD_Init(bool usingTP)
{
    LCD_initSPI();

    if (usingTP)
    {
        /* Configure low true interrupt on P4.0 for TP */
        //config as simple I/O
        P4->SEL1 &= ~BIT0;
        P4->SEL0 & ~BIT0;

        //Set pin(s) direction to input
        P4->DIR &= ~BIT0;

        //enable a pull-up resistor
        //P4->REN |= BIT0;

        //active low
        P4->OUT |= BIT0;
        P4->IES |= BIT0;

        P4->IFG &= ~BIT0;      //clr any pending flags
        P4->IE |= BIT0;  //enable the interrupts on these pins

        NVIC_SetPriority(PORT4_IRQn, 7);
        NVIC_EnableIRQ(PORT4_IRQn);
    }

    LCD_reset();

    LCD_WriteReg(0xE5, 0x78F0); /* set SRAM internal timing */
    LCD_WriteReg(DRIVER_OUTPUT_CONTROL, 0x0100); /* set Driver Output Control */
    LCD_WriteReg(DRIVING_WAVE_CONTROL, 0x0700); /* set 1 line inversion */
    LCD_WriteReg(ENTRY_MODE, 0x1038); /* set GRAM write direction and BGR=1 */  //1038
    LCD_WriteReg(RESIZING_CONTROL, 0x0000); /* Resize register */
    LCD_WriteReg(DISPLAY_CONTROL_2, 0x0207); /* set the back porch and front porch */
    LCD_WriteReg(DISPLAY_CONTROL_3, 0x0000); /* set non-display area refresh cycle ISC[3:0] */
    LCD_WriteReg(DISPLAY_CONTROL_4, 0x0000); /* FMARK function */
    LCD_WriteReg(RGB_DISPLAY_INTERFACE_CONTROL_1, 0x0000); /* RGB interface setting */
    LCD_WriteReg(FRAME_MARKER_POSITION, 0x0000); /* Frame marker Position */
    LCD_WriteReg(RGB_DISPLAY_INTERFACE_CONTROL_2, 0x0000); /* RGB interface polarity */

    /* Power On sequence */
    LCD_WriteReg(POWER_CONTROL_1, 0x0000); /* SAP, BT[3:0], AP, DSTB, SLP, STB */
    LCD_WriteReg(POWER_CONTROL_2, 0x0007); /* DC1[2:0], DC0[2:0], VC[2:0] */
    LCD_WriteReg(POWER_CONTROL_3, 0x0000); /* VREG1OUT voltage */
    LCD_WriteReg(POWER_CONTROL_4, 0x0000); /* VDV[4:0] for VCOM amplitude */
    LCD_WriteReg(DISPLAY_CONTROL_1, 0x0001);
    Delay(200);

    /* Dis-charge capacitor power voltage */
    LCD_WriteReg(POWER_CONTROL_1, 0x1090); /* SAP, BT[3:0], AP, DSTB, SLP, STB */
    LCD_WriteReg(POWER_CONTROL_2, 0x0227); /* Set DC1[2:0], DC0[2:0], VC[2:0] */
    Delay(50); /* Delay 50ms */
    LCD_WriteReg(POWER_CONTROL_3, 0x001F);
    Delay(50); /* Delay 50ms */
    LCD_WriteReg(POWER_CONTROL_4, 0x1500); /* VDV[4:0] for VCOM amplitude */
    LCD_WriteReg(POWER_CONTROL_7, 0x0027); /* 04 VCM[5:0] for VCOMH */
    LCD_WriteReg(FRAME_RATE_AND_COLOR_CONTROL, 0x000D); /* Set Frame Rate */
    Delay(50); /* Delay 50ms */
    LCD_WriteReg(GRAM_HORIZONTAL_ADDRESS_SET, 0x0000); /* GRAM horizontal Address */
    LCD_WriteReg(GRAM_VERTICAL_ADDRESS_SET, 0x0000); /* GRAM Vertical Address */

    /* Adjust the Gamma Curve */
    LCD_WriteReg(GAMMA_CONTROL_1,    0x0000);
    LCD_WriteReg(GAMMA_CONTROL_2,    0x0707);
    LCD_WriteReg(GAMMA_CONTROL_3,    0x0307);
    LCD_WriteReg(GAMMA_CONTROL_4,    0x0200);
    LCD_WriteReg(GAMMA_CONTROL_5,    0x0008);
    LCD_WriteReg(GAMMA_CONTROL_6,    0x0004);
    LCD_WriteReg(GAMMA_CONTROL_7,    0x0000);
    LCD_WriteReg(GAMMA_CONTROL_8,    0x0707);
    LCD_WriteReg(GAMMA_CONTROL_9,    0x0002);
    LCD_WriteReg(GAMMA_CONTROL_10,   0x1D04);

    /* Set GRAM area */
    LCD_WriteReg(HOR_ADDR_START_POS, 0x0000);     /* Horizontal GRAM Start Address */
    LCD_WriteReg(HOR_ADDR_END_POS, (MAX_SCREEN_Y - 1));  /* Horizontal GRAM End Address */
    LCD_WriteReg(VERT_ADDR_START_POS, 0x0000);    /* Vertical GRAM Start Address */
    LCD_WriteReg(VERT_ADDR_END_POS, (MAX_SCREEN_X - 1)); /* Vertical GRAM Start Address */
    LCD_WriteReg(GATE_SCAN_CONTROL_0X60, 0x2700); /* Gate Scan Line */
    LCD_WriteReg(GATE_SCAN_CONTROL_0X61, 0x0001); /* NDL,VLE, REV */
    LCD_WriteReg(GATE_SCAN_CONTROL_0X6A, 0x0000); /* set scrolling line */

    /* Partial Display Control */
    LCD_WriteReg(PART_IMAGE_1_DISPLAY_POS, 0x0000);
    LCD_WriteReg(PART_IMG_1_START_END_ADDR_0x81, 0x0000);
    LCD_WriteReg(PART_IMG_1_START_END_ADDR_0x82, 0x0000);
    LCD_WriteReg(PART_IMAGE_2_DISPLAY_POS, 0x0000);
    LCD_WriteReg(PART_IMG_2_START_END_ADDR_0x84, 0x0000);
    LCD_WriteReg(PART_IMG_2_START_END_ADDR_0x85, 0x0000);

    /* Panel Control */
    LCD_WriteReg(PANEL_ITERFACE_CONTROL_1, 0x0010);
    LCD_WriteReg(PANEL_ITERFACE_CONTROL_2, 0x0600);
    LCD_WriteReg(DISPLAY_CONTROL_1, 0x0133); /* 262K color and display ON */
    Delay(50); /* delay 50 ms */

    LCD_Clear(LCD_BACKGROUNDCOLOR);
}

void LCD_initSPI()
{
    /* P10.1 - CLK
     * P10.2 - MOSI (to SDI of LCD)
     * P10.3 - MISO (to SDO of LCD)
     * P10.4 - LCD CS 
     * P10.5 - TP CS 
     */
    P10->DIR |= 0x30;
    P10->OUT |= 0x30;
    P10->SEL0 |= 0x0E;

    eUSCI_SPI_MasterConfig master;
    master.clockPhase = EUSCI_SPI_PHASE_DATA_CHANGED_ONFIRST_CAPTURED_ON_NEXT;
    master.clockPolarity = EUSCI_SPI_CLOCKPOLARITY_INACTIVITY_HIGH;
    master.clockSourceFrequency = 48000000;
    master.desiredSpiClock = 12000000;
    master.spiMode = EUSCI_A_SPI_3PIN;
    master.msbFirst = EUSCI_SPI_MSB_FIRST;
    master.selectClockSource = EUSCI_SPI_CLOCKSOURCE_SMCLK;

    SPI_initMaster(EUSCI_B3_BASE, (&master));
    EUSCI_B_SPI_enable(EUSCI_B3_BASE);
}

/*******************************************************************************
 * Function Name  : LCD_reset
 * Description    : Resets LCD
 * Input          : None
 * Output         : None
 * Return         : None
 * Attention      : Uses P10.0 for reset
 *******************************************************************************/
void LCD_reset()
{
    P10DIR |= BIT0;
    P10OUT |= BIT0;  // high
    Delay(100);
    P10OUT &= ~BIT0; // low
    Delay(100);
    P10OUT |= BIT0;  // high
}

/************************************  Private Functions  *******************************************/


/************************************  Public Functions  *******************************************/

/*******************************************************************************
 * Function Name  : LCD_DrawRectangle
 * Description    : Draw a rectangle as the specified color
 * Input          : xStart, xEnd, yStart, yEnd, Color
 * Output         : None
 * Return         : None
 * Attention      : Must draw from left to right, top to bottom!
 *******************************************************************************/
void LCD_DrawRectangle(int16_t xStart, int16_t xEnd, int16_t yStart, int16_t yEnd, uint16_t Color)
{
    // Optimization complexity: O(64 + 2N) Bytes Written 
    //assumes the start is always before the end

    //can have negative an postiive numbers

    /* Check special cases for out of bounds */
    //X wrap around
    //Y wrap around
    //XY wrap around
    //bigger than the screen <= will add this in ltr

    bool wrap = false;
    int16_t new_xStart = xStart;
    int16_t new_xEnd = xEnd;
    int16_t new_yStart = yStart;
    int16_t new_yEnd = yEnd;
    if(xStart == xEnd || yStart == yEnd){

    }
    else if(xStart > xEnd || yStart > yEnd){

    }
    else{
        //if a rectangle is bigger than the scren, the bound it to the screen for the correct coordinates
        if(xEnd - xStart + 1 > MAX_SCREEN_X){
            xStart = 0;
            xEnd = MAX_SCREEN_X - 1;
        }
        if(yEnd - yStart + 1 > MAX_SCREEN_Y){
             yStart = 0;
             xEnd = MAX_SCREEN_Y - 1;
         }

        //handling wrap arounds for X
            //partially on screen
        while(new_xStart < MIN_SCREEN_X && (new_xEnd >= MIN_SCREEN_X && new_xEnd < MIN_SCREEN_X)){   //half on the left side of the screen (Ik xEnd is going to be less than the Max screen size implicity here)
            new_xStart += MAX_SCREEN_X;

            new_xEnd = MAX_SCREEN_X - 1;
            xStart = 0;
            wrap = true;
        }
        while(new_xEnd > MAX_SCREEN_X && (new_xStart >= MIN_SCREEN_X && new_xStart < MAX_SCREEN_X)){   //half on the right side of the screen (Ik xStart is going to be less than the Max screen size implicity here)
            new_xEnd -= MAX_SCREEN_X;

            new_xStart = 0;
            xEnd = MAX_SCREEN_X - 1;
            wrap = true;
        }
        //make sure this before the next two while loops
            //completely off screen
        while(xStart < MIN_SCREEN_X && xEnd < MIN_SCREEN_X){   //completely off the left side of the screen (Ik xEnd is going to be less than the Max screen size implicity here)
            xStart += MAX_SCREEN_X;
            xEnd += MAX_SCREEN_X;
        }
        while(xStart >= MAX_SCREEN_X && xEnd >= MAX_SCREEN_X){   //completely off the right side of the screen (Ik xEnd is going to be less than the Max screen size implicity here)
            xStart -= MAX_SCREEN_X;
            xEnd -= MAX_SCREEN_X;
        }

        //handling wrap arounds for Y
            //partially on screen
        while(new_yStart < MIN_SCREEN_Y && (new_yEnd >= MIN_SCREEN_Y && new_yEnd < MIN_SCREEN_Y)){   //half on the left side of the screen (Ik yEnd is going to be less than the Max screen size implicity here)
            new_yStart += MAX_SCREEN_Y;

            new_yEnd = MAX_SCREEN_Y - 1;
            yStart = 0;
            wrap = true;
        }
        while(new_yEnd > MAX_SCREEN_Y && (new_yStart >= MIN_SCREEN_Y && new_yStart < MAX_SCREEN_Y)){   //half on the right side of the screen (Ik xStart is going to be less than the Max screen size implicity here)
            new_yEnd -= MAX_SCREEN_Y;

            new_yStart = 0;
            yEnd = MAX_SCREEN_Y - 1;
            wrap = true;
        }
        //make sure this before the next two while loops
            //completely off screen
        while(yStart < MIN_SCREEN_Y && yEnd < MIN_SCREEN_Y){   //completely off the left side of the screen (Ik xEnd is going to be less than the Max screen size implicity here)
            yStart += MAX_SCREEN_Y;
            yEnd += MAX_SCREEN_Y;
        }
        while(yStart >= MAX_SCREEN_Y && yEnd >= MAX_SCREEN_Y){   //completely off the right side of the screen (Ik xEnd is going to be less than the Max screen size implicity here)
            yStart -= MAX_SCREEN_Y;
            yEnd -= MAX_SCREEN_Y;
        }

        //Assuming all points
        //making sure start is before end X


        /* Set window area for high-speed RAM write */
        LCD_WriteReg(HOR_ADDR_START_POS, yStart);
        LCD_WriteReg(HOR_ADDR_END_POS, yEnd);

        LCD_WriteReg(VERT_ADDR_START_POS, xStart);
        LCD_WriteReg(VERT_ADDR_END_POS, xEnd);
        /* Set cursor */
        LCD_SetCursor(yStart, xStart);
        /* Set index to GRAM */
        LCD_WriteIndex(GRAM);
        /* Send out data only to the entire area */

        SPI_CS_LOW;
        LCD_Write_Data_Start();
       for(uint32_t i = 0; i < (xEnd - xStart + 1)*(yEnd - yStart + 1); i++){
           LCD_Write_Data_Only(Color);
       }
       SPI_CS_HIGH;

//       /* Set window area for high-speed RAM write */
//       LCD_WriteReg(HOR_ADDR_START_POS, new_yStart);
//       LCD_WriteReg(HOR_ADDR_END_POS, new_yEnd);
//
//       LCD_WriteReg(VERT_ADDR_START_POS, new_xStart);
//       LCD_WriteReg(VERT_ADDR_END_POS, new_xEnd);
//       /* Set cursor */
//       LCD_SetCursor(new_yStart, new_xStart);
//       /* Set index to GRAM */
//       LCD_WriteIndex(GRAM);
//       /* Send out data only to the entire area */
//
//       Delay(2);
//       SPI_CS_LOW;
//       LCD_Write_Data_Start();
//      for(uint32_t i = 0; i < (new_xEnd - new_xStart + 1)*(new_yEnd - new_yStart + 1); i++){
//          LCD_Write_Data_Only(Color);
//      }
//      SPI_CS_HIGH;

       if(wrap){
           LCD_DrawRectangle(new_xStart, new_xEnd, new_yStart, new_yEnd, Color);
       }
    }
}

/******************************************************************************
 * Function Name  : PutChar
 * Description    : Lcd screen displays a character
 * Input          : - Xpos: Horizontal coordinate
 *                  - Ypos: Vertical coordinate
 *                  - ASCI: Displayed character
 *                  - charColor: Character color
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
inline void PutChar( uint16_t Xpos, uint16_t Ypos, uint8_t ASCI, uint16_t charColor)
{
    uint16_t i, j;
    uint8_t buffer[16], tmp_char;
    GetASCIICode(buffer,ASCI);  /* get font data */
    for( i=0; i<16; i++ )
    {
        tmp_char = buffer[i];
        for( j=0; j<8; j++ )
        {
            if( (tmp_char >> 7 - j) & 0x01 == 0x01 )
            {
                LCD_SetPoint( Xpos + j, Ypos + i, charColor );  /* Character color */
            }
        }
    }
}

/******************************************************************************
 * Function Name  : GUI_Text
 * Description    : Displays the string
 * Input          : - Xpos: Horizontal coordinate
 *                  - Ypos: Vertical coordinate
 *                  - str: Displayed string
 *                  - charColor: Character color
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
void LCD_Text(uint16_t Xpos, uint16_t Ypos, uint8_t *str, uint16_t Color)
{
    uint8_t TempChar;

    /* Set area back to span the entire LCD */
    /* Set area back to span the entire LCD */
    LCD_WriteReg(HOR_ADDR_START_POS, 0x0000);     /* Horizontal GRAM Start Address */
    LCD_WriteReg(HOR_ADDR_END_POS, (MAX_SCREEN_Y - 1));  /* Horizontal GRAM End Address */
    LCD_WriteReg(VERT_ADDR_START_POS, 0x0000);    /* Vertical GRAM Start Address */
    LCD_WriteReg(VERT_ADDR_END_POS, (MAX_SCREEN_X - 1)); /* Vertical GRAM Start Address */

    do
    {
        TempChar = *str++;
        PutChar( Xpos, Ypos, TempChar, Color);
        if( Xpos < MAX_SCREEN_X - 8)
        {
            Xpos += 8;
        }
        else if ( Ypos < MAX_SCREEN_X - 16)
        {
            Xpos = 0;
            Ypos += 16;
        }
        else
        {
            Xpos = 0;
            Ypos = 0;
        }
    }
    while ( *str != 0 );
}


/*******************************************************************************
 * Function Name  : LCD_Clear
 * Description    : Fill the screen as the specified color
 * Input          : - Color: Screen Color
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
void LCD_Clear(uint16_t Color)
{
    /* Set area back to span the entire LCD */
    LCD_WriteReg(HOR_ADDR_START_POS, MIN_SCREEN_Y);
    LCD_WriteReg(HOR_ADDR_END_POS, MAX_SCREEN_Y-1);
    LCD_WriteReg(VERT_ADDR_START_POS, MIN_SCREEN_X);
    LCD_WriteReg(VERT_ADDR_END_POS, MAX_SCREEN_X-1);

    /* Set cursor to (0,0) */
    LCD_SetCursor(0,0);

    /* Set write index to GRAM */
    LCD_WriteIndex(GRAM);

    /* Start data transmission */
    SPI_CS_LOW;
    LCD_Write_Data_Start();
    for(int i = 0; i < MAX_SCREEN_X*MAX_SCREEN_Y; i++){
        LCD_Write_Data_Only(Color);
    }
    SPI_CS_HIGH;
    // You'll need to call LCD_Write_Data_Start() and then send out only data to fill entire screen with color 
}

/******************************************************************************
 * Function Name  : LCD_SetPoint
 * Description    : Drawn at a specified point coordinates
 * Input          : - Xpos: Row Coordinate
 *                  - Ypos: Line Coordinate
 * Output         : None
 * Return         : None
 * Attention      : 18N Bytes Written
 *******************************************************************************/
void LCD_SetPoint(uint16_t Xpos, uint16_t Ypos, uint16_t color)
{
    /* Should check for out of bounds */ 

    /* Set cursor to Xpos and Ypos */ 
    LCD_SetCursor(Ypos, Xpos);
    /* Write color to GRAM reg */
    LCD_WriteReg(GRAM, color);
}

/*******************************************************************************
 * Function Name  : LCD_Write_Data_Only
 * Description    : Data writing to the LCD controller
 * Input          : - data: data to be written
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
inline void LCD_Write_Data_Only(uint16_t data)
{

    /* Send out MSB */ 
    SPISendRecvByte((data >>   8));                    /* Write D8..D15                */
    /* Send out LSB */ 
    SPISendRecvByte((data & 0xFF));                    /* Write D0..D7                 */

}

/*******************************************************************************
 * Function Name  : LCD_WriteData
 * Description    : LCD write register data
 * Input          : - data: register data
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
inline void LCD_WriteData(uint16_t data)
{
    SPI_CS_LOW;

    SPISendRecvByte(SPI_START | SPI_WR | SPI_DATA);    /* Write : RS = 1, RW = 0       */
    SPISendRecvByte((data >>   8));                    /* Write D8..D15                */
    SPISendRecvByte((data & 0xFF));                    /* Write D0..D7                 */

    SPI_CS_HIGH;
}

/*******************************************************************************
 * Function Name  : LCD_WriteReg
 * Description    : Reads the selected LCD Register.
 * Input          : None
 * Output         : None
 * Return         : LCD Register Value.
 * Attention      : None
 *******************************************************************************/
inline uint16_t LCD_ReadReg(uint16_t LCD_Reg)
{
    /* Write 16-bit Index */
    LCD_WriteIndex(LCD_Reg);
    /* Return 16-bit Reg using LCD_ReadData() */
    return LCD_ReadData();
}

/*******************************************************************************
 * Function Name  : LCD_WriteIndex
 * Description    : LCD write register address
 * Input          : - index: register address
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
inline void LCD_WriteIndex(uint16_t index)
{
    SPI_CS_LOW;

    /* SPI write data */
    SPISendRecvByte(SPI_START | SPI_WR | SPI_INDEX);   /* Write : RS = 0, RW = 0  */
    SPISendRecvByte(0);
    SPISendRecvByte(index);

    SPI_CS_HIGH;
}

/*******************************************************************************
 * Function Name  : SPISendRecvByte
 * Description    : Send one byte then receive one byte of response
 * Input          : uint8_t: byte
 * Output         : None
 * Return         : Recieved value 
 * Attention      : None
 *******************************************************************************/
inline uint8_t SPISendRecvByte (uint8_t byte)
{
    /* Send byte of data */
    UCB3TXBUF_SPI = byte;
    while(!(UCB3IFG & UCTXIFG));    //wait for transmit to finish
    /* Wait as long as busy */ 
    while(!(UCB3IFG & UCRXIFG));    //wait to recieve information
    //while(!(UCB3STATW & UCBUSY));    //wait to recieve information
    /* Return received value*/
    return (UCB3RXBUF_SPI & UCRXBUF_M);
}

/*******************************************************************************
 * Function Name  : LCD_Write_Data_Start
 * Description    : Start of data writing to the LCD controller
 * Input          : None
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
inline void LCD_Write_Data_Start(void)
{
    SPISendRecvByte(SPI_START | SPI_WR | SPI_DATA);    /* Write : RS = 1, RW = 0 */
}

/*******************************************************************************
 * Function Name  : LCD_ReadData
 * Description    : LCD read data
 * Input          : None
 * Output         : None
 * Return         : return data
 * Attention      : Diagram (d) in datasheet
 *******************************************************************************/
inline uint16_t LCD_ReadData()
{
    uint16_t value;
    SPI_CS_LOW;

    SPISendRecvByte(SPI_START | SPI_RD | SPI_DATA);   /* Read: RS = 1, RW = 1   */
    SPISendRecvByte(0);                               /* Dummy read 1           */
    value = (SPISendRecvByte(0) << 8);                /* Read D8..D15           */
    value |= SPISendRecvByte(0);                      /* Read D0..D7            */

    SPI_CS_HIGH;
    return value;
}

/*******************************************************************************
 * Function Name  : LCD_ReadDataPixel
 * Description    : LCD read data variant for reading pixel data
 * Input          : None
 * Output         : None
 * Return         : return data
 * Attention      : Diagram (d) in datasheet
 *******************************************************************************/
inline uint16_t LCD_ReadDataPixel()
{
    uint16_t value;
    SPI_CS_LOW;

    SPISendRecvByte(SPI_START | SPI_RD | SPI_DATA);   /* Read: RS = 1, RW = 1   */
    SPISendRecvByte(0);                               /* Dummy read 1           */
    SPISendRecvByte(0);                               /* Dummy read 2           */
    SPISendRecvByte(0);                               /* Dummy read 3           */
    SPISendRecvByte(0);                               /* Dummy read 4           */
    SPISendRecvByte(0);                               /* Dummy read 5           */

    value = (SPISendRecvByte(0) << 8);                /* Read D8..D15           */
    value |= SPISendRecvByte(0);                      /* Read D0..D7            */

    SPI_CS_HIGH;
    return value;
}

/*******************************************************************************
 * Function Name  : LCD_ReadPixelColor
 * Description    : Reads color from designated pixel
 * Input          : a and y coordiante
 * Output         : None
 * Return         : Pixel color
 * Attention      : None
 *******************************************************************************/
uint16_t LCD_ReadPixelColor(uint16_t x, uint16_t y)
{
    LCD_SetCursor(y, x);
    LCD_WriteIndex(GRAM);
    return LCD_ReadDataPixel();
}

/*******************************************************************************
 * Function Name  : LCD_WriteReg
 * Description    : Writes to the selected LCD register.
 * Input          : - LCD_Reg: address of the selected register.
 *                  - LCD_RegValue: value to write to the selected register.
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
inline void LCD_WriteReg(uint16_t LCD_Reg, uint16_t LCD_RegValue)
{
    /* Write 16-bit Index */
    LCD_WriteIndex(LCD_Reg);
    /* Write 16-bit Reg Data */
    LCD_WriteData(LCD_RegValue);
}

/*******************************************************************************
 * Function Name  : LCD_SetCursor
 * Description    : Sets the cursor position.
 * Input          : - Xpos: specifies the X position.
 *                  - Ypos: specifies the Y position.
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
inline void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos )
{
    /* Should just be two LCD_WriteReg to appropriate registers */ 

    /* Set horizonal GRAM coordinate (Ypos) */ 
    LCD_WriteReg(GRAM_HORIZONTAL_ADDRESS_SET, Xpos);
    /* Set vertical GRAM coordinate (Xpos) */
    LCD_WriteReg(GRAM_VERTICAL_ADDRESS_SET, Ypos);
}

/*******************************************************************************
 * Function Name  : LCD_Init
 * Description    : Configures LCD Control lines, sets whole screen black
 * Input          : bool usingTP: determines whether or not to enable TP interrupt 
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
/*******************************************************************************
 * Function Name  : TP_ReadXY
 * Description    : Obtain X and Y touch coordinates
 * Input          : None
 * Output         : None
 * Return         : Pointer to "Point" structure
 * Attention      : None
 *******************************************************************************/
Point TP_ReadXY()
{
    Point press;    //will have to be allocated in memeory in heap
    EUSCI_B_SPI_masterChangeClock(EUSCI_B3_BASE, 48000000, 3000000);;

    press.x = TP_ReadX();
    press.y = TP_ReadY();

    EUSCI_B_SPI_masterChangeClock(EUSCI_B3_BASE, 48000000, 12000000);;

    /* Return point  */ 
    return press;
}

/*******************************************************************************
 * Function Name  : TP_ReadX
 * Description    : Obtain X touch coordinates
 * Input          : None
 * Output         : None
 * Return         : int16_t
 * Attention      : None
 *******************************************************************************/
int32_t TP_ReadX(){
    int32_t coor;
    int16_t val1;
    int16_t val2;

    SPI_CS_TP_LOW;
    /* Read X coord. */
    SPISendRecvByte(CHX);
    val1 = SPISendRecvByte(0); //XXXX XXXX | X--- ---- | XXXX XXXX
    val2 = SPISendRecvByte(0);
    SPI_CS_TP_HIGH;

    val1 = val1 << 5;
    val2 = val2 >> 3;
    coor = ((val1 | val2) * MAX_SCREEN_X) / 4095;

    //now scale for touch pad to LCD pixels
    //coor = (coor * MAX_SCREEN_X) / MAX_TOUCH_X;
    return coor;
}

/*******************************************************************************
 * Function Name  : TP_ReadY
 * Description    : Obtain Y touch coordinates
 * Input          : None
 * Output         : None
 * Return         : int16_t
 * Attention      : None
 *******************************************************************************/
int32_t TP_ReadY(){
    int32_t coor;
    int16_t val1;
    int16_t val2;

    SPI_CS_TP_LOW;
   /* Read Y coord. */
   SPISendRecvByte(CHY);
   val1 = SPISendRecvByte(0); //XXXX XXXX | X--- ---- | XXXX XXXX
   val2 = SPISendRecvByte(0);
   SPI_CS_TP_HIGH;

   val1 = val1 << 5;
   val2 = val2 >> 3;
   coor = ((val1 | val2) * MAX_SCREEN_Y) / 4095;

   //coor = (coor * MAX_SCREEN_Y) / MAX_TOUCH_Y;
   return coor;

}

/************************************  Public Functions  *******************************************/



