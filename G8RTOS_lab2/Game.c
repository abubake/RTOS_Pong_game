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

/* The paddle */
GeneralPlayerInfo_t PlayerPaddle;
int direction = 0; //Direction of the paddle

//Game state to be sent from host to client
GameState_t curGame;

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
		• Set initial SpecificPlayerInfo_t struct attributes (you can get the IP address by calling getLocalIP() (SETUP)
		• Send player into to the host (?)
		• Wait for server response (?)
		• If you’ve joined the game, acknowledge you’ve joined to the host and show connection with an LED (?)
		• Initialize the board state, semaphores, and add the following threads (?)
		o ReadJoystickClient
		o SendDataToHost
		o ReceiveDataFromHost
		o DrawObjects
		o MoveLEDs
		o Idle (SETUP)
		• Kill self (SETUP)
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
		• (?) Continually receive data until a return value greater than zero is returned (meaning valid data has been read)
		o Note: Remember to release and take the semaphore again so you’re still able to send data
		o Sleeping here for 1ms would avoid a deadlock
		*/
		sleep(1);
		/*
		• Empty the received packet
		• If the game is done, add EndOfGameClient thread with the highest priority
		• Sleep for 5ms
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
		• Read joystick and add offset
		• Add Displacement to Self accordingly
		• Sleep 10ms
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
	• Wait for all semaphores to be released
	• Kill all other threads
	• Re-initialize semaphores
	• Clear screen with winner’s color
	• Wait for host to restart game
	• Add all threads back and restart game variables
	• Kill Self
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
	• Only thread created before launching the OS
	• Initializes the players
	• Establish connection with client (use an LED on the Launchpad to indicate Wi-Fi connection)
	o Should be trying to receive a packet from the client
	o Should acknowledge client once client has joined
	*/

	/* W/O WIFI
	• Initialize the board (draw arena, players, and scores)
	*/
	InitBoardState();

	/* Add these threads. (Need better priority definitions) */
	G8RTOS_AddThread(GenerateBall, 100, "GenerateBall");
	G8RTOS_AddThread(DrawObjects, 200, "DrawObjects");
	//G8RTOS_AddThread(ReadJoystickHost, 200, "ReadJoystickHost");
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
		• Fill packet for client
		• Send packet
		• Check if game is done
		o If done, Add EndOfGameHost thread with highest priority
		• Sleep for 5ms (found experimentally to be a good amount of time for synchronization)
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
		• Continually receive data until a return value greater than zero is returned (meaning valid data has been read)
		o Note: Remember to release and take the semaphore again so you’re still able to send data
		o Sleeping here for 1ms would avoid a deadlock
		*/
		sleep(1);
		/*
		• Update the player’s current center with the displacement received from the client
		• Sleep for 2ms (again found experimentally)
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
		• Adds another MoveBall thread if the number of balls is less than the max
		• Sleeps proportional to the number of balls currently in play
		*/
	    if(curBalls < MAX_NUM_OF_BALLS){
	        curBalls++;
	        G8RTOS_AddThread(MoveBall, 30, "MoveBall");
	        curGame.numberOfBalls++;
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
		• You can read the joystick ADC values by calling GetJoystickCoordinates
		• You’ll need to add a bias to the values (found experimentally) since every joystick is offset by some small amount displacement and noise
		• Change Self.displacement accordingly (you can experiment with how much you want to scale the ADC value)
		• Sleep for 10ms
		*/
		GetJoystickCoordinates(&host_X_coord, &host_Y_coord); //must wait for its semaphore!

		if(host_X_coord > 1500){ //FIXME: Adjust this so it will work at different speeds
			//writeFIFO(JOYSTICKFIFO, RIGHT);
		    difference = 1;
		    direction = RIGHT; //For testing purposes
		}
		else if(host_X_coord < -1500){
			//writeFIFO(JOYSTICKFIFO, LEFT);
		    difference = -1;
		    direction = LEFT;
		}
		else{
		    difference = 0;
		}
		PlayerPaddle.currentCenter += difference;
		sleep(10); // makes game for fair

        /*
        • Then add the displacement to the bottom player in the list of players (general list that’s sent to the client and used for drawing) i.e. players[0].position += self.displacement
        • By sleeping before updating the bottom player’s position, it makes the game more fair between client and host
        */
		curGame.players[0].currentCenter += difference;


	}
}

/*
 * Thread to move a single ball
 */
