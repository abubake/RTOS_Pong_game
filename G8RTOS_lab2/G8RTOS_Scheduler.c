/*
 * G8RTOS_Scheduler.c
 */

/*********************************************** Dependencies and Externs *************************************************************/

#include <stdint.h>
#include "msp.h"
#include "G8RTOS_Scheduler.h"
#include "G8RTOS_Structures.h"
#include <DriverLib.h>
#include "BSP.h"
#include "cc3100_usage.h"

/*
 * G8RTOS_Start exists in asm
 */
extern void G8RTOS_Start();

/* System Core Clock From system_msp432p401r.c */
extern uint32_t SystemCoreClock;

/*
 * Pointer to the currently running Thread Control Block
 */
extern tcb_t * CurrentlyRunningThread;


/*********************************************** Dependencies and Externs *************************************************************/


/*********************************************** Defines ******************************************************************************/

/* Status Register with the Thumb-bit Set */
#define THUMBBIT 0x01000000

/* Defines for requesting PendSV from SysTick */
#define TICKS_PER_QUANTUM 1
#define ICSR (*((volatile unsigned int*) (0xe000ed04)))
#define ICSR_PENDSVSET (1 << 28)

//#define SHPR3 0xe000ed20
#define PRI_14 (*((volatile unsigned int*) (0xe000ed22))) //PendSV
#define PRI_15 (*((volatile unsigned int*) (0xe000ed23)))

/* For self implementing Systick */
#define SYST_CSR (*((volatile unsigned int*)(0xe000e010)))
#define SYST_RVR (*((volatile unsigned int*)(0xe000e014)))
#define SYST_CALIB (*((volatile unsigned int*)(0xe000e01c)))
#define SYST_ENABLE (1 << 0)
#define SYST_TICKINT (1 << 1)
//#define SYST_RELOAD ((1 << 24) – 1)


/**********************************************************************************************************************************/


/*********************************************** Data Structures Used *****************************************************************/

/* Thread Control Blocks
 *	- An array of thread control blocks to hold pertinent information for each thread
 */
static tcb_t threadControlBlocks[MAX_THREADS];

/* Thread Stacks
 *	- An array of arrays that will act as individual stacks for each thread
 */
static int32_t threadStacks[MAX_THREADS][STACKSIZE];

/* Periodic Event Threads
 * - An array of periodic events to hold pertinent information for each thread */
static ptcb_t Pthread[MAXPTHREADS];


/*********************************************** Data Structures Used *****************************************************************/


/*********************************************** Private Variables ********************************************************************/

/*
 * Current Number of Threads currently in the scheduler
 */
static uint32_t NumberOfThreads;

/*  Current Number of Periodic Threads currently in the scheduler */
static uint32_t NumberOfPthreads;

/*********************************************** Private Variables ********************************************************************/

/* holds the number of ticks in the quantum (100 total, incremented in Systick each call) */
static volatile unsigned int ticks_count;

/*********************************************** Private Functions ********************************************************************/

/*
 * Initializes the Systick and Systick Interrupt
 * The Systick interrupt will be responsible for starting a context switch between threads
 * Param "numCycles": Number of cycles for each systick interrupt
 */
static void InitSysTick(uint32_t numCycles)
{
	//uint32_t reload = (SYST_RELOAD - 1);
    SysTick_Config(numCycles); // ticks to count
    //SYST_RVR = SYST_CALIB & reload;
    //SYST_CSR = SYST_ENABLE | SYST_TICKINT;
}

/*
 * Chooses the next thread to run.
 * Lab 2 Scheduling Algorithm:
 * 	- Simple Round Robin: Choose the next running thread by selecting the currently running thread's next pointer
 */
