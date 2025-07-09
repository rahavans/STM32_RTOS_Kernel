/*
 * handler.s
 *
 *  Created on: Jan 25, 2024
 *      Author: maste
 */
 .syntax unified
.section .text

.global SVC_Handler
.global PendSV_Handler
.global svc_number
.global osExitHandler
.global osYieldHandler
.global osFirstTaskHandler
.global PendSV_Setup

.thumb

.thumb_func
SVC_Handler:
	.global SVC_Handler_Main

	TST LR, #4
	ITE EQ
	MRSEQ R0, MSP
	MRSNE R0, PSP

	LDR R1, =0xE000ED04

	LDR R2, [R1]
	LDR R3, =0x10000000
	ORR R3, R2

	STR R3, [R1]
	ISB

.thumb_func
PendSV_Handler:
	.global PendSV_Handler_Main
	//BL PendSV_Handler_Main

	LDR R2, =svc_number //Added this for now to ease debugging
	LDR R2, [R2]

	CMP R2, #1
	BEQ osFirstTaskHandler
	CMP R2, #2
	BEQ osYieldHandler
	CMP R2, #4 //For PendSV_setup
	BEQ osYieldHandler
	BNE osExitHandler //If SVC number is 3


.thumb_func
osExitHandler:
	.global setStackExit
	LDR R0, =next_ptr //Load next pointer location
	LDR R1, [R0] //Dereference next pointer
	LDMIA R1!, {R4-R11} //Load registers from next PSP

	MSR PSP, R1 //Set current PSP to next PSP

	BL setStackExit //To set current pointer to my next pointer and current index to next index

	MOV LR, #0xFFFFFFFD
	BX LR //Branch to function

.thumb_func
osYieldHandler:
	.global setStack
	MRS R0, PSP //Loading current PSP into R0
	STMDB R0!, {R4-R11} //Store current registers into PSP
	MSR PSP, R0 //Updating PSP location

	BL setStack //Branching to update stack high value

	LDR R0, =next_ptr //Getting next_ptr
	LDR R1, [R0] //Dereference next_ptr
	LDMIA R1!, {R4-R11} //Popping off current registers

	MSR PSP, R1 //Set current PSP to next PSP

	MOV LR, #0xFFFFFFFD
	BX LR //Branch to new function

.thumb_func
osFirstTaskHandler:
	.global setStackFirst
	LDR R3, =curr_ptr //Getting current stack pointer
	LDR R2, [R3] //Dereference pointer
	LDMIA R2!, {R4-R11} //Load values into registers, decrement R2 register
	MSR PSP, R2 //Set PSP to new location

	BL setStackFirst

	MOV LR, #0xFFFFFFFD
	BX LR //Branch to new function

.thumb_func
PendSV_Setup:

	LDR R1, =0xE000ED04

	LDR R2, [R1]
	LDR R3, =0x10000000
	ORR R3, R2

	STR R3, [R1]
	ISB

	B osYieldHandler

