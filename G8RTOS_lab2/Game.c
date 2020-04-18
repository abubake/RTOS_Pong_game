#include "Game.h"
#include "Joystick.h"
#include "G8RTOS.h"
#include <stdlib.h>
#include "LCD.h"
#include "RGBLeds.h"

SpecificPlayerInfo_t clientToHostInfo;

/* Joystick Info */
int16_t X_coord;
int16_t Y_coord;
int16_t host_X_coord; // Since I have two joystick read functions, shouldn't be an issue since one will be host and
						//other board won't
int16_t host_Y_coord;

/*Ball related Info */
balls_t myBalls[MAX_NUM_OF_BALLS];
int ballNumber;
int curBalls = 0;

int HostPoints = 0; // Number of points (i.e. LED's that are on)
bool pointScored = false;
bool iterated = false; //for the LEDs
int displayVal = 1; //No so that first LED comes on correctly

/* The paddles */
GeneralPlayerInfo_t PlayerPaddle;
GeneralPlayerInfo_t ClientPaddle;

//Previous Locations of Players
PrevPlayer_t prevHostLoc;
PrevPlayer_t prevClientLoc;

int direction = 0; //Direction of the paddle

//Game state to be sent from host to client
GameState_t curGame;

bool isClient;

/*********************************************** Client Threads *********************************************************************/
/*
 * Thread for client to join game
 */
void JoinGame(){

	/* NEED TO SET ALL THESE WITH REAL VALUES */
	clientToHostInfo.IP_address = getLocalIP();
	clientToHostInfo.acknowledge = false;
	clientToHostInfo.displacement = 0;
	clientToHostInfo.joined = false;
	clientToHostInfo.playerNumber = 0;
	clientToHostInfo.ready = false;

		/*
		Only thread to run after launching the OS
		� Set initial SpecificPlayerInfo_t struct attributes (you can get the IP address by calling getLocalIP() (SETUP)
		� Send player into to the host (?)
		� Wait for server response (?)
		� If you�ve joined the game, acknowledge you�ve joined to the host and show connection with an LED (?)
		� Initialize the board state, semaphores, and add the following threads (?)
		o ReadJoystickClient
		o SendDataToHost
		o ReceiveDataFromHost
		o DrawObjects
		o MoveLEDs
		o Idle (SETUP)
		� Kill self (SETUP)
		*/
	G8RTOS_KillSelf();
	while(1); // feeling cute, might delete later
}

/*
 * Thread that receives game state packets from host
 */
void ReceiveDataFromHost(){
	while(1){
		/*
		� (?) Continually receive data until a return value greater than zero is returned (meaning valid data has been read)
		o Note: Remember to release and take the semaphore again so you�re still able to send data
		o Sleeping here for 1ms would avoid a deadlock
		*/
		sleep(1);
		/*
		� Empty the received packet
		� If the game is done, add EndOfGameClient thread with the highest priority
		� Sleep for 5ms
		*/
		sleep(5);

	}
}

/*
 * Thread that sends UDP packets to host
 */
void SendDataToHost(){
	while(1){
		//send data to host and sleep (need to fill in paramters of function (from cc3100_usage.h))

		//SendData(_u8 *data, _u32 IP, _u16 BUF_SIZE);
		sleep(2);
	}
}

/*
 * Thread to read client's joystick
 */
void ReadJoystickClient(){
	while(1){
		/*
		� Read joystick and add offset
		� Add Displacement to Self accordingly
		� Sleep 10ms
		*/
		GetJoystickCoordinates(&X_coord, &Y_coord); //must wait for its semaphore!
		// need to add offset
		sleep(10);
	}
}

/*
 * End of game for the client
 */
void EndOfGameClient(){
	/*
	� Wait for all semaphores to be released
	� Kill all other threads
	� Re-initialize semaphores
	� Clear screen with winner�s color
	� Wait for host to restart game
	� Add all threads back and restart game variables
	� Kill Self
	*/
	LCD_Clear(LCD_RED); //should clear with player's color
	G8RTOS_KillSelf();
}

/*********************************************** Client Threads *********************************************************************/


/*********************************************** Host Threads *********************************************************************/
/*
 * Thread for the host to create a game
 */