void G8RTOS_Scheduler()
{
    uint8_t currentMaxPriority = 255;
    tcb_t *tempNextThread = CurrentlyRunningThread->Next;

    //Iterate through the other threads to check for higher priorities
    for(uint16_t i = 0; i < NumberOfThreads; i++){
        if((tempNextThread->blocked == 0) && (tempNextThread->asleep == false)){
            //If not blocked and not asleep, check if has higher priority
            //If so, set CurrentlyRunningThread
            if(tempNextThread->priority <= currentMaxPriority){
                currentMaxPriority = tempNextThread->priority;
                CurrentlyRunningThread =  tempNextThread;
            }
        }
        //Update temp to check all threads
        tempNextThread = tempNextThread->Previous;
    }
}


/*
 * SysTick Handler
 * Currently the Systick Handler will only increment the system time
 * and set the PendSV flag to start the scheduler
 *
 * In the future, this function will also be responsible for sleeping threads and periodic threads
 */
void SysTick_Handler()
{
	SystemTime++;
	uint16_t i = 0;
	ptcb_t *periodicPtr = &Pthread[0];
	while(i < NumberOfPthreads){
		if (periodicPtr->executionTime == SystemTime){
			periodicPtr->executionTime = periodicPtr->Period + SystemTime;
			periodicPtr->Handler();
			periodicPtr = periodicPtr->Next;
			i++; // we move on to the next pthread
		}
		else {
			periodicPtr = periodicPtr->Next;
			i++;
		}
	}

	tcb_t *threadPtr = CurrentlyRunningThread;
	for(uint32_t i=0; i<NumberOfThreads; i++){
		if(threadPtr->sleepCount <= SystemTime){ //Was >=
			threadPtr->asleep = false;
			threadPtr->sleepCount = 0;
		}
		threadPtr = threadPtr->Next;
	}

	StartContextSwitch();
}

/*********************************************** Private Functions ********************************************************************/


/*********************************************** Public Variables *********************************************************************/

/* Holds the current time for the whole System */
uint32_t SystemTime;

uint8_t deadFlag = 0;

/****************************************************************************************************************************************/


/*********************************************** Public Functions *********************************************************************/

/*
 * Sets variables to an initial state (system time and number of threads)
 * Enables board for highest speed clock and disables watchdog
 */
void G8RTOS_Init()
{
	/* Turns off the watchdog timer with HOLD and sets a required password to avoid entering reset  */
	WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;

    //system time to 0 (holds current time for the whole system)
    SystemTime = 0;
    //# of threads = 0
    NumberOfThreads = 0;


    uint32_t newVTORTable = 0x20000000;
    memcpy((uint32_t *)newVTORTable, (uint32_t *)SCB->VTOR, 57*4); // 57 interrupt vectors to copy
    SCB->VTOR = newVTORTable;


    //init all hardware on the board
    //BSP_InitBoard();
}

/*
 * Starts G8RTOS Scheduler
 * 	- Initializes the Systick, taking into account that we are operating at max clock frequency
 * 	- Sets the priority for the PendSV and Systick exceptions, with PendSV as the lowest (7) and Systick higher (5)
 * 	- Sets Context to first thread
 * 	- Calls the start assembly subroutine that loads the first thread's context
 *  - Returns: Error Code for starting scheduler. This will only return if the scheduler fails */
int G8RTOS_Launch()
{
	InitSysTick(48000); //initializes the SysTick
	NVIC_SetPriority(PendSV_IRQn ,OSINT_PRIORITY);
	NVIC_SetPriority(SysTick_IRQn ,OSTICK_PRIORITY);

	CurrentlyRunningThread = &threadControlBlocks[0];

	tcb_t * tempCurrentThread = CurrentlyRunningThread;
	tcb_t * tempHolderThread = tempCurrentThread->Next;

	for (uint32_t i = 0; i < NumberOfThreads; i++){

		tempHolderThread = tempCurrentThread->Next; //this is always one ahead of tempCurrentThread
		tempCurrentThread = tempCurrentThread->Next;

		if (tempHolderThread->priority < tempCurrentThread->priority){ //we want whatever has the lowest numeric priority
			CurrentlyRunningThread = tempHolderThread;
		}

	}


	G8RTOS_Start();

return -1;
}


