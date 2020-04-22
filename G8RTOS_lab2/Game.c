#include "Game.h"
#include "Joystick.h"
#include "G8RTOS.h"
#include <stdlib.h>
#include "LCD.h"
#include "RGBLeds.h"
#include "cc3100_usage.h"
#include "demo_sysctl.h"
#include "G8RTOS_Scheduler.h"

/* Joystick Info */
int16_t X_coord;
int16_t Y_coord;
int16_t host_X_coord; // Since I have two joystick read functions, shouldn't be an issue since one will be host and
						//other board won't
int16_t host_Y_coord;


int HostPoints = 0; // Number of points (i.e. LED's that are on)
bool pointScored = false;
bool iterated = false; //for the LEDs
int displayVal = 1; //No so that first LED comes on correctly

//Previous Locations of Players for updating drawings
PrevPlayer_t prevHostLoc;
PrevPlayer_t prevClientLoc;
PrevBall_t prevBallLocs[MAX_NUM_OF_BALLS];

int direction = 0; //Direction of the paddle for testing

//Game state to be sent from host to client
GameState_t curGame;
//Player state that client sends to host
SpecificPlayerInfo_t clientToHostInfo;

uint32_t clientIP = 0; //the client's IP address that is recieved by the host

//ISR bools
bool isClient = false;
bool readyForGame = false;
bool NewGame = false;

//acks for handshake
uint8_t C2H_ack = 12;
uint8_t H2C_ack = 9;


/*********************************************** Client Threads *********************************************************************/
/*
 * Thread for client to join game
 */
void JoinGame(){
		/*
		Only thread to run after launching the OS
		• Set initial SpecificPlayerInfo_t struct attributes (you can get the IP address by calling getLocalIP() (SETUP)
		• Send player info to the host
		• Wait for server response
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

	/* NEED TO SET ALL THESE WITH REAL VALUES */
	clientToHostInfo.IP_address = getLocalIP();
	clientToHostInfo.acknowledge = true;
	clientToHostInfo.displacement = MAX_SCREEN_X/2;
	clientToHostInfo.joined = true;
	clientToHostInfo.playerNumber = 1;
	clientToHostInfo.ready = true;
	clientToHostInfo.playerNumber = TOP;

	/* Sends the Client's IP address to the host */
	int retval = -1;

	//pinging
	while(retval < 0 || !clientToHostInfo.acknowledge){
	    clientToHostInfo.acknowledge = true;
	    SendData((uint8_t *)&clientToHostInfo, HOST_IP_ADDR, sizeof(clientToHostInfo));
        clientToHostInfo.acknowledge = false;
	    retval = ReceiveData((uint8_t *)&clientToHostInfo, sizeof(clientToHostInfo));
	}

	//tell the host that I am ready to join
    SendData((uint8_t *)&clientToHostInfo, HOST_IP_ADDR, sizeof(clientToHostInfo));



	/* Connection established, launch RTOS */
	BITBAND_PERI(P2->DIR, 0) = 1;
	BITBAND_PERI(P2->OUT, 0) = 1;

	InitBoardState(); // The stuff

    G8RTOS_AddThread(ReadJoystickClient, 4, "ReadJoystickClient");
    G8RTOS_AddThread(ReceiveDataFromHost, 3, "ReceiveDataFromHost");
    G8RTOS_AddThread(SendDataToHost, 3, "SendDataToHost");
    G8RTOS_AddThread(DrawObjects, 2, "DrawObjects");
    G8RTOS_AddThread(MoveLEDs, 250, "MoveLEDs");
    G8RTOS_AddThread(IdleThread, 250, "idle");
	G8RTOS_KillSelf();
	DelayMs(1);
}

/*
 * Thread that receives game state packets from host
 */
void ReceiveDataFromHost(){
	while(1){
		/*
		• Continually receive data until a return value greater than zero is returned (meaning valid data has been read)
		o Note: Remember to release and take the semaphore again so you’re still able to send data
		o Sleeping here for 1ms would avoid a deadlock
		*/
	    //Before receiving new host location, update the previous location of host
	    prevHostLoc.Center = curGame.players[0].currentCenter;


	    G8RTOS_WaitSemaphore(&USING_WIFI);
        ReceiveData((uint8_t *)&curGame, sizeof(curGame));
		G8RTOS_SignalSemaphore(&USING_WIFI);
		sleep(1);
		/*
		• Empty the received packet
		• If the game is done, add EndOfGameClient thread with the highest priority
		• Sleep for 5ms
		*/
		if(curGame.gameDone == true){
			G8RTOS_AddThread(EndOfGameClient, 1, "enditallPLZ");
		}

		sleep(5);
	}
}

/*
 * Thread that sends UDP packets to host
 */
void SendDataToHost(){
    uint16_t count = 0;
	while(1){
		//send data to host and sleep (need to fill in parameters of function (from cc3100_usage.h))
	    if(count == 5){
	        G8RTOS_WaitSemaphore(&USING_WIFI);
	        SendData((uint8_t *)&clientToHostInfo, HOST_IP_ADDR, sizeof(clientToHostInfo));
	        G8RTOS_SignalSemaphore(&USING_WIFI);
	        count = 0;
	    }
	    count++;

		sleep(2);
	}
}