void MoveBall(){

	/*
	• Go through array of balls and find one that’s not alive
	• Once found, initialize random position and X and Y velocities, as well as color and alive attributes
	• Checking for collision given the current center and the velocity
	• If collision occurs, adjust velocity and color accordingly
	• If the ball passes the boundary edge, adjust score, account for the game possibly ending, and kill self
	• Otherwise, just move the ball in its current direction according to its velocity
	• Sleep for 35ms
	*/
	//Initialize ball if it was newly made
    uint8_t ind;
    for (int i = 0; i < MAX_NUM_OF_BALLS; i++){
        if(myBalls[i].alive == false){ // Searching for the first dead ball

        	//• Once found, initialize random position and X and Y velocities, as well as color and alive attributes
            /* Gives random position */

            //Random x that will be within arena bounds and not too close to a wall
            myBalls[i].xPos = (rand() % (ARENA_MAX_X - ARENA_MIN_X - 10)) + ARENA_MIN_X + 5;
            //Random y that won't be too close to the paddles
            myBalls[i].yPos = (rand() % MAX_SCREEN_Y - 50) + 25;

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
        • If the ball passes the boundary edge, adjust score, account for the game possibly ending, and kill self
        • Otherwise, just move the ball in its current direction according to its velocity
        */
	while(1){
		/* WALL COLLISION DETECTION */
		bool collision = false; //maybe make this global
		bool wall = false; //both variables get reset
		bool paddle = false;

		if((myBalls[ind].xPos >= ARENA_MAX_X - BALL_SIZE - 3) || (myBalls[ind].xPos <= ARENA_MIN_X + BALL_SIZE + 3)){ // subtracted 9 and added 9 from actual edges to prevent eroding wall effect
			collision = true; //TODO: Handle case where this is within the paddle's range (x = 0 to 4)
			wall = true;
		} //Checks if low enough on y-axis and between paddle left and right bounds to see if a collision occurs
		else if((myBalls[ind].yPos >= 230)&&(myBalls[ind].xPos > PlayerPaddle.paddleLeftEdge)&&(myBalls[ind].xPos < PlayerPaddle.paddleRightEdge)){ //42 is (32 + offset) of 10
			collision = true;
			paddle = true;
		}

		if(collision){
			 	if(wall){
			 		//If wall is hit, maintain vertical velocity, but reflect horizontally
			 		myBalls[ind].xVel = myBalls[ind].xVel * -1;
			 	}
			 	else if(paddle){
			 		if(myBalls[ind].xPos < PlayerPaddle.paddleLeftEdge + 30 ){ //On the left side of paddle, ask baker about 30
			 			myBalls[ind].yVel = myBalls[ind].yVel * -1;
			 			myBalls[ind].xVel = myBalls[ind].xVel * -1; //TODO: Ensure this goes left
			 		}
			 		else if(myBalls[ind].xPos > PlayerPaddle.paddleRightEdge - 30){ //On the right of paddle
			 			myBalls[ind].yVel = myBalls[ind].yVel * -1;
			 			myBalls[ind].xVel = myBalls[ind].xVel * 1; //TODO: Ensure this goes right
			 		}
			 		else{ //This is the center of the paddle's area, a space of 24
			 			myBalls[ind].yVel = myBalls[ind].yVel * -1;
			 		}

			 	}

		}


	    //TODO If ball has passed boundary, adjust score

	    //TODO If score happened, adjust score and killself
	    //TODO Check if game has ended


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

		sleep(35);
	}
}

/*
 * End of game for the host
 */
void EndOfGameHost(){
		/*
		• Kill all other threads (you’ll need to make a new function in the scheduler for this)
		• Re-initialize semaphores
		• Clear screen with the winner’s color
		• Print some message that waits for the host’s action to start a new game
		• Create an aperiodic thread that waits for the host’s button press (the client will just be waiting on the host to start a new game
		• Once ready, send notification to client, reinitialize the game and objects, add back all the threads, and kill self
		*/
		LCD_Clear(LCD_RED);


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
		• Should hold arrays of previous players and ball positions
		• Draw and/or update balls (you’ll need a way to tell whether to draw a new ball, or update its position
		(i.e. if a new ball has just been created – hence the alive attribute in the Ball_t struct.
		• Update players
		• Sleep for 20ms (reasonable refresh rate)
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
	    //TODO: Make it update both players
	     //readFIFO(JOYSTICKFIFO);  // = ReadFIFO(JOYSTICKFIFO) (Is it going left or right?)

	    if(direction == LEFT){
	    	G8RTOS_WaitSemaphore(&USING_SPI);
	    	LCD_DrawRectangle(PlayerPaddle.paddleRightEdge - 10, PlayerPaddle.paddleRightEdge, 239, 235, LCD_BLACK);
	    	G8RTOS_SignalSemaphore(&USING_SPI);
	    	G8RTOS_WaitSemaphore(&USING_SPI);
	    	LCD_DrawRectangle(PlayerPaddle.paddleLeftEdge + 10, PlayerPaddle.paddleLeftEdge, 239, 235, LCD_RED);
	    	G8RTOS_SignalSemaphore(&USING_SPI);
	    	PlayerPaddle.currentCenter -= 10;
	    	PlayerPaddle.paddleRightEdge -= 10;
	    	PlayerPaddle.paddleLeftEdge -= 10;
	    		//FIXME: Make the color based on who's paddle it is
	    }
	    else if(direction == RIGHT){
	    	G8RTOS_WaitSemaphore(&USING_SPI);
	    	LCD_DrawRectangle(PlayerPaddle.paddleLeftEdge, PlayerPaddle.paddleLeftEdge + 10, 239, 235, LCD_BLACK);
	    	G8RTOS_SignalSemaphore(&USING_SPI);
	    	G8RTOS_WaitSemaphore(&USING_SPI);
	    	LCD_DrawRectangle(PlayerPaddle.paddleRightEdge, PlayerPaddle.paddleRightEdge + 10, 239, 235, LCD_RED);
	    	G8RTOS_SignalSemaphore(&USING_SPI);
	    	PlayerPaddle.currentCenter += 10;
	    	PlayerPaddle.paddleRightEdge += 10;
	    	PlayerPaddle.paddleLeftEdge += 10;
	    }

	    iterated = false; // After objects are redrawn, we are now able to update LEDs again for points
		sleep(20);
	}
}

/*
 * Thread to update LEDs based on score
 */
void MoveLEDs(){
	while(1){
		/*
		• Responsible for updating the LED array with current scores
		*/
		if(iterated == false){ //Run the code
			iterated = true;
			for(int i=0; i<curBalls; i++){
				if((myBalls[i].yPos < 3)&&(myBalls[i].prevLoc.CenterY >= 3)){
					pointScored = true;
				}
			}

			if(pointScored == true){
				pointScored = false; //For the next time
				HostPoints += 1;
				LP3943_DataDisplay(RED, ON, displayVal); //Display val init'd to 1, so from then on it displays the right val
				displayVal = displayVal*2 + 1;
			}
		}

	}
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
void DrawPlayer(GeneralPlayerInfo_t * player){

}

/*
 * Updates player's paddle based on current and new center
 */
void UpdatePlayerOnScreen(PrevPlayer_t * prevPlayerIn, GeneralPlayerInfo_t * outPlayer){

}

/*
 * Function updates ball position on screen
 *
 * Changed the Ball_t parameter to balls_t that we made
 */
void UpdateBallOnScreen(PrevBall_t * previousBall, balls_t * currentBall, uint16_t outColor){
    //If not new, look for previous location
    int16_t prevX = previousBall->CenterX;
    int16_t prevY = previousBall->CenterY;

    //Paint background color over where it was
    G8RTOS_WaitSemaphore(&USING_SPI);
    LCD_DrawRectangle(prevX- BALL_SIZE_D2, prevX + BALL_SIZE_D2, prevY - BALL_SIZE_D2, prevY + BALL_SIZE_D2, BACK_COLOR);
    G8RTOS_SignalSemaphore(&USING_SPI);

    //Paint where it is now
    G8RTOS_WaitSemaphore(&USING_SPI);
    LCD_DrawRectangle(currentBall->xPos - BALL_SIZE_D2, currentBall-> xPos + BALL_SIZE_D2, currentBall->yPos - BALL_SIZE_D2, currentBall->yPos + BALL_SIZE_D2, outColor);
    G8RTOS_SignalSemaphore(&USING_SPI);
}


/*
 * Initializes and prints initial game state
 */
void InitBoardState(){
	/* White lines to define arena size */
	G8RTOS_WaitSemaphore(&USING_SPI);
	LCD_DrawRectangle(1, 40, 1, 239, LCD_WHITE);
	G8RTOS_SignalSemaphore(&USING_SPI);

	G8RTOS_WaitSemaphore(&USING_SPI);
	LCD_DrawRectangle(280, 319, 1, 239, LCD_WHITE);
	G8RTOS_SignalSemaphore(&USING_SPI);

	/* Host Starting Score, in bottom left */
	G8RTOS_WaitSemaphore(&USING_SPI);
	LCD_Text(5, 5,"00", PLAYER_RED);
	G8RTOS_SignalSemaphore(&USING_SPI);

	/* The initial paddle */
	G8RTOS_WaitSemaphore(&USING_SPI);
	LCD_DrawRectangle(128, 192, 235, 239, PLAYER_RED);
	G8RTOS_SignalSemaphore(&USING_SPI);

	/* Set Center of player paddle */
	PlayerPaddle.currentCenter = PADDLE_X_CENTER;
	PlayerPaddle.paddleRightEdge = PlayerPaddle.currentCenter + 42;
	PlayerPaddle.paddleLeftEdge = PlayerPaddle.currentCenter - 42;


}

/*********************************************** Public Functions *********************************************************************/