/*
 * Adds threads to G8RTOS Scheduler
 * 	- Checks if there are still available threads to insert to scheduler
 * 	- Initializes the thread control block for the provided thread
 * 	- Initializes the stack for the provided thread to hold a "fake context"
 * 	- Sets stack tcb stack pointer to top of thread stack
 * 	- Sets up the next and previous tcb pointers in a round robin fashion
 * Param "threadToAdd": Void-Void Function to add as preemptable main thread
 * Returns: Error code for adding threads */
sched_ErrCode_t G8RTOS_AddThread(void (*threadToAdd)(void), uint16_t priority, char * name)
{
	static uint16_t IDCounter = 0;
	uint32_t x = StartCriticalSection();

	if (NumberOfThreads == MAX_THREADS){
		EndCriticalSection(x);
		return THREAD_LIMIT_REACHED;
	}

	NumberOfThreads++; // preinc
	//Find first dead
	for(uint32_t addedThread = 0; addedThread < NumberOfThreads; addedThread++){
	    //Check if dead
		if(threadControlBlocks[addedThread].isAlive == false){
		    //Check if only living thread
			if(NumberOfThreads == 1){
			    //If it is the only living thread, it points to itself
				threadControlBlocks[addedThread].Next = &threadControlBlocks[addedThread];
				threadControlBlocks[addedThread].Previous = &threadControlBlocks[addedThread];

				threadControlBlocks[addedThread].isAlive = true;
				threadControlBlocks[addedThread].priority = priority;
				threadControlBlocks[addedThread].threadID = ((uint32_t) IDCounter++ << 16) | addedThread;
				SetInitialStack(addedThread);
				threadStacks[addedThread][STACKSIZE-2] = (uint32_t)threadToAdd; // PC
				threadControlBlocks[addedThread].blocked = 0;
				(*threadControlBlocks[addedThread].threadName) = *name; //maybe not pointer tho and cast
				EndCriticalSection(x);
				return NO_ERROR; // success
			}
			else{
			    //If not the only living thread, needs to be added to linked list
			    //Find a living thread to link to
			    for(uint16_t i = 0; i < MAX_THREADS; i++){
			        if(threadControlBlocks[i].isAlive){
			            //If found block is alive, link to it as the next
			            threadControlBlocks[addedThread].Next = &threadControlBlocks[i];
			            //Make the previous of this thread the former previous of the next
			            threadControlBlocks[addedThread].Previous = threadControlBlocks[i].Previous;


			            //Make this thread the next of the former next threads previous
			            threadControlBlocks[addedThread].Previous->Next = &threadControlBlocks[addedThread];
			            //Make this thread the new previous of the found one
			            threadControlBlocks[i].Previous = &threadControlBlocks[addedThread];
			            break;
			        }
			    }

				threadControlBlocks[addedThread].priority = priority;
				threadControlBlocks[addedThread].isAlive = true;
				threadControlBlocks[addedThread].threadID = ((uint32_t) IDCounter++ << 16) | addedThread;
				SetInitialStack(addedThread);
				threadStacks[addedThread][STACKSIZE-2] = (uint32_t)threadToAdd; // PC
				threadControlBlocks[addedThread].blocked = 0;
				*(threadControlBlocks[addedThread].threadName) = *name;
				EndCriticalSection(x);
				return NO_ERROR; // success

			}
		}
	}
	//NumberOfThreads--;
	EndCriticalSection(x);
	return THREADS_INCORRECTLY_ALIVE;
}


/**********************SET INITIAL STACK******************************
 * Called by: G8RTOS_AddThread
 * Purpose: To create a stack for each of the threads
 *
 * *******************************************************************/