void CreateGame(){
	/* WITH WIFI
	� Only thread created before launching the OS
	� Initializes the players
	� Establish connection with client (use an LED on the Launchpad to indicate Wi-Fi connection)
	o Should be trying to receive a packet from the client
	o Should acknowledge client once client has joined
	*/

	/* W/O WIFI
	� Initialize the board (draw arena, players, and scores)
	*/
	InitBoardState();

	/* Add these threads. (Need better priority definitions) */
	G8RTOS_AddThread(GenerateBall, 100, "GenerateBall");
	G8RTOS_AddThread(DrawObjects, 200, "DrawObjects");
	G8RTOS_AddThread(ReadJoystickHost, 201, "ReadJoystickHost");
	//G8RTOS_AddThread(SendDataToClient, 200, "SendDataToClient");
	//G8RTOS_AddThread(ReceiveDataFromClient, 200, "ReceiveDataFromClient");
	G8RTOS_AddThread(MoveLEDs, 250, "MoveLEDs"); //lower priority
	G8RTOS_AddThread(IdleThread, 254, "Idle");

	G8RTOS_KillSelf();
}

/*
 * Thread that sends game state to client
 */
void SendDataToClient(){
	while(1){
		/*
		� Fill packet for client
		� Send packet
		� Check if game is done
		o If done, Add EndOfGameHost thread with highest priority
		� Sleep for 5ms (found experimentally to be a good amount of time for synchronization)
		*/
		sleep(5);
	}
}

/*
 * Thread that receives UDP packets from client
 */
void ReceiveDataFromClient(){
	while(1){
		/*
		� Continually receive data until a return value greater than zero is returned (meaning valid data has been read)
		o Note: Remember to release and take the semaphore again so you�re still able to send data
		o Sleeping here for 1ms would avoid a deadlock
		*/
		sleep(1);
		/*
		� Update the player�s current center with the displacement received from the client
		� Sleep for 2ms (again found experimentally)
		*/
		sleep(2);
	}
}

/*
 * Generate Ball thread
 */
void GenerateBall(){
	while(1){
		/*
		� Adds another MoveBall thread if the number of balls is less than the max
		� Sleeps proportional to the number of balls currently in play
		*/
	    if(curBalls < MAX_NUM_OF_BALLS){
	        curBalls++;
	        G8RTOS_AddThread(MoveBall, 30, "MoveBall");

	    }
	    //TODO Adjust scalar for sleep based on experiments to see what makes the game fun
	    sleep(curBalls*4000);
	}
}

/*
 * Thread to read host's joystick
 */
void ReadJoystickHost(){
    int16_t difference = 0;
	while(1){
		/*
		� You can read the joystick ADC values by calling GetJoystickCoordinates
		� You�ll need to add a bias to the values (found experimentally) since every joystick is offset by some small amount displacement and noise
		� Change Self.displacement accordingly (you can experiment with how much you want to scale the ADC value)
		� Sleep for 10ms
		*/
		GetJoystickCoordinates(&host_X_coord, &host_Y_coord); //must wait for its semaphore!

		if(host_X_coord > 2000){ //FIXME: Adjust this so it will work at different speeds
			//writeFIFO(JOYSTICKFIFO, RIGHT);
		    difference = -1;
		    direction = RIGHT; //For testing purposes
		}
		else if(host_X_coord < -2000){
			//writeFIFO(JOYSTICKFIFO, LEFT);
		    difference = 1;
		    direction = LEFT;
		}
		else{
		    difference = 0;
		}

		PlayerPaddle.currentCenter += difference;
		//Stop from going outside arena
		if(PlayerPaddle.currentCenter < HORIZ_CENTER_MIN_PL){
		    PlayerPaddle.currentCenter = HORIZ_CENTER_MIN_PL;
		}
		else if(PlayerPaddle.currentCenter > HORIZ_CENTER_MAX_PL){
		    PlayerPaddle.currentCenter = HORIZ_CENTER_MAX_PL;
		}


		sleep(10); // makes game for fair

        /*
        � Then add the displacement to the bottom player in the list of players (general list that�s sent to the client and used for drawing) i.e. players[0].position += self.displacement
        � By sleeping before updating the bottom player�s position, it makes the game more fair between client and host
        */
		curGame.players[0].currentCenter += difference;


		prevHostLoc.Center += difference;
        if(prevHostLoc.Center < HORIZ_CENTER_MIN_PL){
            prevHostLoc.Center = HORIZ_CENTER_MIN_PL;
        }
        else if(prevHostLoc.Center > HORIZ_CENTER_MAX_PL){
            prevHostLoc.Center = HORIZ_CENTER_MAX_PL;
        }

	}
}

