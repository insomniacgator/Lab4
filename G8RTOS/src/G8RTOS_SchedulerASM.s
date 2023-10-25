; G8RTOS_SchedulerASM.s
; Created: 2022-07-26
; Updated: 2022-07-26
; Contains assembly functions for scheduler.

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

	;Load the address of RunningPtr
	LDR R0, RunningPtr
	;Load the currently running pointer using the address of runningptr stored into r0 from previous step
	LDR R1, [R0]
	;Load the first thread's stack pointer
	LDR SP, [R1] ; IS THIS WHY WE WANT THE STACK POINTER TO BE THE FIRST VARIABLE of the TCB struct?
	;Load LR with the first thread's PC
	LDR LR, [SP, #56] ; Assuming the PC is stored at an offset of 14 words in the SP (4-byte word). Load LR with PC (2 spots from last)

	BX LR				;Branches to the link register (thread function of first thread)				;Branches to the first thread

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
	; put your assembly code here!
	CPSID I				; disable interrupts
	PUSH {R4 - R11}		; push R4-R11 into the stack (others are pushed automatically)

	; put your assembly code here!
	LDR R4, RunningPtr  ; Load TCB pointer into RunningPtr
	LDR R5, [R4]
	;STR R5, [SP]
	STR SP, [R5]
	PUSH {LR}

	BL G8RTOS_Scheduler	;

	POP {LR}
	LDR R4, RunningPtr
	LDR R5, [R4]		; get new stack pointer from next thread
	LDR SP, [R5]




	POP {R4 - R11}		; pop R4-R11 from the stack
	CPSIE I				; re-enable interrupts

	BX LR				; return

	.endasmfunc

	; end of the asm file
	.align
	.end
