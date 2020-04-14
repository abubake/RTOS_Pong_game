#include "Game.h"
#include "Joystick.h"
#include "G8RTOS.h"
#include <stdlib.h>
#include "LCD.h"

SpecificPlayerInfo_t clientToHostInfo;

/* Joystick Info */
int16_t X_coord;
int16_t Y_coord;
int16_t host_X_coord; // Since I have two joystick read functions, shouldn't be an issue since one will be host and
						//other board won't
int16_t host_Y_coord;

/*Ball related Info */
balls_t myBalls[20];
int NumBalls = 20;
int ballNumber;
int curBalls = 0;


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
	//G8RTOS_AddThread(DrawObjects, 200, "DrawObjects");
	//G8RTOS_AddThread(ReadJoystickHost, 200, "ReadJoystickHost");
	//G8RTOS_AddThread(SendDataToClient, 200, "SendDataToClient");
	//G8RTOS_AddThread(ReceiveDataFromClient, 200, "ReceiveDataFromClient");
	//G8RTOS_AddThread(MoveLEDs, 250, "MoveLEDs"); //lower priority
	G8RTOS_AddThread(IdleThread, 200, "Idle");

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
	    if(curBalls < NumBalls){
	        curBalls++;
	        G8RTOS_AddThread(MoveBall, 5, "MoveBall");

	    }
	    //TODO Adjust scalar for sleep based on experiments to see what makes the game fun
	    sleep(curBalls*5000);
	}
}

/*
 * Thread to read host's joystick
 */
void ReadJoystickHost(){
	while(1){
		/*
		• You can read the joystick ADC values by calling GetJoystickCoordinates
		• You’ll need to add a bias to the values (found experimentally) since every joystick is offset by some small amount displacement and noise
		• Change Self.displacement accordingly (you can experiment with how much you want to scale the ADC value)
		• Sleep for 10ms
		*/
		GetJoystickCoordinates(&host_X_coord, &host_Y_coord); //must wait for its semaphore!
		sleep(10); // makes game for fair

		/*
		• Then add the displacement to the bottom player in the list of players (general list that’s sent to the client and used for drawing) i.e. players[0].position += self.displacement
		• By sleeping before updating the bottom player’s position, it makes the game more fair between client and host
		*/

	}
}

/*
 * Thread to move a single ball
 */
void MoveBall(){
    /*
    • Go through array of balls and find one that’s not alive (DONE)
    • Once found, initialize random position and X and Y velocities, as well as color and alive attributes (DONE)
    • Checking for collision given the current center and the velocity
    • If collision occurs, adjust velocity and color accordingly
    • If the ball passes the boundary edge, adjust score, account for the game possibly ending, and kill self
    • Otherwise, just move the ball in its current direction according to its velocity
    • Sleep for 35ms
    */

    //Initialize ball if it was newly made
    for (int i = 0; i < NumBalls; i++){
        if(myBalls[i].alive == false){ // Searching for the first dead ball
            /* Gives random position */
            myBalls[i].xPos = (rand() % 10) + 100;
            myBalls[i].yPos = (rand() % 10) + 100;

            /* Getting a random speed */
            myBalls[i].speed = (rand() % 2) + 1;

            //Ball is initially white
            myBalls[i].color = LCD_WHITE;
            myBalls[i].alive = true;
            break;
        }
    }
	while(1){
	    //Check for collision

	    //If collision occurred, change color and velocity

	    //If ball has passed boundary, adjust score

	    //If score happened, adjust score and killself
	    //Check if game has ended

	    //If not killed, move ball to position according to its velocity
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
 */
void UpdateBallOnScreen(PrevBall_t * previousBall, Ball_t * currentBall, uint16_t outColor){

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

}

/*********************************************** Public Functions *********************************************************************/