/*
 * Thread to move a single ball
 */
void MoveBall(){

	/*
	� Go through array of balls and find one that�s not alive
	� Once found, initialize random position and X and Y velocities, as well as color and alive attributes
	� Checking for collision given the current center and the velocity
	� If collision occurs, adjust velocity and color accordingly
	� If the ball passes the boundary edge, adjust score, account for the game possibly ending, and kill self
	� Otherwise, just move the ball in its current direction according to its velocity
	� Sleep for 35ms
	*/
	//Initialize ball if it was newly made
    uint8_t ind;
    for (int i = 0; i < MAX_NUM_OF_BALLS; i++){
        if(myBalls[i].alive == false){ // Searching for the first dead ball

        	//� Once found, initialize random position and X and Y velocities, as well as color and alive attributes
            /* Gives random position */

            //Random x that will be within arena bounds and not too close to a wall
            myBalls[i].xPos = (rand() % (ARENA_MAX_X - ARENA_MIN_X - 20)) + ARENA_MIN_X + 10;
            //Random y that won't be too close to the paddles
            myBalls[i].yPos = (rand() % MAX_SCREEN_Y - 60) + 30;

            /* Getting a random speed */
            //TODO Experimentally determine a good max speed
            int16_t xMag = (rand() % MAX_BALL_SPEED) + 1;
            int16_t yMag = (rand() % MAX_BALL_SPEED) + 1;

            //Get random x-direction
            if(rand() % 2){
                myBalls[i].xVel = xMag * -1;
            }
            else{
                myBalls[i].xVel = xMag;
            }
            //Get random y-direction
            if(rand() % 2){
                myBalls[i].yVel = yMag * -1;
            }
            else{
                myBalls[i].yVel = yMag;
            }

            //Ball is initially white
            myBalls[i].color = LCD_WHITE;
            myBalls[i].alive = true;

            myBalls[i].prevLoc.CenterX = myBalls[i].xPos;
            myBalls[i].prevLoc.CenterY = myBalls[i].yPos;
            myBalls[i].newBall = true;

            //Update structure to be sent
            curGame.numberOfBalls++;
            curGame.balls[i].alive = true;
            curGame.balls[i].color = LCD_WHITE;
            curGame.balls[i].xPos = myBalls[i].xPos;
            curGame.balls[i].yPos = myBalls[i].yPos;
            curGame.balls[i].prevLoc.CenterX = myBalls[i].xPos;
            curGame.balls[i].prevLoc.CenterY = myBalls[i].yPos;
            curGame.balls[i].newBall = true;

            ind = i;
            break;
        }
    }
    	/*
        � If the ball passes the boundary edge, adjust score, account for the game possibly ending, and kill self
        � Otherwise, just move the ball in its current direction according to its velocity
        */
	while(1){
		/* WALL COLLISION DETECTION */
		bool collision = false; //maybe make this global
		bool wall = false; //both variables get reset
		bool paddle = false;

		//Check if it collided with a wall
		if((myBalls[ind].xPos >= ARENA_MAX_X - BALL_SIZE - 4) || (myBalls[ind].xPos <= ARENA_MIN_X + BALL_SIZE + 4)){ // subtracted 9 and added 9 from actual edges to prevent eroding wall effect
			collision = true; //TODO: Handle case where this is within the paddle's range (x = 0 to 4)
			wall = true;
		} //Checks if low enough on y-axis and between paddle left and right bounds to see if a collision occurs
		else if((myBalls[ind].yPos >= 232)&&(myBalls[ind].xPos > PlayerPaddle.currentCenter - PADDLE_LEN_D2 - 2)&&(myBalls[ind].xPos < PlayerPaddle.currentCenter + PADDLE_LEN_D2 + 2)){
			collision = true;
			paddle = true;
		}
		else if((myBalls[ind].yPos <= 9)&&(myBalls[ind].xPos > ClientPaddle.currentCenter - PADDLE_LEN_D2 - 2)&&(myBalls[ind].xPos < ClientPaddle.currentCenter + PADDLE_LEN_D2 + 2)){
		    collision = true;
		    paddle = true;
		}

		if(collision){
			 	if(wall){
			 		//If wall is hit, maintain vertical velocity, but reflect horizontally
			 		myBalls[ind].xVel = myBalls[ind].xVel * -1;
			 	}
			 	//Can be both paddle and wall
			 	if(paddle){
			 	    //Check which paddle it hits
			 	    if(myBalls[ind].yPos > MAX_SCREEN_Y - PADDLE_WID - BALL_SIZE - 7){
			 	        //Hit bottom paddle
			 	        myBalls[ind].color = LCD_RED;
			 	    }
			 	    else if(myBalls[ind].yPos < PADDLE_WID + BALL_SIZE + 7){
			 	        //Collided with top paddle
			 	        myBalls[ind].color = LCD_BLUE;
			 	    }
			 	    //Left Side
			 		if(myBalls[ind].xPos < PlayerPaddle.currentCenter - PADDLE_LEN_D2 + 16 ){
			 		    myBalls[ind].yVel = myBalls[ind].yVel * -1;
			 		    if(myBalls[ind].xVel > 1){
			 		        //Make ball go left
			 		       myBalls[ind].xVel = myBalls[ind].xVel * -1;
			 		    }
			 		}
			 		//Right side
			 		else if(myBalls[ind].xPos > PlayerPaddle.currentCenter + PADDLE_LEN_D2 - 16){
			 			myBalls[ind].yVel = myBalls[ind].yVel * -1;
			 			if(myBalls[ind].xVel < 1){
			 			   myBalls[ind].xVel = myBalls[ind].xVel * -1;
			 			}
			 		}
			 		//Middle
			 		else{ //This is the center of the paddle's area, a space of 24
			 			myBalls[ind].yVel = myBalls[ind].yVel * -1;
			 		}
			 	}
		}

	    //If ball has passed boundary, adjust score
		bool passedBoundary = false;
		if((myBalls[ind].yPos < ARENA_MIN_Y) || (myBalls[ind].yPos > ARENA_MAX_X)){
		    if(myBalls[ind].color == LCD_RED){
		        //Red Ball Scored
		        curBalls--;
		        curGame.numberOfBalls -= 1;
		        curGame.LEDScores[0] += 1;
		        passedBoundary = true;
		    }
		    else if(myBalls[ind].color == LCD_BLUE){
		        //Blue Ball Scored
                curBalls--;
                curGame.numberOfBalls -= 1;
		        curGame.LEDScores[1] += 1;
		        passedBoundary = true;
		    }
		    else{
		        //White Ball fell through- No one scores but ball still is killed
                curBalls--;
                curGame.numberOfBalls -= 1;
		        passedBoundary = true;
		    }
		}
	    //Check if game has ended
		if(curGame.LEDScores[0] >= POINTS_TO_WIN){
		    //Bottom player has won
		    curGame.gameDone = true;        //Not sure what the difference is between gameDone and winner
		    curGame.winner = true;
		}
		else if(curGame.LEDScores[1] >= POINTS_TO_WIN){
		    //Top player has won
		    curGame.gameDone = true;
		    curGame.winner = true;
		}

		if(passedBoundary){
		    if(curGame.gameDone == true){
                //If this was the final point, add ending thread
                G8RTOS_AddThread(EndOfGameHost, 4, "EndOfGame");
		    }
		    else{
                //Kill this MoveBall
                myBalls[ind].alive = false;
                G8RTOS_KillSelf();
		    }

		}
		else{
	        //Save old position
	        myBalls[ind].prevLoc.CenterX = myBalls[ind].xPos;
	        myBalls[ind].prevLoc.CenterY = myBalls[ind].yPos;

	        //If not killed, move ball to position according to its velocity
	        myBalls[ind].xPos = myBalls[ind].xPos + myBalls[ind].xVel;
	        myBalls[ind].yPos = myBalls[ind].yPos + myBalls[ind].yVel;

	        //If went beyond boundary, adjust so it is on boundary
	        //I did this to try to stop the balls from eroding walls
	        if(myBalls[ind].xPos > ARENA_MAX_X - BALL_SIZE){
	            myBalls[ind].xPos = ARENA_MAX_X - BALL_SIZE;
	        }
	        else if(myBalls[ind].xPos < ARENA_MIN_X + BALL_SIZE){
	            myBalls[ind].xPos = ARENA_MAX_X + BALL_SIZE;
	        }
	        //Update GameState to be sent
	        curGame.balls[ind].alive = myBalls[ind].alive;
	        curGame.balls[ind].color = myBalls[ind].color;
	        curGame.balls[ind].xPos = myBalls[ind].xPos;
	        curGame.balls[ind].yPos = myBalls[ind].yPos;
	        curGame.balls[ind].prevLoc.CenterX = myBalls[ind].xPos;
	        curGame.balls[ind].prevLoc.CenterY = myBalls[ind].yPos;
		}

		sleep(35);
	}
}

