/*
 * G8RTOS_Scheduler.c
 */

/*********************************************** Dependencies and Externs *************************************************************/
#include <stdint.h>
#include "msp.h"
#include <BSP.h>
#include <pong-lab/G8RTOS_lab2/G8RTOS.h>
#include <pong-lab/G8RTOS_lab2/G8RTOS_Structures.h>

extern void PendSV_Handler();
extern int32_t StartCriticalSection();
extern void EndCriticalSection(int32_t);


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

/*********************************************** Defines ******************************************************************************/


/*********************************************** Data Structures Used *****************************************************************/

/* Thread Control Blocks
 *	- An array of thread control blocks to hold pertinent information for each thread
 */
static tcb_t threadControlBlocks[MAX_THREADS];
static pt_t periodicControlBlocks[MAX_PERIODICS];

/* Thread threadStacks
 *	- An array of arrays that will act as invdividual stacks for each thread
 */
static int32_t threadStacks[MAX_THREADS][STACKSIZE];


/*********************************************** Data Structures Used *****************************************************************/


/*********************************************** Private Variables ********************************************************************/

/*
 * Current Number of Threads currently in the scheduler
 */
static uint32_t NumberOfThreads = 0;
static uint32_t NumberOfPeriodics = 0;
static uint16_t currentMaxPriortity = 256;
static uint16_t deathCount = 0;
static uint16_t IDCounter = 0;
static bool RTOS_Launched = false;

/*********************************************** Private Variables ********************************************************************/


/*********************************************** Private Functions ********************************************************************/

/*
 * Initializes the Systick and Systick Interrupt
 * The Systick interrupt will be responsible for starting a context switch between threads
 * Param "numCycles": Number of cycles for each systick interrupt
 */
static void InitSysTick(uint32_t numCycles)
{
    //think about using the function to get the clock speed
    SysTick->LOAD = numCycles;    //3MHz clk controlled by MCLK (1 ms -> 10^-3*3*10^6 = 3000) = 2999
    SysTick->VAL = 0;             //clear current clock val
    SysTick->CTRL = 7;            //enable systick, interrupt when systick is 0, system clock (MCLK) as source
}

/*
 * Chooses the next thread to run.
 * Lab 4 Scheduling Algorithm:
 * 	- Priorty Scheduler
 */
void G8RTOS_Scheduler()
{
    /******************************PERIODIC THREADS************************************/
   uint32_t lcm = getLCM();
    if(lcm || NumberOfPeriodics){
        uint32_t counter = (SystemTime) % lcm;
        for(uint8_t i=0; i<NumberOfPeriodics; i++){
            if((counter % periodicControlBlocks[i].period) == periodicControlBlocks[i].exec_time){
                periodicControlBlocks[i].handler();
            }
        }
    }
    /******************************PERIODIC THREADS************************************/
    //sleep evalulation
    tcb_t* ptr = CurrentlyRunningThread;
    tcb_t* end = CurrentlyRunningThread;

    for(uint8_t i = 0; i < NumberOfThreads; i++){
        if(ptr->asleep){
            if(SystemTime != 0 && SystemTime % (ptr->sleep_cnt-1) == 0){
                ptr->asleep = 0;
                ptr->sleep_cnt = 0;
            }
        }
        ptr = ptr->next;
    }

    //priority evaluation
    tcb_t* threadToRun = ptr;   //The thread to run intially is the currently running thread (base case)

    if(CurrentlyRunningThread->isAlive){    //ptr is pointing to CurrentlyRunning Thread now and there is no point in checking its priorty over itself so we move to the next thread
        ptr = ptr->next;
    }

    currentMaxPriortity = 256;
    for(uint8_t i = 0; i < NumberOfThreads; i++){
         if(ptr->isAlive && !ptr->asleep && !ptr->blocked){
             if(ptr->priority < currentMaxPriortity){
                 threadToRun = ptr;
                 currentMaxPriortity = ptr->priority;
             }
         }
         ptr = ptr->next;
     }
    CurrentlyRunningThread = threadToRun;
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
    ++SystemTime;
    _ICSR |= ICSR_PENDSVSET;
}

/*********************************************** Private Functions ********************************************************************/
void cntxt_switch()
{
    //forced context switch to next thread
    SysTick->VAL = 0;
    __DSB();
    __ISB();
    _ICSR |= ICSR_PENDSVSET;
}

void sleep(uint32_t duration){
    CurrentlyRunningThread->sleep_cnt = SystemTime + duration;
    CurrentlyRunningThread->asleep = 1;
    cntxt_switch();
}

int gcd(int n0, int n1)
{
    //base case
    if (n1 == 0){
        //mod of anything by zero is itself
        return n0;
    }

    //recursive call
    return gcd(n1, n0 % n1);
}