void SetInitialStack(int i){
    threadControlBlocks[i].Stack_pointer = &threadStacks[i][STACKSIZE-16]; //this points to thread sp
    threadStacks[i][STACKSIZE - 1] = THUMBBIT; //thumb bit
    // 2 IS SKIPPED BECAUSE PC IS THERE, AND IS SET WITH THE TASK FUNCTER IN ADDTHREAD
    // THESE DUMMY VALUES JUST ASSIST WITH DEBUGGING
    threadStacks[i][STACKSIZE - 3] = 0x14141414; //R14
	threadStacks[i][STACKSIZE - 4] = 0x12121212; //R12
	threadStacks[i][STACKSIZE - 5] = 0x03030303; //R3
	threadStacks[i][STACKSIZE - 6] = 0x02020202; //R2
	threadStacks[i][STACKSIZE - 7] = 0x01010101; //R1
	threadStacks[i][STACKSIZE - 8] = 0x00000000; //R0
	threadStacks[i][STACKSIZE - 9] = 0x11111111; //R11
	threadStacks[i][STACKSIZE - 10] = 0x10101010; //R10
	threadStacks[i][STACKSIZE - 11] = 0x09090909; //R9
	threadStacks[i][STACKSIZE - 12] = 0x08080808; //R8
	threadStacks[i][STACKSIZE - 13] = 0x07070707; //R7
	threadStacks[i][STACKSIZE - 14] = 0x06060606; //R6
	threadStacks[i][STACKSIZE - 15] = 0x05050505; //R5
	threadStacks[i][STACKSIZE - 16] = 0x04040404; //R4
}

/**************************************************************************************************************************************/

/* Causes a context switch to occur immediately
 * Parameters: N/A
 * Returns: Void */
void StartContextSwitch(){
	__DSB();
	__ISB();
	ICSR |= ICSR_PENDSVSET;
}

/*
 * Adds periodic threads to G8RTOS Scheduler
 * Function will initialize a periodic event struct to represent event.
 * The struct will be added to a linked list of periodic events
 * Param Pthread To Add: void-void function for P thread handler
 * Param period: period of P thread to add
 * Returns: Error code for adding threads */
int G8RTOS_AddPeriodicEvent(void (*PthreadToAdd)(void), uint32_t period)
{
	uint32_t x;
	x = StartCriticalSection();
    /* Implement this */
	if(NumberOfPthreads == MAXPTHREADS){
		EndCriticalSection(x);
		return -1;
	}
	else{
		if (NumberOfPthreads == 0){

			Pthread[NumberOfPthreads].Next = &Pthread[NumberOfPthreads];
			Pthread[NumberOfPthreads].Previous = &Pthread[NumberOfPthreads];
			Pthread[NumberOfPthreads].Period = period;
			Pthread[NumberOfPthreads].executionTime = period + NumberOfPthreads;
			Pthread[NumberOfPthreads].Handler = PthreadToAdd;
		}
		else{
			Pthread[NumberOfPthreads-1].Next = &Pthread[NumberOfPthreads];
			Pthread[NumberOfPthreads].Previous = &Pthread[NumberOfPthreads-1];

			Pthread[NumberOfPthreads].Next = &Pthread[0];
			Pthread[0].Previous = &Pthread[NumberOfPthreads];

			Pthread[NumberOfPthreads].Handler = PthreadToAdd;
			Pthread[NumberOfPthreads].Period = period;
			Pthread[NumberOfPthreads].executionTime = period + NumberOfPthreads;
		}
		NumberOfPthreads++;
		EndCriticalSection(x);
		return 1;
		}
}

sched_ErrCode_t G8RTOS_AddAPeriodicEvent(void (*AthreadToAdd)(void), uint16_t priority, IRQn_Type IRQn){
	uint32_t x;
	x = StartCriticalSection();
	if((IRQn > PSS_IRQn)&&(IRQn < PORT6_IRQn)){ // Not sure on this; We were told to check the opposite conditions
		if (priority <= 6){
		__NVIC_SetVector(IRQn, (uint32_t)AthreadToAdd); // takes address of interrupt, so this should be fine
		__NVIC_SetPriority(IRQn, priority);
		__NVIC_EnableIRQ(IRQn);
		EndCriticalSection(x);
		return NO_ERROR;
		}
		else{
			EndCriticalSection(x);
			return HWI_PRIORITY_INVALID; // its own special error term
		}
	}
	else {
		EndCriticalSection(x);
		return IRQn_INVALID; // fix to be appr. error term
	}
}