/*
 * End of game for the host
 */
void EndOfGameHost(){
		/*
		� Kill all other threads (you�ll need to make a new function in the scheduler for this)
		� Re-initialize semaphores
		� Clear screen with the winner�s color
		� Print some message that waits for the host�s action to start a new game
		� Create an aperiodic thread that waits for the host�s button press (the client will just be waiting on the host to start a new game
		� Once ready, send notification to client, reinitialize the game and objects, add back all the threads, and kill self
		*/

    //TODO Wait for all semaphores to be unblocked?

    G8RTOS_KillAllOthers();

    //TODO Reinitialize semaphores

    //Clear screen with winner's color
    if(curGame.LEDScores[0] > curGame.LEDScores[1]){
        //Player 0 won, make screen their color
        LCD_Clear(curGame.players[0].color);
    }
    else if(curGame.LEDScores[0] < curGame.LEDScores[1]){
        //Player 1 won, make screen their color
        LCD_Clear(curGame.players[1].color);
    }

    //TODO Print a message and wait for host's action to start

    //TODO Create aperiodic thread waiting for host's action

    //TODO When ready, notify client, reinitialize game, add threads back, kill self


}

/*********************************************** Host Threads *********************************************************************/


/*********************************************** Common Threads *********************************************************************/
/*
 * Idle thread
 */