uint32_t getLCM(){
    uint32_t lcm = periodicControlBlocks[0].period;

    for (uint32_t i = 1; i < NumberOfPeriodics; i++) {
        // check if i is a factor of both integers
        lcm = ((periodicControlBlocks[i].period * lcm) / (gcd(periodicControlBlocks[i].period, lcm)));
    }

    return lcm;
}

/*********************************************** Public Variables *********************************************************************/

/* Holds the current time for the whole System */
uint32_t SystemTime;
/*********************************************** Public Variables *********************************************************************/


/*********************************************** Public Functions *********************************************************************/

/*
 * Sets variables to an initial state (system time and number of threads)
 * Enables board for highest speed clock and disables watchdog
 */
void G8RTOS_Init()
{
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;
    //system time to zero
    SystemTime = 0;
    //set number of threads to zero
    NumberOfThreads = 0;
    NumberOfPeriodics = 0;
    //init hardware on board
    BSP_InitBoard();
    //relocate interrupt vector table
    uint32_t newVTORTable = 0x20000000;
    memcpy((uint32_t*)newVTORTable,(uint32_t*)SCB->VTOR,57*4);
    SCB->VTOR = newVTORTable;
}

/*
 * Starts G8RTOS Scheduler
 * 	- Initializes the Systick
 * 	- Sets Context to first thread
 * Returns: Error Code for starting scheduler. This will only return if the scheduler fails
 */
int G8RTOS_Launch()
{
    //I already set my currently running thread in AddThread
    //I built it that way so AddThread is an encapsulated Round-Robin making function
    InitSysTick(48000);
    NVIC_SetPriority(SysTick_IRQn, OSSYS_PRIORITY);
    NVIC_SetPriority(PendSV_IRQn, OSINT_PRIORITY);
    RTOS_Launched = true;

    G8RTOS_Start();
    return NO_ERROR;
}


/*
 * Adds threads to G8RTOS Scheduler
 * 	- Checks if there are stil available threads to insert to scheduler
 * 	- Initializes the thread control block for the provided thread
 * 	- Initializes the stack for the provided thread to hold a "fake context"
 * 	- Sets stack tcb stack pointer to top of thread stack
 * 	- Sets up the next and previous tcb pointers in a round robin fashion
 * Param "threadToAdd": Void-Void Function to add as preemptable main thread
 * Returns: Error code for adding threads
 */
sched_ErrCode_t G8RTOS_AddThread(void (*threadToAdd)(void), uint8_t priority, char* name){
    int32_t status = StartCriticalSection();

    if(NumberOfThreads == MAX_THREADS)
    {
        EndCriticalSection(status);
        return THREAD_LIMIT_REACHED; // Error: Max Threads
    }

    NumberOfThreads++;
    for(uint8_t threadIndex = 0; threadIndex < MAX_THREADS; threadIndex++){
        if(!threadControlBlocks[threadIndex].isAlive){
            //Replace info regarding this thread with the new one

            threadStacks[threadIndex][STACKSIZE-1] = THUMBBIT;
            threadControlBlocks[threadIndex].sp = &threadStacks[threadIndex][STACKSIZE-16];
            threadStacks[threadIndex][STACKSIZE - 2] = (int32_t)(threadToAdd);

            if(threadIndex == 0 && RTOS_Launched){
                threadControlBlocks[threadIndex].next = &threadControlBlocks[threadIndex+1];
                threadControlBlocks[threadIndex+1].prev->next = &threadControlBlocks[threadIndex];

                threadControlBlocks[threadIndex].prev = threadControlBlocks[threadIndex+1].prev;
                threadControlBlocks[threadIndex+1].prev = &threadControlBlocks[threadIndex];
            }

            else if(NumberOfThreads > 1){
                threadControlBlocks[threadIndex].next = &threadControlBlocks[0];
                threadControlBlocks[threadIndex].prev = threadControlBlocks[0].prev;

                threadControlBlocks[0].prev->next = &threadControlBlocks[threadIndex];
                threadControlBlocks[0].prev = &threadControlBlocks[threadIndex];
            }
            //one thread
            else{
              threadControlBlocks[0].next = &threadControlBlocks[0];
              threadControlBlocks[0].prev = &threadControlBlocks[0];
            }

            if(!RTOS_Launched){
              //set the Currently Running Pointer to the thread with the highest priority before launching RTOS
              if(threadControlBlocks[threadIndex].priority < currentMaxPriortity){
                  CurrentlyRunningThread = &threadControlBlocks[threadIndex];
                  currentMaxPriortity = threadControlBlocks[threadIndex].priority;
              }
            }

            threadControlBlocks[threadIndex].asleep = 0;
            threadControlBlocks[threadIndex].blocked = 0;
            threadControlBlocks[threadIndex].sleep_cnt = 0;
            threadControlBlocks[threadIndex].priority = priority;
            threadControlBlocks[threadIndex].isAlive = true;
            threadControlBlocks[threadIndex].threadID = ((IDCounter++) << 16) | threadIndex;

            for(uint8_t i = 0; i < MAX_NAME_LENGTH; i++){
            threadControlBlocks[threadIndex].threadName[i] = name[i];
            }

            EndCriticalSection(status);
            return NO_ERROR;
        }
    }
    NumberOfThreads--;
    EndCriticalSection(status);
    return THREADS_INCORRECTLY_ALIVE;;
}


