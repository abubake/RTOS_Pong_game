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
RunningPtr: .field CurrentlyRunningThread, 32 ;0x20006BB5 (C0 6B 00 20) what does that data mean?

; G8RTOS_Start
;	Sets the first thread to be the currently running thread
;	Starts the currently running thread by setting Link Register to tcb's Program Counter
G8RTOS_Start:

	.asmfunc
		; correct pushing here?
	ldr r0, RunningPtr ; Takes in address of CurrentlyRunningThread (0x20006C80) which has the address if the TCB's in it (it's data)
	ldr r1, [r0]	; loading the value at CurrentlyRunningThread's address into r1
	ldr sp, [r1]	; load from r1 into stack pointer ; task0 is 0x00 01 31 68

	pop {r4 - r11}	; restores
	pop {r0 - r3}	; restores
	pop {r12}
	add sp, sp, #4	; discard LR from initial stack
	pop {lr}	; the start location upon exiting

	add sp, sp, #4 ; discard the PSR

	cpsie I	; enable interrupts at level of the proccesor
	bx lr	; start first thread

	.endasmfunc


; PendSV_Handler
; - Performs a context switch in G8RTOS
; 	- Saves remaining registers into thread stack
;	- Saves current stack pointer to tcb
;	- Calls G8RTOS_Scheduler to get new tcb
;	- Set stack pointer to new stack pointer from new tcb
;	- Pops registers from thread stack
; 1. save context 2. call scheduler 3. load next scheduled thread's context
PendSV_Handler:
	;.align 4
	.asmfunc

	cpsid I
	push {r4 - r11}	; save remaining registers r4 - r11
	ldr r0, RunningPtr ; Takes in address of CurrentlyRunningThread (0x20006C80) which has the address if the TCB's in it (it's data)
	ldr r1, [r0]	; loading the value at CurrentlyRunningThread's address into r1
	str sp, [r1]	; load from r1 into stack pointer ; task0 is 0x00 01 31 68
	push {r0, lr}

	bl G8RTOS_Scheduler

	pop {r0, lr}
	;ldr r0, RunningPtr ; accessing the next TCB
	ldr r1, [r0]	;
	ldr sp, [r1]	;
	pop {r4 - r11}
	cpsie I
	bx lr	; next thread

	.endasmfunc
	
	; end of the asm file
	.align
	.end