void IdleThread(){
	while(1);
}

/*
 * Thread to draw all the objects in the game
 */
void DrawObjects(){
	while(1){
		/*
		� Should hold arrays of previous players and ball positions
		� Draw and/or update balls (you�ll need a way to tell whether to draw a new ball, or update its position
		(i.e. if a new ball has just been created � hence the alive attribute in the Ball_t struct.
		� Update players
		� Sleep for 20ms (reasonable refresh rate)
		*/

	    //FIXME Sometimes after a very long time the balls leave a trail behind them, hard to make happen so hard
	    //      to observe and debug.  Unclear what causes this, but may be a non-issue once we get collisions
	    //      and scoring/ball suicide working.

	    //Update Ball Locations
	    for(uint8_t i = 0; i < MAX_NUM_OF_BALLS; i++){
	        //Check if ball is alive
	        if(myBalls[i].alive){
	            if(myBalls[i].newBall){
	                //If a new ball, paint its initial location
	                int16_t xCoord = myBalls[i].xPos;
	                int16_t yCoord = myBalls[i].yPos;

	                G8RTOS_WaitSemaphore(&USING_SPI);
	                LCD_DrawRectangle(xCoord - BALL_SIZE_D2, xCoord + BALL_SIZE_D2, yCoord - BALL_SIZE_D2, yCoord + BALL_SIZE_D2, LCD_WHITE);
	                G8RTOS_SignalSemaphore(&USING_SPI);

	                //Not new anymore
	                myBalls[i].newBall = false;
	                curGame.balls[i].newBall = false;
	            }
	            else{
	                //If not a new ball, update its location
	                UpdateBallOnScreen(&myBalls[i].prevLoc, &myBalls[i], myBalls[i].color);
	            }
	        }
	    }

	    //Update Players
	    //Update Location of Each Player
	    //Host
	    UpdatePlayerOnScreen(&prevHostLoc, &PlayerPaddle);
	    //Update Client Location
	    UpdatePlayerOnScreen(&prevClientLoc, &ClientPaddle);

	    iterated = false; // After objects are redrawn, we are now able to update LEDs again for points
		sleep(20);
	}
}

/*
 * Thread to update LEDs based on score
 */
void MoveLEDs(){
    uint16_t valToWrite;
	while(1){
		/*
		� Responsible for updating the LED array with current scores
		*/
	    if(!isClient){
	        //Update Host (Red/Bottom)
	        valToWrite = numToLitLEDS(curGame.LEDScores[0]);
	        G8RTOS_WaitSemaphore(&USING_LED_I2C);
	        LP3943_DataDisplay(RED, ON, valToWrite);
	        G8RTOS_SignalSemaphore(&USING_LED_I2C);
	    }
	    else{
	        //Update Client (Blue/Top)
            valToWrite = numToLitLEDS(curGame.LEDScores[1]);
            G8RTOS_WaitSemaphore(&USING_LED_I2C);
            LP3943_DataDisplay(BLUE, ON, valToWrite);
            G8RTOS_SignalSemaphore(&USING_LED_I2C);

	    }

	}
}

