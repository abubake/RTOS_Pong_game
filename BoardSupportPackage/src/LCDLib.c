/*
 * LCDLib.c
 *
 *  Created on: Mar 2, 2017
 *      Author: Danny
 */

#include "LCDLib.h"
#include "msp.h"
#include "driverlib.h"
#include "AsciiLib.h"

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
static void LCD_initSPI()
{
	/* For the Chip selects */
	P10->DIR |= 0x30;
	P10->OUT |= 0x30; //CS are HIGH
	P10->SEL0 |= 0x0E;

	eUSCI_SPI_MasterConfig masterInfo;
	masterInfo.clockPhase = EUSCI_SPI_PHASE_DATA_CHANGED_ONFIRST_CAPTURED_ON_NEXT;
	masterInfo.clockPolarity = EUSCI_SPI_CLOCKPOLARITY_INACTIVITY_HIGH;
	masterInfo.clockSourceFrequency = 24000000; //48
	masterInfo.desiredSpiClock = 12000000;
	masterInfo.spiMode = EUSCI_SPI_3PIN;
	masterInfo.msbFirst = EUSCI_SPI_MSB_FIRST;
	masterInfo.selectClockSource = EUSCI_SPI_CLOCKSOURCE_SMCLK;

	SPI_initMaster(EUSCI_B3_BASE, (&masterInfo));
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
static void LCD_reset()
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
	/* IN BOUNDS */
	if (((MAX_SCREEN_X-1) > xEnd)&&((MAX_SCREEN_Y-1) > yEnd)&&(xStart >= 0)&&(yStart >= 0)){
		/* use the original data since it's only one rectangle */
				LCD_WriteReg(HOR_ADDR_START_POS, yStart);
				LCD_WriteReg(HOR_ADDR_END_POS, yEnd-1);
				LCD_WriteReg(VERT_ADDR_START_POS, xStart);
			    LCD_WriteReg(VERT_ADDR_END_POS, xEnd-1);

			    /* Set cursor to (0,0) */
			    LCD_SetCursor(xStart, yStart);
			    /* Set write index to GRAM */
			    LCD_WriteIndex(GRAM);
			    /* Start data transmit */
			    SPI_CS_LOW;

			    LCD_Write_Data_Start();
			    for(uint32_t i = 0; i < (xEnd - xStart +1)*(yEnd - yStart + 1); i++){
			    	LCD_Write_Data_Only(Color);
			    	}
			    SPI_CS_HIGH;

	} /* X Out of bounds on right */
	else if(((MAX_SCREEN_X-1) < xEnd)&&((MAX_SCREEN_Y-1) > yEnd)&&(yStart >= 0)){
		LCD_DrawRectangle(xStart, MAX_SCREEN_X-1, yStart, yEnd, Color);
		LCD_DrawRectangle(0, xEnd-MAX_SCREEN_X-1, yStart, yEnd, Color);

	} /* X Out of bounds on left */
	else if((xStart < 0)&&((MAX_SCREEN_Y-1) > yEnd)&&(yStart >= 0)){
		LCD_DrawRectangle(0, xEnd, yStart, yEnd, Color);
		LCD_DrawRectangle(MAX_SCREEN_X-1+xStart, MAX_SCREEN_X-1, yStart, yEnd, Color);

	} /* Y Out of bounds on bottom */
	else if(((MAX_SCREEN_X-1) > xEnd)&&(xStart >= 0)&&((MAX_SCREEN_Y-1) < yEnd)){
		LCD_DrawRectangle(xStart, xEnd, yStart, MAX_SCREEN_Y-1, Color);
		LCD_DrawRectangle(xStart, xEnd, 0, yEnd-MAX_SCREEN_Y-1, Color);

	} /* Y Out of bounds on top */
	else if(((MAX_SCREEN_X-1) > xEnd)&&(xStart >= 0)&&(yStart < 0)){
		LCD_DrawRectangle(xStart, xEnd, 0, yEnd, Color);
		LCD_DrawRectangle(xStart, xEnd, MAX_SCREEN_Y-1 + yStart, MAX_SCREEN_Y-1, Color);

	} /* Top right corner */
	else if((xEnd > (MAX_SCREEN_X-1))&&(yStart < 0)){
		LCD_DrawRectangle(xStart, MAX_SCREEN_X-1, 0, yEnd, Color);
		LCD_DrawRectangle(xStart, MAX_SCREEN_X-1, MAX_SCREEN_Y+yStart, MAX_SCREEN_Y-1, Color);
		LCD_DrawRectangle(0, xEnd-MAX_SCREEN_X-1, 0, yEnd, Color);
		LCD_DrawRectangle(0, xEnd-MAX_SCREEN_X-1, yStart+MAX_SCREEN_Y-1, MAX_SCREEN_Y-1, Color);

	} /* Top left corner */
	else if((xStart < 0)&&(yStart < 0)){
		LCD_DrawRectangle(0, xEnd, 0, yEnd, Color);
		LCD_DrawRectangle(MAX_SCREEN_X-1+xStart, MAX_SCREEN_X-1, 0, yEnd, Color);
		LCD_DrawRectangle(0, xEnd, MAX_SCREEN_Y-1 + yStart, MAX_SCREEN_Y-1, Color);
	    LCD_DrawRectangle(MAX_SCREEN_X-1+xStart, MAX_SCREEN_X-1, MAX_SCREEN_Y-1+yStart, MAX_SCREEN_Y-1, Color);

	} /* Bottom left corner */
	else if((xStart < 0)&&(MAX_SCREEN_Y-1 <= yEnd)){
		LCD_DrawRectangle(0, xEnd, yStart, MAX_SCREEN_Y-1, Color);
		LCD_DrawRectangle(0, xEnd, 0, yEnd-MAX_SCREEN_Y-1, Color);
		LCD_DrawRectangle(xStart + MAX_SCREEN_X-1, MAX_SCREEN_X-1, yStart, MAX_SCREEN_Y-1, Color);
		LCD_DrawRectangle(xStart + MAX_SCREEN_X-1, MAX_SCREEN_X-1, 0, yEnd-MAX_SCREEN_Y-1, Color);

	} /* Bottom right corner */
	else if((xEnd >= MAX_SCREEN_X-1)&&(yEnd > MAX_SCREEN_Y-1)){
		LCD_DrawRectangle(xStart, MAX_SCREEN_X-1, yStart, MAX_SCREEN_Y-1, Color);
		LCD_DrawRectangle(xStart, MAX_SCREEN_X-1, 0, yEnd-MAX_SCREEN_Y-1, Color);
		LCD_DrawRectangle(0, xEnd-MAX_SCREEN_X-1 , 0, yEnd-MAX_SCREEN_Y-1, Color);
		LCD_DrawRectangle(0, xEnd-MAX_SCREEN_X-1, yStart, MAX_SCREEN_Y-1, Color);
	}
}

