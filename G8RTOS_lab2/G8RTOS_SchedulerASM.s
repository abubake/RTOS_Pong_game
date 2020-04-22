; G8RTOS_SchedulerASM.s
; Holds all ASM functions needed for the scheduler
; Note: If you have an h file, do not have a C file and an S file of the same name

	; Functions Defined
	.def G8RTOS_Start, PendSV_Handler

	; Dependencies
	.ref CurrentlyRunningThread, G8RTOS_Scheduler

	.thumb		; Set to thumb mode
	.align 2	; Align by 2 bytes (thumb mode uses allignment by 2 or 4)
	.text		; Text section

; Need to have the address defined in file 
; (label needs to be close enough to asm code to be reached with PC relative addressing)
RunningPtr: .field CurrentlyRunningThread, 32

; G8RTOS_Start
;	Sets the first thread to be the currently running thread
;	Starts the currently running thread by setting Link Register to tcb's Program Counter
G8RTOS_Start:

	.asmfunc
	LDR     R0, RunningPtr    ;take preset intial thread ptr and put it into R0
    LDR     R1, [R0]           ;R1 holds the thread now
    LDR     SP, [R1]           ;main stack pointer will take in R1 data
    POP     {R4-R11}           ;restore regs R4-R11
    POP     {R0-R3}            ;restore regs R0-R3
    POP     {R12}
    ADD     SP,SP,#4           ;discard LR from initial stack
    POP     {LR}               ;start location
    ADD     SP,SP,#4           ;discard PSR (jump over PC)
    CPSIE   I                  ;Enable interrupts at processor level
    BX      LR                 ;start first thread

	.endasmfunc

; PendSV_Handler
; - Performs a context switch in G8RTOS
; 	- Saves remaining registers into thread stack
;	- Saves current stack pointer to tcb
;	- Calls G8RTOS_Scheduler to get new tcb
;	- Set stack pointer to new stack pointer from new tcb
;	- Pops registers from thread stack
PendSV_Handler:
	
	.asmfunc

	CPSID   I       			;disable interrupts dueing context switch
    PUSH    {R4-R11}          	;saves remaining registers during context switch (others saved by branch already)
    LDR     R0, RunningPtr		;R0 will be the pointer from the old thread (MAY NEED TO USE =RunningPtr instead of CRT)
    LDR     R1, [R0]           	;R1 = the current thread
    STR     SP, [R1]        	;save the main stack pointer into the tcb stack to pick up where the thread left off

    PUSH    {LR}				;use the scheduler to get a new currently running thread
    BL      G8RTOS_Scheduler
    POP     {LR}

    LDR     R0, RunningPtr
    LDR     R1, [R0]         	;the new thread pointer is stored in R0 so put the addy of it in to R1
    LDR     SP, [R1]          	;put the addy of the new thread into the main stack pointer
    POP     {R4-R11}          	;restore r4-r11
    CPSIE   I                 	;enable interrupts
    BX      LR                	;branch to the new thread and restore R0-R3,r12,LR,PC, and PSR

	.endasmfunc
	
	; end of the asm file
	.align
	.end