/*
 * Takes in a number and returns the number to light up that many LEDs
 * (Left to Right)
 */
inline uint16_t numToLitLEDS(uint8_t playerScore){
    uint16_t toSend = 0;
    //Convert score to set that number of LEDs
    for(uint16_t i = 0; i < playerScore; i++){
        toSend = toSend*2 + 1;
    }
    return toSend;
}

/*********************************************** Common Threads *********************************************************************/

/*********************************************** Public Functions *********************************************************************/
/*
 * Returns either Host or Client depending on button press
 */
playerType GetPlayerRole(){

}

/*
 * Draw players given center X center coordinate
 */
inline void DrawPlayer(GeneralPlayerInfo_t * player){
    if(player->position == TOP){
        //Draw paddle on top of screen
        G8RTOS_WaitSemaphore(&USING_SPI);
        LCD_DrawRectangle(MAX_SCREEN_X/2 - PADDLE_LEN_D2, MAX_SCREEN_X/2 + PADDLE_LEN_D2, TOP_PADDLE_EDGE - PADDLE_WID, TOP_PADDLE_EDGE, player->color);
        G8RTOS_SignalSemaphore(&USING_SPI);
    }
    else if(player->position == BOTTOM){
        //Draw paddle on bottom of screen
        G8RTOS_WaitSemaphore(&USING_SPI);
        LCD_DrawRectangle(MAX_SCREEN_X/2 - PADDLE_LEN_D2, MAX_SCREEN_X/2 + PADDLE_LEN_D2, BOTTOM_PADDLE_EDGE-1, BOTTOM_PADDLE_EDGE + PADDLE_WID, player->color);
        G8RTOS_SignalSemaphore(&USING_SPI);
    }
}

/*
 * Updates player's paddle based on current and new center
 */
