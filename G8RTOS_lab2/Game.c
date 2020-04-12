
#include "Game.h"

/* Under Development ********************/
//FIXME Leaves behind colored pixels when changing direction
//FIXME Seems very fast/sensitive.  May want to slow down how quickly
//      the paddle moves

/*
 * Reads joystick to move positions
 */
int16_t lPos = MAX_SCREEN_X/2 - PADDLE_LEN_D2;
int16_t rPos = MAX_SCREEN_X/2 + PADDLE_LEN_D2;
void JoystickRead(void){
    while(1){
        int16_t xCoor = 0;
        int16_t yCoor = 0;
        GetJoystickCoordinates(&xCoor, &yCoor);
        if(xCoor > 1500){
            //Being pushed left
            xCoor = 1;
            writeFIFO(0, (int32_t)xCoor);
        }
        else if(xCoor < -2000){
            //Being pushed right
            xCoor = -1;
            writeFIFO(0, (int32_t)xCoor);
        }
        else{
            xCoor = 0;
        }

        sleep(60);
    }
}


/*
 * Draws a Paddle by only drawing the differences
 */
void DrawPaddle(void){
    while(1){
        int32_t pos = readFIFO(0);
        if(pos == 1){
            //Update left and right positions
            lPos = lPos - 1;
            if(lPos < 0){
                lPos = 0;
            }
            rPos = rPos - 1;
            if(rPos < PADDLE_LEN){
                rPos = PADDLE_LEN;
            }
            //Draw color on the left, black on right
            LCD_DrawRectangle(lPos-1, lPos, MAX_SCREEN_Y - PADDLE_WID, MAX_SCREEN_Y, LCD_BLUE);
            LCD_DrawRectangle(rPos, rPos+1, MAX_SCREEN_Y - PADDLE_WID, MAX_SCREEN_Y, LCD_BLACK);
        }
        else if(pos == -1){
            //Draw to the right
            lPos = lPos + 1;
            if(lPos > MAX_SCREEN_X - PADDLE_LEN){
                lPos = MAX_SCREEN_X - PADDLE_LEN;
            }
            rPos = rPos + 1;
            if(rPos > MAX_SCREEN_X){
                rPos = MAX_SCREEN_X;
            }
            //Draw color on the Right, black on Left
            LCD_DrawRectangle(lPos-1, lPos, MAX_SCREEN_Y - PADDLE_WID, MAX_SCREEN_Y, LCD_BLACK);
            LCD_DrawRectangle(rPos, rPos+1, MAX_SCREEN_Y - PADDLE_WID, MAX_SCREEN_Y, LCD_BLUE);
        }
        else{
            //Nothing- Joystick in middle position
        }
        sleep(30);
    }
}



void Idle(void){
    while(1);
}