int G8RTOS_AddPeriodic(void (*periodicToAdd)(void), uint32_t period)
{
    if(MAX_PERIODICS > NumberOfPeriodics){
        int32_t status = StartCriticalSection();
            //initialize periodic
            periodicControlBlocks[NumberOfPeriodics].cur_time = 0;
            periodicControlBlocks[NumberOfPeriodics].exec_time = NumberOfPeriodics;

            periodicControlBlocks[NumberOfPeriodics].handler = periodicToAdd;
            periodicControlBlocks[NumberOfPeriodics].period = period;


            if(NumberOfPeriodics > 0){
                periodicControlBlocks[NumberOfPeriodics - 1].next = &periodicControlBlocks[NumberOfPeriodics];
                periodicControlBlocks[NumberOfPeriodics].prev = &periodicControlBlocks[NumberOfPeriodics - 1];

                periodicControlBlocks[NumberOfPeriodics].next = &periodicControlBlocks[NumberOfPeriodics-1];    //doubly-linked
            }
            //one thread
            else{
                periodicControlBlocks[0].next = 0;
                periodicControlBlocks[0].prev = 0;

                //set the Currently Running Pointer to the first thread
                //CurrentlyRunningPeriodic = &periodicControlBlocks[0];
            }
            NumberOfPeriodics++;

        //this is the trival solution (will implement semaphores soon)
        EndCriticalSection(status);
        return 0;
    }
    else{
        return -1;
    }
}

sched_ErrCode_t G8RTOS_AddAPeriodicEvent(void (*AthreadToAdd)(void),uint8_t priority,IRQn_Type IRQn){
    int32_t status = StartCriticalSection();
    if(IRQn > PSS_IRQn && IRQn < PORT6_IRQn){\
        if(priority <= 6){
            __NVIC_SetVector(IRQn, (uint32_t) AthreadToAdd);
            __NVIC_SetPriority(IRQn, priority);
            __NVIC_EnableIRQ(IRQn);
            EndCriticalSection(status);
            return NO_ERROR;
        }
        else{
            EndCriticalSection(status);
            return HWI_PRIORITY_INVALID;
        }
    }
    else{
        EndCriticalSection(status);
        return IRQn_INVALID;
    }
}

threadId_t G8RTOS_GetThreadId(){
    return CurrentlyRunningThread->threadID;
}

sched_ErrCode_t G8RTOS_KillThread(threadId_t threadId){
    int32_t status = StartCriticalSection();
    if(NumberOfThreads == 1){
        EndCriticalSection(status);
        return CANNOT_KILL_LAST_THREAD;
    }

    tcb_t* ptr = CurrentlyRunningThread;
    bool killed = false;
    do{
        if(ptr->threadID == threadId){
            ptr->isAlive = false;
            killed = true;
            NumberOfThreads--;
            break;
        }
        ptr = ptr->next;
    }while(ptr != CurrentlyRunningThread);

    if(!killed){
        EndCriticalSection(status);
        return THREAD_DOES_NOT_EXIST;
    }

    tcb_t* temp = ptr->next;

    ptr->prev->next = ptr->next;
    temp->prev = ptr->prev;

    if(ptr == CurrentlyRunningThread){
        cntxt_switch();
    }

    EndCriticalSection(status);
    return NO_ERROR;
}

sched_ErrCode_t G8RTOS_KillSelf(){
    int32_t status = StartCriticalSection();
    if(NumberOfThreads == 1){
       return THREADS_INCORRECTLY_ALIVE;
    }
    CurrentlyRunningThread->isAlive = false;

    tcb_t* temp = CurrentlyRunningThread->next;

    CurrentlyRunningThread->prev->next = CurrentlyRunningThread->next;
    temp->prev = CurrentlyRunningThread->prev;

    NumberOfThreads--;
    EndCriticalSection(status);
    cntxt_switch();
    return NO_ERROR;
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
/*********************************************** Public Functions *********************************************************************/
