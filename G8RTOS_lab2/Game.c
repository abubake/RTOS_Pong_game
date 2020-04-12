#include "game.h"
#include "Joystick.h"
#include "G8RTOS.h"

SpecificPlayerInfo_t clientToHostInfo;

/* Joystick Info */
int16_t X_coord;
int16_t Y_coord;

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
	/*
	• Only thread created before launching the OS
	• Initializes the players
	• Establish connection with client (use an LED on the Launchpad to indicate Wi-Fi connection)
	o Should be trying to receive a packet from the client
	o Should acknowledge client once client has joined
	• Initialize the board (draw arena, players, and scores)
	• Add the following threads:
	o GenerateBall
	o DrawObjects
	o ReadJoystickHost
	o SendDataToClient
	o ReceiveDataFromClient
	o MoveLEDs (lower priority)
	o Idle
	• Kill self
	*/

}

/*
 * Thread that sends game state to client
 */
void SendDataToClient(){
	while(1){

	}
}

/*
 * Thread that receives UDP packets from client
 */
void ReceiveDataFromClient(){
	while(1){

	}
}

/*
 * Generate Ball thread
 */
void GenerateBall(){
	while(1){

	}
}

/*
 * Thread to read host's joystick
 */
void ReadJoystickHost(){
	while(1){

	}
}

/*
 * Thread to move a single ball
 */
void MoveBall(){
	while(1){

	}
}

/*
 * End of game for the host
 */
void EndOfGameHost(){
	while(1){

	}
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

	}
}

/*
 * Thread to update LEDs based on score
 */
void MoveLEDs(){
	while(1){

	}
}

/*********************************************** Common Threads *********************************************************************/