/* Puts the current thread into a sleep state.
 * Inits a sleep count for the current thread, puts the thread to sleep,
 * and starts a context switch to allow other threads to run
 *  param durationMS: Duration of sleep time in ms */
void sleep(uint32_t durationMS)
{
	int32_t x = StartCriticalSection();
	CurrentlyRunningThread->sleepCount = durationMS + SystemTime;
	CurrentlyRunningThread->asleep = true;
	EndCriticalSection(x);
	StartContextSwitch();

}

/* Returns the currently running thread's thread ID */
threadId_t G8RTOS_GetThreadId(){

	return CurrentlyRunningThread->threadID;
}


/* Takes in a thread ID, indicating the thread to kill */
sched_ErrCode_t G8RTOS_KillThread(threadId_t threadId){

	int32_t x = StartCriticalSection();
	bool threadDNE = true;
	tcb_t * deadManThread;

	if (NumberOfThreads == 1){
		EndCriticalSection(x);
		return CANNOT_KILL_LAST_THREAD; // will change error codes later
	}
	else {
		tcb_t * tempThread = CurrentlyRunningThread->Next;

		for(uint32_t i = 0; i < MAX_THREADS; i++){
			if(tempThread->threadID == threadId){
				deadManThread = tempThread;
				threadDNE = false;
				break;
			}
			tempThread = tempThread->Next;
		}
		/* Checking to see if that thread request existed */
		if(threadDNE == true){
			EndCriticalSection(x);
			return THREAD_DOES_NOT_EXIST;
		}
		deadManThread->isAlive = false;

		/* Remove the dead thread/ Ball from the linked list */
		tcb_t * nextThread =  deadManThread->Next;
		tcb_t * prevThread =  deadManThread->Previous;

		prevThread->Next = nextThread;
		nextThread->Previous = prevThread;

		/* See if we need to context switch or not */
		if(deadManThread == CurrentlyRunningThread){
		        NumberOfThreads--;
		        EndCriticalSection(x);
		        StartContextSwitch();
		        return NO_ERROR;
		    }
		    else{
		        NumberOfThreads--;
		        EndCriticalSection(x);
		        return NO_ERROR;
		    }
		}
	}


sched_ErrCode_t G8RTOS_KillSelf(){

	int32_t x = StartCriticalSection();

	if (NumberOfThreads == 1){
		EndCriticalSection(x);
		return CANNOT_KILL_LAST_THREAD;
		}
	else{
	CurrentlyRunningThread->isAlive = false;

	/* Remove the dead thread/ Ball from the linked list */
	CurrentlyRunningThread->Next->Previous = CurrentlyRunningThread->Previous;
	CurrentlyRunningThread->Previous->Next = CurrentlyRunningThread->Next;

	NumberOfThreads--;
	EndCriticalSection(x);
	StartContextSwitch();
	return NO_ERROR;
	}
}

/*
 * G8RTOS_KillAllOthers()
 *   Kills all threads other than the current thread
 */
sched_ErrCode_t G8RTOS_KillAllOthers(){
    //Start critical section so the killing is uninterrupted
    int32_t x = StartCriticalSection();
    sched_ErrCode_t errorCode = NO_ERROR;

    //Get the ID of the thread NOT to kill
    threadId_t curThread = G8RTOS_GetThreadId();

    //Iterate through all other threads, looking for living threads with different IDs
    for(uint16_t i = 0; i < MAX_THREADS; i++){
        //If alive and has different ID, kill
        if((threadControlBlocks[i].threadID != curThread) && threadControlBlocks[i].isAlive){
            //Kill that thread
            errorCode = G8RTOS_KillThread(threadControlBlocks[i].threadID);
        }
        if(errorCode != NO_ERROR){
            //If errorCode is not NO_ERROR then something went wrong
            break;
        }
    }


    EndCriticalSection(x);
    return errorCode;
}