/*
 * Thread to read client's joystick
 */
void ReadJoystickClient(){
    int16_t clientXCoord;
    int16_t clientYCoord;
    int16_t clientDifference;
	while(1){
		/*
		• Read joystick and add offset
		• Add Displacement to Self accordingly
		• Sleep 10ms
		*/
	    GetJoystickCoordinates(&clientXCoord, &clientYCoord);
	    if(clientXCoord > 2000){
	        clientDifference = -1;
	    }
	    else if(clientXCoord < -2000){
	        clientDifference = 1;
	    }
	    else{
	        clientDifference = 0;
	    }

	    //Set previous location as old location
	    //prevClientLoc.Center = clientToHostInfo.displacement; //Should only be updated on receipt of new data from host

	    //Update current location in struct
		clientToHostInfo.displacement += clientDifference;
		//Check and fix if out of bounds
        if(clientToHostInfo.displacement < HORIZ_CENTER_MIN_PL){
            clientToHostInfo.displacement = HORIZ_CENTER_MIN_PL;
        }
        else if(clientToHostInfo.displacement > HORIZ_CENTER_MAX_PL){
            clientToHostInfo.displacement = HORIZ_CENTER_MAX_PL;
        }

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

    //Wait for semaphores to be available so that threads using them are not killed yet
    G8RTOS_WaitSemaphore(&USING_SPI);
    G8RTOS_WaitSemaphore(&USING_LED_I2C);
    G8RTOS_WaitSemaphore(&USING_WIFI);

    //Now has both semaphores, kill all other threads
    G8RTOS_KillAllOthers();


    if(curGame.LEDScores[0] > curGame.LEDScores[1]){
        //Player 0 won, make screen their color
        LCD_Clear(curGame.players[0].color);
        LCD_Text(90, 75, "Wait for Host Push", curGame.players[1].color);
    }
    else if(curGame.LEDScores[0] < curGame.LEDScores[1]){
        //Player 1 won, make screen their color
        LCD_Clear(curGame.players[1].color);
        LCD_Text(90, 75, "Wait for Host Push", curGame.players[0].color);
    }

    //TODO Wait for host to restart game
    while(NewGame == false){
        /* Sets up a semaphore for indicating if the LED resource and the sensor resource are available */
        ReceiveData((uint8_t *)&curGame , sizeof(curGame));
        if(curGame.gameDone != false){
            //I think this should work as long as curGame has been updated with a new game status
            NewGame = true;
        }
    }

    //Host has pressed

    resetGameExScores();
    //Redraw arena- Assumes host sent new packet with overall game scores in it
    InitBoardState();

    //Reset game variables for sending
    clientToHostInfo.displacement = PADDLE_X_CENTER;

    G8RTOS_InitSemaphore(&USING_LED_I2C, 1);
    G8RTOS_InitSemaphore(&USING_SPI, 1);
    G8RTOS_InitSemaphore(&USING_WIFI, 1);
    /* Add back client threads */
    G8RTOS_AddThread(DrawObjects, 2, "DrawObjects");
    G8RTOS_AddThread(ReadJoystickClient, 3, "ReadJoystickClient");
    G8RTOS_AddThread(SendDataToHost, 3, "SendDataToHost");
    G8RTOS_AddThread(ReceiveDataFromHost, 2, "ReceiveDataFromHost");
    G8RTOS_AddThread(MoveLEDs, 250, "MoveLEDs"); //lower priority
    G8RTOS_AddThread(IdleThread, 254, "Idle");

    //Kill self
    G8RTOS_KillSelf();
    DelayMs(1);
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
	• Initialize the board (draw arena, players, and scores)
	*/
    /* Sets up a semaphore for indicating if the LED resource and the sensor resource are available */

	int retval = -1;
    clientToHostInfo.acknowledge = false;
    //waiting for a client to connect
	    //Not Received  OR  not Acknowledged
    while(retval < 0 || !clientToHostInfo.acknowledge){
        retval = ReceiveData((uint8_t *)&clientToHostInfo, sizeof(clientToHostInfo));
    }

    //Send acknowledge to client allowing to connect
    clientToHostInfo.acknowledge = true;
    SendData((uint8_t *)&clientToHostInfo, clientToHostInfo.IP_address, sizeof(clientToHostInfo));
    clientToHostInfo.acknowledge = false;

    //wait until the client says it has joined
    while(retval < 0 || !clientToHostInfo.acknowledge){
        retval = ReceiveData((uint8_t *)&clientToHostInfo, sizeof(clientToHostInfo));
    }

    BITBAND_PERI(P2->DIR, 0) = 1;
    BITBAND_PERI(P2->OUT, 0) = 1;

	InitBoardState();

	/* Add these threads. (Need better priority definitions) */
    G8RTOS_AddThread(GenerateBall, 6, "GenerateBall");
    G8RTOS_AddThread(ReceiveDataFromClient, 4, "ReceiveDataFromClient");
    G8RTOS_AddThread(DrawObjects, 2, "DrawObjects");
    G8RTOS_AddThread(ReadJoystickHost, 3, "ReadJoystickHost");
    G8RTOS_AddThread(SendDataToClient, 5, "SendDataToClient");
    //G8RTOS_AddThread(MoveLEDs, 250, "MoveLEDs"); //lower priority
    G8RTOS_AddThread(IdleThread, 250, "idle");
	G8RTOS_KillSelf();
	DelayMs(1);
}

/*
 * Thread that sends game state to client
 */
void SendDataToClient(){
    uint16_t count = 0;
	while(1){
		/*
		• Fill packet for client
		• Send packet
		• Check if game is done
		o If done, Add EndOfGameHost thread with highest priority
		• Sleep for 5ms (found experimentally to be a good amount of time for synchronization)
		*/
	    if(count == 5){
	        G8RTOS_WaitSemaphore(&USING_WIFI);
	        SendData((uint8_t *)&curGame, clientToHostInfo.IP_address, sizeof(curGame));
	        G8RTOS_SignalSemaphore(&USING_WIFI);

	        if(curGame.gameDone == true){
	            G8RTOS_AddThread(EndOfGameHost, 1, "desolation"); //The end is approaching
	        }
	        count = 0;
	    }
	    count++;
		sleep(5);
	}
}

/*
 * Thread that receives UDP packets from client
 */
void ReceiveDataFromClient(){
    //Before receiving new client location update previous location
    prevClientLoc.Center = curGame.players[1].currentCenter;
	while(1){
		/*
		• Continually receive data until a return value greater than zero is returned (meaning valid data has been read)
		o Note: Remember to release and take the semaphore again so you’re still able to send data
		o Sleeping here for 1ms would avoid a deadlock
		*/
        G8RTOS_WaitSemaphore(&USING_WIFI);
        ReceiveData((uint8_t *)&clientToHostInfo, sizeof(clientToHostInfo));
        G8RTOS_SignalSemaphore(&USING_WIFI);
		sleep(1);
		/*
		• Update the player’s current center with the displacement received from the client
		*/
		//G8RTOS_WaitSemaphore(&USING_WIFI);
		if((clientToHostInfo.displacement > ARENA_MIN_X + PADDLE_LEN_D2) && (clientToHostInfo.displacement < ARENA_MAX_X - PADDLE_LEN_D2)){
			curGame.players[1].currentCenter = clientToHostInfo.displacement;
		}
		//G8RTOS_SignalSemaphore(&USING_WIFI);
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
	    if(curGame.numberOfBalls < MAX_NUM_OF_BALLS){
	        //G8RTOS_WaitSemaphore(&USING_WIFI);
	        curGame.numberOfBalls++;
	        //G8RTOS_SignalSemaphore(&USING_WIFI);
	        G8RTOS_AddThread(MoveBall, 5, "MoveBall");
	    }
	    //TODO Adjust scalar for sleep based on experiments to see what makes the game fun
	    sleep(curGame.numberOfBalls*2500);
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

		if(host_X_coord > 2000){ //FIXME: Adjust this so it will work at different speeds

		    difference = -1;
		    direction = RIGHT; //For testing purposes
		}
		else if(host_X_coord < -2000){

		    difference = 1;
		    direction = LEFT;
		}
		else{
		    difference = 0;
		}

		sleep(10); // makes game for fair

		//Update prev location as current location
		prevHostLoc.Center = curGame.players[0].currentCenter;

		//Update current location
		//G8RTOS_WaitSemaphore(&USING_WIFI);
        curGame.players[0].currentCenter += difference;
        if(curGame.players[0].currentCenter < HORIZ_CENTER_MIN_PL){
            curGame.players[0].currentCenter  = HORIZ_CENTER_MIN_PL;
        }
        else if(curGame.players[0].currentCenter  > HORIZ_CENTER_MAX_PL){
            curGame.players[0].currentCenter  = HORIZ_CENTER_MAX_PL;
        }
        //G8RTOS_SignalSemaphore(&USING_WIFI);
        sleep(10); //Sleep at the end, maybe we don't need it, may help
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
    //G8RTOS_WaitSemaphore(&USING_WIFI);
    uint8_t ind;
    for (int i = 0; i < MAX_NUM_OF_BALLS; i++){
        if(curGame.balls[i].alive == false){ // Searching for the first dead ball

        	//Once found, initialize random position and X and Y velocities, as well as color and alive attributes
            /* Gives random position */
            //Random x that will be within arena bounds and not too close to a wall
            curGame.balls[i].xPos = (rand() % (ARENA_MAX_X - ARENA_MIN_X - 20)) + ARENA_MIN_X + 10;
            //Random y that won't be too close to the paddles
            curGame.balls[i].yPos = (rand() % MAX_SCREEN_Y - 60) + 30;

            /* Getting a random speed */
            //TODO Experimentally determine a good max speed
            int16_t xMag = (rand() % MAX_BALL_SPEED) + 1;
            int16_t yMag = (rand() % MAX_BALL_SPEED) + 1;

            //Get random x-direction
            if(rand() % 2){
                curGame.balls[i].xVel = xMag * -1;
            }
            else{
                curGame.balls[i].xVel = xMag;
            }
            //Get random y-direction
            if(rand() % 2){
                curGame.balls[i].yVel = yMag * -1;
            }
            else{
                curGame.balls[i].yVel = yMag;
            }

            //Ball is initially white
            curGame.balls[i].color = LCD_WHITE;
            curGame.balls[i].alive = true;

            //Set previous location as this location
            prevBallLocs[i].CenterX = curGame.balls[i].xPos;
            prevBallLocs[i].CenterY = curGame.balls[i].yPos;
            curGame.balls[i].newBall = true;

            ind = i;

            break;
        }
    }
    //G8RTOS_SignalSemaphore(&USING_WIFI);

    	/*
        • If the ball passes the boundary edge, adjust score, account for the game possibly ending, and kill self
        • Otherwise, just move the ball in its current direction according to its velocity
        */
	while(1){
	   // G8RTOS_WaitSemaphore(&USING_WIFI);
		/* WALL COLLISION DETECTION */
		bool collision = false; //maybe make this global
		bool wall = false; //both variables get reset
		bool paddle = false;

		//Check if it collided with a wall
		//TODO mess with values until they are nice
		if((curGame.balls[ind].xPos >= ARENA_MAX_X - BALL_SIZE - 4) || (curGame.balls[ind].xPos <= ARENA_MIN_X + BALL_SIZE + 4)){ // subtracted 9 and added 9 from actual edges to prevent eroding wall effect
			collision = true; //TODO: Handle case where this is within the paddle's range (x = 0 to 4)
			wall = true;
		} //Checks if low enough on y-axis and between paddle left and right bounds to see if a collision occurs
		else if((curGame.balls[ind].yPos >= 232)&&(curGame.balls[ind].xPos > curGame.players[0].currentCenter - PADDLE_LEN_D2 - 2)&&(curGame.balls[ind].xPos < curGame.players[0].currentCenter + PADDLE_LEN_D2 + 2)){
			collision = true;
			paddle = true;
		}
		else if((curGame.balls[ind].yPos <= 9)&&(curGame.balls[ind].xPos > curGame.players[1].currentCenter - PADDLE_LEN_D2 - 2)&&(curGame.balls[ind].xPos < curGame.players[1].currentCenter + PADDLE_LEN_D2 + 2)){
		    collision = true;
		    paddle = true;
		}

		if(collision){
			 	if(wall){
			 		//If wall is hit, maintain vertical velocity, but reflect horizontally
			 		curGame.balls[ind].xVel = curGame.balls[ind].xVel * -1;
			 	}
			 	//Can be both paddle and wall
			 	if(paddle){
			 	    //Check which paddle it hits
			 	    if(curGame.balls[ind].yPos > MAX_SCREEN_Y - PADDLE_WID - BALL_SIZE - 7){
			 	        //Hit bottom paddle
			 	        curGame.balls[ind].color = LCD_RED;
			 	    }
			 	    else if(curGame.balls[ind].yPos < PADDLE_WID + BALL_SIZE + 7){
			 	        //Collided with top paddle
			 	        curGame.balls[ind].color = LCD_BLUE;
			 	    }
			 	    //Left Side
			 		if(curGame.balls[ind].xPos < curGame.players[0].currentCenter - PADDLE_LEN_D2 + 16 ){
			 		    curGame.balls[ind].yVel = curGame.balls[ind].yVel * -1;
			 		    if(curGame.balls[ind].xVel > 1){
			 		        //Make ball go left
			 		       curGame.balls[ind].xVel = curGame.balls[ind].xVel * -1;
			 		    }
			 		}
			 		//Right side
			 		else if(curGame.balls[ind].xPos > curGame.players[0].currentCenter + PADDLE_LEN_D2 - 16){
			 			curGame.balls[ind].yVel = curGame.balls[ind].yVel * -1;
			 			if(curGame.balls[ind].xVel < 1){
			 			   curGame.balls[ind].xVel = curGame.balls[ind].xVel * -1;
			 			}
			 		}
			 		//Middle
			 		else{ //This is the center of the paddle's area, a space of 24
			 			curGame.balls[ind].yVel = curGame.balls[ind].yVel * -1;
			 		}
			 	}
		}

	    //If ball has passed boundary, adjust score
		bool passedBoundary = false;
		if((curGame.balls[ind].yPos < ARENA_MIN_Y) || (curGame.balls[ind].yPos > ARENA_MAX_X)){
		    //Has passed boundary and will kill itself
		    if(curGame.balls[ind].color == LCD_RED){
		        //Red Ball Scored
		        curGame.numberOfBalls--;
		        curGame.LEDScores[0] += 1;
		        passedBoundary = true;
		    }
		    else if(curGame.balls[ind].color == LCD_BLUE){
		        //Blue Ball Scored
                curGame.numberOfBalls--;
		        curGame.LEDScores[1] += 1;
		        passedBoundary = true;
		    }
		    else{
		        //White Ball fell through- No one scores but ball still is killed
                curGame.numberOfBalls--;
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
                G8RTOS_AddThread(EndOfGameHost, 1, "EndOfGame");
		    }
		    else{
                //Kill this MoveBall
                curGame.balls[ind].alive = false;
                G8RTOS_KillSelf();
		    }
		}
		else{
	        //Save position as the now previous position
		    //prevBallLocs[ind].CenterX = curGame.balls[ind].xPos;
	        //prevBallLocs[ind].CenterY = curGame.balls[ind].yPos;

	        //Update current location of the ball
	        curGame.balls[ind].xPos = curGame.balls[ind].xPos + curGame.balls[ind].xVel;
	        curGame.balls[ind].yPos = curGame.balls[ind].yPos + curGame.balls[ind].yVel;

	        //If went beyond boundary, adjust so it is on boundary
	        //I did this to try to stop the balls from eroding walls
	        if(curGame.balls[ind].xPos > ARENA_MAX_X - BALL_SIZE){
	            curGame.balls[ind].xPos = ARENA_MAX_X - BALL_SIZE;
	        }
	        else if(curGame.balls[ind].xPos < ARENA_MIN_X + BALL_SIZE){
	            curGame.balls[ind].xPos = ARENA_MAX_X + BALL_SIZE;
	        }
		}
		//G8RTOS_SignalSemaphore(&USING_WIFI);
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

    //Wait for semaphores to be available so that threads using them are not killed yet
    G8RTOS_WaitSemaphore(&USING_WIFI);
    G8RTOS_WaitSemaphore(&USING_LED_I2C);
    G8RTOS_WaitSemaphore(&USING_SPI);

    //Now has both semaphores, kill all other threads
    G8RTOS_KillAllOthers();



    //Clear screen with winner's color and print message
    if(curGame.LEDScores[0] > curGame.LEDScores[1]){
        //Player 0 won, make screen their color
        //Increment overall scores
        curGame.overallScores[0] += 1;
        LCD_Clear(curGame.players[0].color);
        LCD_Text(95, 75, "Host Press Button", curGame.players[1].color);
    }
    else if(curGame.LEDScores[0] < curGame.LEDScores[1]){
        //Player 1 won, make screen their color
        //Increment overall scores
        curGame.overallScores[1] += 1;
        LCD_Clear(curGame.players[1].color);
        LCD_Text(95, 75, "Host Press Button", curGame.players[0].color);
    }

    //4.5
    readyForGame = false;
    G8RTOS_AddAPeriodicEvent(HOST_TAP, 4, PORT4_IRQn);
    while(!readyForGame){
        //Send data showing game is over
        SendData((uint8_t*)&curGame, clientToHostInfo.IP_address, sizeof(curGame));
    }


    //When ready, notify client, reinitialize game, add threads back, kill self
    //Notify client
    SendData((uint8_t*)&curGame, clientToHostInfo.IP_address, sizeof(curGame));

    //Reinitialize game with new scores
    InitBoardState();

    /* Add these threads. (Need better priority definitions) */
    G8RTOS_AddThread(GenerateBall, 2, "GenerateBall");
    G8RTOS_AddThread(ReceiveDataFromClient, 2, "ReceiveDataFromClient");
    G8RTOS_AddThread(DrawObjects, 5, "DrawObjects");
    G8RTOS_AddThread(ReadJoystickHost, 5, "ReadJoystickHost");
    G8RTOS_AddThread(SendDataToClient, 5, "SendDataToClient");
    G8RTOS_AddThread(MoveLEDs, 250, "MoveLEDs"); //lower priority
    G8RTOS_AddThread(IdleThread, 254, "Idle");


    //Reinitialize semaphores
    G8RTOS_InitSemaphore(&USING_LED_I2C, 1);
    G8RTOS_InitSemaphore(&USING_SPI, 1);
    G8RTOS_InitSemaphore(&USING_WIFI, 1);


    G8RTOS_KillSelf();
    DelayMs(1);

}

void HOST_TAP(){
    readyForGame = true;
    P4->IFG &= ~BIT5;       //May not need
}

inline void resetGameExScores(){
    //G8RTOS_WaitSemaphore(&USING_WIFI);
    //Make LED scores 0
    curGame.LEDScores[0] = 0;
    curGame.LEDScores[1] = 0;

    //Clear each ball
    for(int i = 0; i < MAX_NUM_OF_BALLS; i++){
        curGame.balls[i].alive = false;
        curGame.balls[i].color = LCD_WHITE;
        curGame.balls[i].newBall = true;
    }

    curGame.winner = false;
    curGame.gameDone = false;
    curGame.numberOfBalls = 0;

    curGame.players[0].color = PLAYER_RED;
    curGame.players[1].color = PLAYER_BLUE;

    curGame.players[0].position = BOTTOM;
    curGame.players[1].position = TOP;

    curGame.players[0].currentCenter = PADDLE_X_CENTER;
    curGame.players[1].currentCenter = PADDLE_X_CENTER;

    prevHostLoc.Center = PADDLE_X_CENTER;
    prevClientLoc.Center = PADDLE_X_CENTER;
    //G8RTOS_SignalSemaphore(&USING_WIFI);
    //curGame.overallScores[]   Scores not reset between games
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
	    for(int i = 0; i < MAX_NUM_OF_BALLS; i++){
	        //Check if ball is alive
	        if(curGame.balls[i].alive){
	            if(curGame.balls[i].newBall){
	                //If a new ball, paint its initial location
	                //G8RTOS_WaitSemaphore(&USING_WIFI);
	                int16_t xCoord = curGame.balls[i].xPos;
	                int16_t yCoord = curGame.balls[i].yPos;
	                //G8RTOS_SignalSemaphore(&USING_WIFI);
	                G8RTOS_WaitSemaphore(&USING_SPI);
	                LCD_DrawRectangle(xCoord - BALL_SIZE_D2, xCoord + BALL_SIZE_D2, yCoord - BALL_SIZE_D2, yCoord + BALL_SIZE_D2, LCD_WHITE);
	                G8RTOS_SignalSemaphore(&USING_SPI);

	                //G8RTOS_WaitSemaphore(&USING_WIFI);
	                //Not new anymore
	                curGame.balls[i].newBall = false;
	                curGame.balls[i].newBall = false;
	                //G8RTOS_SignalSemaphore(&USING_WIFI);
	            }
	            else{
	                //If not a new ball, update its location
	                //G8RTOS_WaitSemaphore(&USING_WIFI);
	                UpdateBallOnScreen(&prevBallLocs[i], &curGame.balls[i], curGame.balls[i].color);
                    for(int i = 0; i < MAX_NUM_OF_BALLS; i++){
                        //Save ball locations as previous balls
                        prevBallLocs[i].CenterX = curGame.balls[i].xPos;
                        prevBallLocs[i].CenterY = curGame.balls[i].yPos;
                    }
                    //G8RTOS_SignalSemaphore(&USING_WIFI);
	            }
	        }
	    }
	    //Update Players
	    //Update Location of Each Player
	    //Host
	    UpdatePlayerOnScreen(&prevHostLoc, &curGame.players[0]);
	    //Update Client Location
	    UpdatePlayerOnScreen(&prevClientLoc, &curGame.players[1]);

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
		• Responsible for updating the LED array with current scores
		*/

	    //Print both scores for debugging purposes- uncomment if/else for normal function
	    //if(!isClient){
	        //Update Host (Red/Bottom)
	        valToWrite = numToLitLEDS(curGame.LEDScores[0]);
	        G8RTOS_WaitSemaphore(&USING_LED_I2C);
	        LP3943_DataDisplay(RED, ON, valToWrite);
	        G8RTOS_SignalSemaphore(&USING_LED_I2C);
	    //}
	    //else{
	        //Update Client (Blue/Top)
            valToWrite = numToLitLEDS(curGame.LEDScores[1]);
            G8RTOS_WaitSemaphore(&USING_LED_I2C);
            LP3943_DataDisplay(BLUE, ON, valToWrite);
            G8RTOS_SignalSemaphore(&USING_LED_I2C);

	    //}

	}
}

/*
 * Takes in a number and returns the number to light up that many LEDs
 * (Left to Right)
 */
inline uint16_t numToLitLEDS(uint8_t playerScore){
    uint16_t toSend = 0;
    //Convert score to set that number of LEDs
    for(int i = 0; i < playerScore; i++){
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

		P4->DIR &= ~(BIT4 | BIT5);
	    P4->REN |= BIT4|BIT5; //Pull-up resistor
	    P4->OUT |= BIT4|BIT5; //Set resistor to pull-up
	    while(1)
	    {
	        if(!(P4->IN & BIT4))
	        {
	            DelayMs(10);
	            if(!(P4->IN & BIT4))
	            {
	                return Client;
	            }
	        }
	        if(!(P4->IN & BIT5))
	        {
	            DelayMs(10);
	            if(!(P4->IN & BIT5))
	            {
	                readyForGame = true;
	                return Host;
	            }
	        }
	    }
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
    G8RTOS_WaitSemaphore(&USING_SPI);
    int16_t pixelsMoved = outPlayer->currentCenter - prevPlayerIn->Center;
    //In current-previous, positive values mean moved rightward
    //FIXME Direction Changes sometimes leaves a small black line behind inside paddle
    if(outPlayer->position == TOP){
        if(pixelsMoved > 0){
            //Moved rightward
            //Paint background on left side
            //G8RTOS_WaitSemaphore(&USING_SPI);
            LCD_DrawRectangle(prevPlayerIn->Center - PADDLE_LEN_D2 - PRINT_OFFSET, outPlayer->currentCenter - PADDLE_LEN_D2, TOP_PADDLE_EDGE - PADDLE_WID, TOP_PADDLE_EDGE, BACK_COLOR);
            //G8RTOS_SignalSemaphore(&USING_SPI);
            //Paint color on right side
            //G8RTOS_WaitSemaphore(&USING_SPI);
            LCD_DrawRectangle(prevPlayerIn->Center + PADDLE_LEN_D2 - PRINT_OFFSET, outPlayer->currentCenter + PADDLE_LEN_D2, TOP_PADDLE_EDGE - PADDLE_WID, TOP_PADDLE_EDGE, outPlayer->color);
            //G8RTOS_SignalSemaphore(&USING_SPI);

            //If was close to wall, repaint white wall
            if(prevPlayerIn->Center <= ARENA_MIN_X + PADDLE_LEN_D2 + PRINT_OFFSET){
                //G8RTOS_WaitSemaphore(&USING_SPI);
                LCD_DrawRectangle(ARENA_MIN_X - PRINT_OFFSET, ARENA_MIN_X, ARENA_MIN_Y, ARENA_MIN_Y + PADDLE_WID, LCD_WHITE);
                //G8RTOS_SignalSemaphore(&USING_SPI);
            }

            //If was close to wall, repaint white wall
            if(prevPlayerIn->Center <= ARENA_MIN_X + PADDLE_LEN_D2 + PRINT_OFFSET){
                //G8RTOS_WaitSemaphore(&USING_SPI);
                LCD_DrawRectangle(ARENA_MIN_X - PRINT_OFFSET, ARENA_MIN_X,  ARENA_MIN_Y, ARENA_MIN_Y + PADDLE_WID + 1, LCD_WHITE);
                //G8RTOS_SignalSemaphore(&USING_SPI);
            }
        }
        else if(pixelsMoved < 0){
            //Moved leftward
            //Paint black on right
            //G8RTOS_WaitSemaphore(&USING_SPI);
            LCD_DrawRectangle(outPlayer->currentCenter + PADDLE_LEN_D2, prevPlayerIn->Center + PADDLE_LEN_D2 + PRINT_OFFSET, TOP_PADDLE_EDGE - PADDLE_WID, TOP_PADDLE_EDGE, BACK_COLOR);
            //G8RTOS_SignalSemaphore(&USING_SPI);
            //Paint color on left side
            //G8RTOS_WaitSemaphore(&USING_SPI);
            LCD_DrawRectangle(outPlayer->currentCenter - PADDLE_LEN_D2 + PRINT_OFFSET, prevPlayerIn->Center - PADDLE_LEN_D2, TOP_PADDLE_EDGE - PADDLE_WID, TOP_PADDLE_EDGE, outPlayer->color);
            //G8RTOS_SignalSemaphore(&USING_SPI);

            //If was close to wall, repaint white wall
            if(prevPlayerIn->Center >= ARENA_MAX_X - PADDLE_LEN_D2 - PRINT_OFFSET){
                //G8RTOS_WaitSemaphore(&USING_SPI);
                LCD_DrawRectangle(ARENA_MAX_X, ARENA_MAX_X + PRINT_OFFSET, ARENA_MIN_Y, ARENA_MIN_Y + PADDLE_WID + 1, LCD_WHITE);
                //G8RTOS_SignalSemaphore(&USING_SPI);
            }

        }
    }
    else if(outPlayer->position == BOTTOM){
        if(pixelsMoved > 0){
            //Moved rightward
            //Paint background on left side
            //G8RTOS_WaitSemaphore(&USING_SPI);
            LCD_DrawRectangle(prevPlayerIn->Center - PADDLE_LEN_D2 - PRINT_OFFSET, outPlayer->currentCenter - PADDLE_LEN_D2, BOTTOM_PADDLE_EDGE-1, BOTTOM_PADDLE_EDGE + PADDLE_WID, BACK_COLOR);
            //G8RTOS_SignalSemaphore(&USING_SPI);
            //Paint color on right side
            //G8RTOS_WaitSemaphore(&USING_SPI);
            LCD_DrawRectangle(prevPlayerIn->Center + PADDLE_LEN_D2 - PRINT_OFFSET, outPlayer->currentCenter + PADDLE_LEN_D2, BOTTOM_PADDLE_EDGE-1, BOTTOM_PADDLE_EDGE + PADDLE_WID, outPlayer->color);
            //G8RTOS_SignalSemaphore(&USING_SPI);

            //If was close to wall, repaint white wall
            if(prevPlayerIn->Center <= ARENA_MIN_X + PADDLE_LEN_D2 + PRINT_OFFSET){
                //G8RTOS_WaitSemaphore(&USING_SPI);
                LCD_DrawRectangle(ARENA_MIN_X - PRINT_OFFSET, ARENA_MIN_X, ARENA_MAX_Y - PADDLE_WID - 1, ARENA_MAX_Y, LCD_WHITE);
                //G8RTOS_SignalSemaphore(&USING_SPI);
            }
        }
        else if(pixelsMoved < 0){
            //Moved leftward
            //Paint black on right
            //G8RTOS_WaitSemaphore(&USING_SPI);
            LCD_DrawRectangle(outPlayer->currentCenter + PADDLE_LEN_D2, prevPlayerIn->Center + PADDLE_LEN_D2 + PRINT_OFFSET, BOTTOM_PADDLE_EDGE-1, BOTTOM_PADDLE_EDGE + PADDLE_WID, BACK_COLOR);
            //G8RTOS_SignalSemaphore(&USING_SPI);
            //Paint color on left side
            //G8RTOS_WaitSemaphore(&USING_SPI);
            LCD_DrawRectangle(outPlayer->currentCenter - PADDLE_LEN_D2, prevPlayerIn->Center + PADDLE_LEN_D2 - PRINT_OFFSET, BOTTOM_PADDLE_EDGE-1, BOTTOM_PADDLE_EDGE + PADDLE_WID, outPlayer->color);
            //G8RTOS_SignalSemaphore(&USING_SPI);

            //If was close to wall, repaint white wall
            if(prevPlayerIn->Center >= ARENA_MAX_X - PADDLE_LEN_D2 - PRINT_OFFSET){
                //G8RTOS_WaitSemaphore(&USING_SPI);
                LCD_DrawRectangle(ARENA_MAX_X, ARENA_MAX_X + PRINT_OFFSET, ARENA_MAX_Y - PADDLE_WID - 1, ARENA_MAX_Y, LCD_WHITE);
                //G8RTOS_SignalSemaphore(&USING_SPI);
            }

        }
    }
    G8RTOS_SignalSemaphore(&USING_SPI);


}

/*
 * Function updates ball position on screen
 *
 * Changed the Ball_t parameter to balls_t that we made
 */
inline void UpdateBallOnScreen(PrevBall_t * previousBall, balls_t * currentBall, uint16_t outColor){
    //If not new, look for previous location
    G8RTOS_WaitSemaphore(&USING_SPI);
    int16_t prevX = previousBall->CenterX;
    int16_t prevY = previousBall->CenterY;

    //Paint background color over where it was
    //Check if it is going to erase in a bad place
    if(prevX < ARENA_MIN_X || prevX > ARENA_MAX_X){

    }
    else{
        //G8RTOS_WaitSemaphore(&USING_SPI);
        LCD_DrawRectangle(prevX- BALL_SIZE_D2, prevX + BALL_SIZE_D2, prevY - BALL_SIZE_D2, prevY + BALL_SIZE_D2, BACK_COLOR);
        //G8RTOS_SignalSemaphore(&USING_SPI);
    }


    //Paint where it is now
    //Check if in boundary
    if(currentBall->yPos > MAX_SCREEN_Y || (currentBall->yPos < MIN_SCREEN_Y)){
        //Out of bounds, Do not draw
    }
    else{
        //G8RTOS_WaitSemaphore(&USING_SPI);
        LCD_DrawRectangle(currentBall->xPos - BALL_SIZE_D2, currentBall-> xPos + BALL_SIZE_D2, currentBall->yPos - BALL_SIZE_D2, currentBall->yPos + BALL_SIZE_D2, outColor);
        //G8RTOS_SignalSemaphore(&USING_SPI);
    }
    G8RTOS_SignalSemaphore(&USING_SPI);
}

/*
 * Initializes and prints initial game state
 */
inline void InitBoardState(){
    /*Clear screen*/
    G8RTOS_WaitSemaphore(&USING_SPI);
    LCD_Clear(BACK_COLOR);
    G8RTOS_SignalSemaphore(&USING_SPI);


	/* White lines to define arena size */
	G8RTOS_WaitSemaphore(&USING_SPI);
	LCD_DrawRectangle(1, 40, 1, 239, LCD_WHITE);
	G8RTOS_SignalSemaphore(&USING_SPI);

	G8RTOS_WaitSemaphore(&USING_SPI);
	LCD_DrawRectangle(280, 319, 1, 239, LCD_WHITE);
	G8RTOS_SignalSemaphore(&USING_SPI);

	//Write Player Scores
	uint8_t scores[3];
	setScoreString(scores, 1);
	G8RTOS_WaitSemaphore(&USING_SPI);
	LCD_Text(5, 5, scores, PLAYER_BLUE);
	G8RTOS_SignalSemaphore(&USING_SPI);

	setScoreString(scores, 0);
    G8RTOS_WaitSemaphore(&USING_SPI);
    LCD_Text(5, MAX_SCREEN_Y - 20, scores, PLAYER_RED);
    G8RTOS_SignalSemaphore(&USING_SPI);

	/* The initial paddle */
    /* Set Center of player paddle */
    resetGameExScores();

    DrawPlayer(&curGame.players[0]);
    DrawPlayer(&curGame.players[1]);

}

/*
 * Takes in array and fills it with the characters of the entered player's score
 */
inline void setScoreString(uint8_t scoreArray[3], uint16_t playerIndex){
    //Get player's score
    uint8_t curScore = curGame.overallScores[playerIndex];

    //Set array with characters that are that score
    scoreArray[0] = curScore/10 + 0x30;
    scoreArray[1] = curScore%10 + 0x30;
    scoreArray[2] = 0x00;   //Terminating character
}
/*********************************************** Public Functions *********************************************************************/