inline void UpdatePlayerOnScreen(PrevPlayer_t * prevPlayerIn, GeneralPlayerInfo_t * outPlayer){
    //Find how much the paddle has moved
    int16_t pixelsMoved = outPlayer->currentCenter - prevPlayerIn->Center;
    //In current-previous, positive values mean moved rightward
    //FIXME Direction Changes sometimes leaves a small black line behind inside paddle
    if(outPlayer->position == TOP){
        if(pixelsMoved > 0){
            //Moved rightward
            //Paint background on left side
            G8RTOS_WaitSemaphore(&USING_SPI);
            LCD_DrawRectangle(prevPlayerIn->Center - PADDLE_LEN_D2 - PRINT_OFFSET, outPlayer->currentCenter - PADDLE_LEN_D2, TOP_PADDLE_EDGE - PADDLE_WID, TOP_PADDLE_EDGE, BACK_COLOR);
            G8RTOS_SignalSemaphore(&USING_SPI);
            //Paint color on right side
            G8RTOS_WaitSemaphore(&USING_SPI);
            LCD_DrawRectangle(prevPlayerIn->Center + PADDLE_LEN_D2, outPlayer->currentCenter + PADDLE_LEN_D2, TOP_PADDLE_EDGE - PADDLE_WID, TOP_PADDLE_EDGE, outPlayer->color);
            G8RTOS_SignalSemaphore(&USING_SPI);

            //If was close to wall, repaint white wall
            if(prevPlayerIn->Center <= ARENA_MIN_X + PADDLE_LEN_D2 + PRINT_OFFSET){
                G8RTOS_WaitSemaphore(&USING_SPI);
                LCD_DrawRectangle(ARENA_MIN_X - PRINT_OFFSET, ARENA_MIN_X, ARENA_MIN_Y, ARENA_MIN_Y + PADDLE_WID, LCD_WHITE);
                G8RTOS_SignalSemaphore(&USING_SPI);
            }

            //If was close to wall, repaint white wall
            if(prevPlayerIn->Center <= ARENA_MIN_X + PADDLE_LEN_D2 + PRINT_OFFSET){
                G8RTOS_WaitSemaphore(&USING_SPI);
                LCD_DrawRectangle(ARENA_MIN_X - PRINT_OFFSET, ARENA_MIN_X,  ARENA_MIN_Y, ARENA_MIN_Y + PADDLE_WID + 1, LCD_WHITE);
                G8RTOS_SignalSemaphore(&USING_SPI);
            }
        }
        else if(pixelsMoved < 0){
            //Moved leftward
            //Paint black on right
            G8RTOS_WaitSemaphore(&USING_SPI);
            LCD_DrawRectangle(outPlayer->currentCenter + PADDLE_LEN_D2, prevPlayerIn->Center + PADDLE_LEN_D2 + PRINT_OFFSET, TOP_PADDLE_EDGE - PADDLE_WID, TOP_PADDLE_EDGE, BACK_COLOR);
            G8RTOS_SignalSemaphore(&USING_SPI);
            //Paint color on left side
            G8RTOS_WaitSemaphore(&USING_SPI);
            LCD_DrawRectangle(outPlayer->currentCenter - PADDLE_LEN_D2, prevPlayerIn->Center - PADDLE_LEN_D2, TOP_PADDLE_EDGE - PADDLE_WID, TOP_PADDLE_EDGE, outPlayer->color);
            G8RTOS_SignalSemaphore(&USING_SPI);

            //If was close to wall, repaint white wall
            if(prevPlayerIn->Center >= ARENA_MAX_X - PADDLE_LEN_D2 - PRINT_OFFSET){
                G8RTOS_WaitSemaphore(&USING_SPI);
                LCD_DrawRectangle(ARENA_MAX_X, ARENA_MAX_X + PRINT_OFFSET, ARENA_MIN_Y, ARENA_MIN_Y + PADDLE_WID + 1, LCD_WHITE);
                G8RTOS_SignalSemaphore(&USING_SPI);
            }

        }
    }
    else if(outPlayer->position == BOTTOM){
        if(pixelsMoved > 0){
            //Moved rightward
            //Paint background on left side
            G8RTOS_WaitSemaphore(&USING_SPI);
            LCD_DrawRectangle(prevPlayerIn->Center - PADDLE_LEN_D2 - PRINT_OFFSET, outPlayer->currentCenter - PADDLE_LEN_D2, BOTTOM_PADDLE_EDGE-1, BOTTOM_PADDLE_EDGE + PADDLE_WID, BACK_COLOR);
            G8RTOS_SignalSemaphore(&USING_SPI);
            //Paint color on right side
            G8RTOS_WaitSemaphore(&USING_SPI);
            LCD_DrawRectangle(prevPlayerIn->Center + PADDLE_LEN_D2, outPlayer->currentCenter + PADDLE_LEN_D2, BOTTOM_PADDLE_EDGE-1, BOTTOM_PADDLE_EDGE + PADDLE_WID, outPlayer->color);
            G8RTOS_SignalSemaphore(&USING_SPI);

            //If was close to wall, repaint white wall
            if(prevPlayerIn->Center <= ARENA_MIN_X + PADDLE_LEN_D2 + PRINT_OFFSET){
                G8RTOS_WaitSemaphore(&USING_SPI);
                LCD_DrawRectangle(ARENA_MIN_X - PRINT_OFFSET, ARENA_MIN_X, ARENA_MAX_Y - PADDLE_WID - 1, ARENA_MAX_Y, LCD_WHITE);
                G8RTOS_SignalSemaphore(&USING_SPI);
            }
        }
        else if(pixelsMoved < 0){
            //Moved leftward
            //Paint black on right
            G8RTOS_WaitSemaphore(&USING_SPI);
            LCD_DrawRectangle(outPlayer->currentCenter + PADDLE_LEN_D2, prevPlayerIn->Center + PADDLE_LEN_D2 + PRINT_OFFSET, BOTTOM_PADDLE_EDGE-1, BOTTOM_PADDLE_EDGE + PADDLE_WID, BACK_COLOR);
            G8RTOS_SignalSemaphore(&USING_SPI);
            //Paint color on left side
            G8RTOS_WaitSemaphore(&USING_SPI);
            LCD_DrawRectangle(outPlayer->currentCenter - PADDLE_LEN_D2, prevPlayerIn->Center - PADDLE_LEN_D2, BOTTOM_PADDLE_EDGE-1, BOTTOM_PADDLE_EDGE + PADDLE_WID, outPlayer->color);
            G8RTOS_SignalSemaphore(&USING_SPI);

            //If was close to wall, repaint white wall
            if(prevPlayerIn->Center >= ARENA_MAX_X - PADDLE_LEN_D2 - PRINT_OFFSET){
                G8RTOS_WaitSemaphore(&USING_SPI);
                LCD_DrawRectangle(ARENA_MAX_X, ARENA_MAX_X + PRINT_OFFSET, ARENA_MAX_Y - PADDLE_WID - 1, ARENA_MAX_Y, LCD_WHITE);
                G8RTOS_SignalSemaphore(&USING_SPI);
            }

        }
    }



}