void rectangleDisplay(uint16_t Color){
	/* use the original data since it's only one rectangle */

		//LCD_WriteReg(HOR_ADDR_START_POS, Ystart);
		//LCD_WriteReg(HOR_ADDR_END_POS, Yend-1);
		//LCD_WriteReg(VERT_ADDR_START_POS, Xstart);
	    //LCD_WriteReg(VERT_ADDR_END_POS, Xend-1);

	    /* Set cursor to (0,0) */
	    //LCD_SetCursor(Xstart, Ystart);
	    /* Set write index to GRAM */
	    LCD_WriteIndex(GRAM);
	    /* Start data transmition */
	    SPI_CS_LOW;
/*
	    LCD_Write_Data_Start();
	    for(uint32_t i = 0; i < (xDimension*yDimension); i++){
	    	LCD_Write_Data_Only(Color);
	    	}
	    SPI_CS_HIGH;
*/
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
    LCD_SetCursor(0, 0);
    /* Set write index to GRAM */
    LCD_WriteIndex(GRAM);
    /* Start data transmittion */
    SPI_CS_LOW;
    LCD_Write_Data_Start();
    for(uint32_t i = 0; i < MAX_SCREEN_X*MAX_SCREEN_Y; i++){
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
	if ((Xpos < MAX_SCREEN_X) && (Ypos < MAX_SCREEN_Y)){
		/* Set cursor to Xpos and Ypos */
		LCD_SetCursor(Xpos, Ypos);
	}
	else{
		/* Set cursor to Xpos and Ypos */
		LCD_SetCursor((Xpos % MAX_SCREEN_X), (Ypos % MAX_SCREEN_Y));
	}
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
	/* Original data taken in is 16 bit, must be split into two bytes to communication with LCD */

    /* Send out MSB */
	SPISendRecvByte(data >> 8);
    /* Send out LSB */
	SPISendRecvByte(data & 0x00FF);
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
	SPI_transmitData(EUSCI_B3_BASE, byte);
    /* Wait as long as busy */
	while(SPI_isBusy(EUSCI_B3_BASE));
    /* Return received value*/
	return SPI_receiveData(EUSCI_B3_BASE);

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
	LCD_WriteReg(HORIZONTAL_GRAM_SET, Ypos);
    /* Set vertical GRAM coordinate (Xpos) */
	LCD_WriteReg(VERTICAL_GRAM_SET, Xpos);

}

/*******************************************************************************
 * Function Name  : LCD_Init
 * Description    : Configures LCD Control lines, sets whole screen black
 * Input          : bool usingTP: determines whether or not to enable TP interrupt
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
    	__disable_irq();

    	P4->DIR &= ~BIT0;
    	P4->IFG &= ~BIT0;
    	P4->IE |= BIT0;
    	P4->IES |= BIT0;
    	NVIC_EnableIRQ(PORT4_IRQn);

    	__enable_irq();
    }

    LCD_reset();

    LCD_WriteReg(0xE5, 0x78F0); /* set SRAM internal timing */
    LCD_WriteReg(DRIVER_OUTPUT_CONTROL, 0x0100); /* set Driver Output Control */
    LCD_WriteReg(DRIVING_WAVE_CONTROL, 0x0700); /* set 1 line inversion */
    LCD_WriteReg(ENTRY_MODE, 0x1038); /* set GRAM write direction and BGR=1 */
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

    LCD_Clear(LCD_BLACK);
}

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
	//0b10010001; //LEFT TO RIGHT: 1; start bit, must be on. 001; selects the channel to have yp and yn drivers
			//0; puts us in 12 bit mode (rather than 8 bit)  0; DFR 01; power down mode select disabled
    /* Read X coord. */
	TP_ReadX();
    /* Read Y coord. */
	TP_ReadY();
    /* Return point  */
	return myData;
}

