#include "game.h"
#include "G8RTOS.h"

SpecificPlayerInfo_t clientToHostInfo;

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
	while(1); // feeling cute, might delete later
	G8RTOS_KillSelf();
}

/*
 * Thread that receives game state packets from host
 */
void ReceiveDataFromHost(){
	while(1){

	}
}

/*
 * Thread that sends UDP packets to host
 */
void SendDataToHost(){
	while(1){
		//send data to host and sleep

		sleep(2);
	}
}

/*
 * Thread to read client's joystick
 */
void ReadJoystickClient(){
	while(1){

	}
}

/*
 * End of game for the client
 */
void EndOfGameClient(){
	while(1){

	}
}

/*********************************************** Client Threads *********************************************************************/


/*********************************************** Host Threads *********************************************************************/
/*
 * Thread for the host to create a game
 */
void CreateGame(){
	while(1){

	}
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