/*
 * Function updates ball position on screen
 *
 * Changed the Ball_t parameter to balls_t that we made
 */
inline void UpdateBallOnScreen(PrevBall_t * previousBall, balls_t * currentBall, uint16_t outColor){
    //If not new, look for previous location
    int16_t prevX = previousBall->CenterX;
    int16_t prevY = previousBall->CenterY;

    //Paint background color over where it was
    //Check if it is going to erase in a bad place
    if(prevX < ARENA_MIN_X || prevX > ARENA_MAX_X){

    }
    else{
        G8RTOS_WaitSemaphore(&USING_SPI);
        LCD_DrawRectangle(prevX- BALL_SIZE_D2, prevX + BALL_SIZE_D2, prevY - BALL_SIZE_D2, prevY + BALL_SIZE_D2, BACK_COLOR);
        G8RTOS_SignalSemaphore(&USING_SPI);
    }


    //Paint where it is now
    //Check if in boundary
    if(currentBall->yPos > MAX_SCREEN_Y || (currentBall->yPos < MIN_SCREEN_Y)){
        //Out of bounds, Do not draw
    }
    else{
        G8RTOS_WaitSemaphore(&USING_SPI);
        LCD_DrawRectangle(currentBall->xPos - BALL_SIZE_D2, currentBall-> xPos + BALL_SIZE_D2, currentBall->yPos - BALL_SIZE_D2, currentBall->yPos + BALL_SIZE_D2, outColor);
        G8RTOS_SignalSemaphore(&USING_SPI);
    }

}


/*
 * Initializes and prints initial game state
 */
inline void InitBoardState(){
	/* White lines to define arena size */
	G8RTOS_WaitSemaphore(&USING_SPI);
	LCD_DrawRectangle(1, 40, 1, 239, LCD_WHITE);
	G8RTOS_SignalSemaphore(&USING_SPI);

	G8RTOS_WaitSemaphore(&USING_SPI);
	LCD_DrawRectangle(280, 319, 1, 239, LCD_WHITE);
	G8RTOS_SignalSemaphore(&USING_SPI);

	//Write Player Scores
	uint8_t scores[3];
	setScoreString(&scores, 0);
	G8RTOS_WaitSemaphore(&USING_SPI);
	LCD_Text(5, 5, scores, PLAYER_BLUE);
	G8RTOS_SignalSemaphore(&USING_SPI);

	//curGame.overallScores[1] = 37;    //Testing high scores
	setScoreString(scores, 1);
    G8RTOS_WaitSemaphore(&USING_SPI);
    LCD_Text(5, MAX_SCREEN_Y - 20, scores, PLAYER_RED);
    G8RTOS_SignalSemaphore(&USING_SPI);

	/* The initial paddle */
    /* Set Center of player paddle */
    PlayerPaddle.currentCenter = PADDLE_X_CENTER;
    PlayerPaddle.position = BOTTOM;
    PlayerPaddle.color = PLAYER_RED;
    DrawPlayer(&PlayerPaddle);

    ClientPaddle.currentCenter = PADDLE_X_CENTER;
    ClientPaddle.position = TOP;
    ClientPaddle.color = PLAYER_BLUE;
    DrawPlayer(&ClientPaddle);

    //Save these in game state
    curGame.players[0].color = PLAYER_RED;
    curGame.players[0].currentCenter = PADDLE_X_CENTER;
    curGame.players[0].position = BOTTOM;
    curGame.LEDScores[0] = 0;

    curGame.players[1].currentCenter = PADDLE_X_CENTER;
    curGame.players[1].position = TOP;
    curGame.players[1].color = PLAYER_BLUE;
    curGame.LEDScores[1] = 0;

    prevHostLoc.Center = PADDLE_X_CENTER;
    prevClientLoc.Center = PADDLE_X_CENTER;
}

inline void setScoreString(uint8_t scoreArray[3], uint16_t playerIndex){
    //Get player's score
    uint8_t curScore = curGame.overallScores[playerIndex];

    //Set array with characters that are that score
    scoreArray[0] = curScore/10 + 0x30;
    scoreArray[1] = curScore%10 + 0x30;
    scoreArray[2] = 0x00;
}


/*********************************************** Public Functions *********************************************************************/