/************************************  Public Functions  *******************************************/
uint16_t TP_ReadX(){ // need to do some or'ing and whatnot

	SPI_CS_TP_LOW;
	uint16_t val1 = SPISendRecvByte(CHX);
	uint16_t val2 = SPISendRecvByte(CHX); //00000000 1111 11100000
	uint16_t val3 = SPISendRecvByte(CHX); //00000000 11111000
	SPI_CS_TP_HIGH;

	val2 = (val2<<5);
	val3 = (val3>>3);
	val1 = (val2 | val3);
	val1 = val1 & 0b0000111111111111;

	myData.x = val1;
	return myData.x;
}

uint16_t TP_ReadY(){

	SPI_CS_TP_LOW;
	uint16_t val1 = SPISendRecvByte(CHY);
	uint16_t val2 = SPISendRecvByte(CHY);
	uint16_t val3 = SPISendRecvByte(CHY);
	SPI_CS_TP_HIGH;

	val2 = (val2<<5); // was 5
	val3 = (val3>>3); // was 3
	val1 = (val2 | val3);
	val1 = val1 & 0b0000111111111111;

	myData.y = val1;
	return myData.y;
 }

/* Returns the new position of the ball for the x and y */
int16_t rectangleVelocity(int16_t accelData){
	int16_t pos = 0;
	if (accelData < -12000){
		pos = -3;
        return pos;
	}
	else if(accelData > 12000){
		pos = 3;
		return pos;
	}
	else if((accelData < 12000)&&(accelData > 6000)){
		pos = 2;
		return pos;
	}
	else if((accelData < -6000)&&(accelData > -12000)){
		pos = -2;
		return pos;
	}
	else if((accelData < 6000)&&(accelData > 800)){
		pos = 1;
		return pos;
	}
	else if((accelData < -800)&&(accelData > -6000)){
		pos = -1;
		return pos;
		}
	else{
		pos = 0;
		return pos;
	}
}
