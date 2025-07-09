/*
 * k_task.c
 *
 *  Created on: Jan 28, 2024
 *      Author: maste
 */


/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdio.h> //You are permitted to use this library, but currently only printf is implemented. Anything else is up to you!
#include "common.h"
#include "k_task.h"


// KERNEL MEMORY VARIABLES/API
#include "k_mem.h"
char kernel_memory_started = 0;
extern uint32_t _img_end;
extern uint32_t _estack;
extern uint32_t _Min_Stack_Size;
char* IMG_END;

char* HEAP_START;//Start of heap
char* HEAP_END;//End of heap - Around 0x2000000
METADATA* head; //Pointer to first metadata block in allocated linked list
METADATA* free_head; //Pointer to first METADATA block in free linked list
METADATA head_meta;


// KERNEL TASK VARIABLES/API
TCB scheduler_TCB[MAX_TASKS];
uint32_t* MSP_INIT_VAL;
uint32_t tot_task_size = 0;
uint32_t svc_number;


int kernel_status = 0;  // 0 for uninitialized, 1 for initialized, 2 kernel is running.
int kernel_init = 0;
int firstTaskRun = 0;
int exitflag = 0;
int task_exit = 0;

uint32_t total_thread_size = 0;

int current_task = 0;
int next_task = 0;


uint32_t* curr_ptr;
uint32_t* next_ptr;


/**
  * @brief  The application entry point.
  * @retval int
  */


/*_____________________________________________________________________________________________________
 	 	 	 	 	 	 	 	 	 	 KERNEL TASK API
_____________________________________________________________________________________________________*/

int osScheduler (void) {
	unsigned int min_deadline = MAX_TIMESTAMP; //SET TO LARGE VALUE TO FIND SMALLEST DEADLINE
		int index = 0; // will find the index in TCB array of the next task to run

		for (int i = 1; i < MAX_TASKS; i++) { //WANT TO START FROM INDEX 1 SINCE INDEX 0 IS RESERVED FOR NULL TASK
			if (scheduler_TCB[i].timestamp < min_deadline && scheduler_TCB[i].state == READY) { //FIND THE TASK WITH EARLIEST DEADLINE AND ONLY SELECT IF READY
				min_deadline = scheduler_TCB[i].timestamp;
				index = i;
			}
			else if(scheduler_TCB[i].timestamp == min_deadline && scheduler_TCB[i].state == READY){
				if(i < index){
					index = i;
				}
			}
		}

	scheduler_TCB[index].state = RUNNING;
	return index; //RUN NULL TASK IF NO TASK WAS FOUND, ELSE RETURN TASK INDEX WITH EARLIEST DEADLINE
}


void osKernelInit(void) {
	__disable_irq();

	MSP_INIT_VAL =  *(uint32_t**) 0x0;

	kernel_init = 1; //Kernel has been initialized

	k_mem_init();

	char* ptr = (char*)k_mem_alloc(STACK_SIZE);


	if(ptr == NULL) {
		return;
	}

	METADATA* temp = ptr - META_SIZE; //GET LOCATION OF ALLOCATED MEMORY BLOCK
	temp->owned_task = 0; //SET NEW OWNER TO THE NEXT TASK BEING CREATED. THIS IS LEGAL SINCE WE'RE IN THE KERNEL -> NO VIOLATION OF SECURITY

	//CREATE NULL TASK
	scheduler_TCB[0].tid = 0;
	scheduler_TCB[0].ptask = &null_task;
	scheduler_TCB[0].stack_high = ptr + STACK_SIZE;
	scheduler_TCB[0].stack_location = scheduler_TCB[0].stack_high;
	scheduler_TCB[0].state = READY;
	scheduler_TCB[0].stack_size = STACK_SIZE;
	scheduler_TCB[0].deadline = -1;
	scheduler_TCB[0].timestamp = -1;
	scheduler_TCB[0].sleep_count = -1;


	++tot_task_size; //Increment the current number of tasks

	*(--scheduler_TCB[0].stack_location) = 1<<24; //This is xPSR, setting the chip to Thumb mode
	*(--scheduler_TCB[0].stack_location) = (uint32_t)scheduler_TCB[0].ptask; //the function name

	for (int j = 0; j < 14; j++) {
		*(--scheduler_TCB[0].stack_location) = 0xA;
	}

	for (int i = 1; i < MAX_TASKS; i++) {
		scheduler_TCB[i].tid = -1;
		scheduler_TCB[i].ptask = NULL;
		scheduler_TCB[i].stack_high = NULL;
		scheduler_TCB[i].stack_location = NULL;
		scheduler_TCB[i].state = DORMANT;
		scheduler_TCB[i].stack_size = 0x0;
		scheduler_TCB[i].deadline = 0;
		scheduler_TCB[i].timestamp = 0;
		scheduler_TCB[i].sleep_count = -1;
	}

	//SET PRIORITIES OF INTERRUPTS
	SHPR3 |= 0xFFU << 24; //Set the priority of SysTick to be the weakest
	SHPR3 |= 0xFEU << 16; //Shift the constant 0xFE 16 bits to set PendSV priority
	SHPR2 |= 0xFDU << 24; //Set the priority of SVC higher than PendSV

	__enable_irq();
}

int osKernelStart(void) { //Doesn't need to return RTX_OK, only RTX_ERR if anything goes wrong
	if (kernel_status == 1 || kernel_init == 0) { //If kernel not initialized or already running??
		 return RTX_ERR;
	}

	else {
		kernel_status = 1; //Set kernel to running

		current_task = osScheduler();

		if (current_task > -1) {
			curr_ptr = scheduler_TCB[current_task].stack_location;
			next_ptr = 0;
			svc_number = 1;
			__asm("SVC #1"); //Call SVC Handler

			return RTX_OK;
		}

		return RTX_ERR; //Return if no tasks found
	}
}

int osCreateTask(TCB* task) {

	__disable_irq();

	if (tot_task_size > MAX_TASKS) {
		__enable_irq();
		return RTX_ERR;
	}

	for (int i = 1; i < MAX_TASKS; i++) {

		if (scheduler_TCB[i].stack_size == 0x0 && scheduler_TCB[i].state == DORMANT) { //First time creating task in thread stack slot

			char* ptr = (char*)k_mem_alloc(task->stack_size);

			if(ptr == NULL) {
				__enable_irq();
				return RTX_ERR;
			}

			METADATA* temp = ptr - META_SIZE; //GET LOCATION OF ALLOCATED MEMORY BLOCK
			temp->owned_task = i; //SET NEW OWNER TO THE NEXT TASK BEING CREATED. THIS IS LEGAL SINCE WE'RE IN THE KERNEL -> NO VIOLATION OF SECURITY



			scheduler_TCB[i].tid = i;
			scheduler_TCB[i].ptask = task->ptask;
			scheduler_TCB[i].stack_size = task->stack_size;
			scheduler_TCB[i].stack_high = ptr + task->stack_size;
			scheduler_TCB[i].stack_location = scheduler_TCB[i].stack_high;
			scheduler_TCB[i].state = READY;
			scheduler_TCB[i].deadline = 5; //DEFAULT VALUE
			scheduler_TCB[i].timestamp = 5; //DEFAULT VALUE
			scheduler_TCB[i].sleep_count = -1; //DEFAULT VALUE

			++tot_task_size; //Increment the current number of tasks

			// push the task info into the Thread stack
			*(--scheduler_TCB[i].stack_location) = 1<<24; //This is xPSR, setting the chip to Thumb mode
			*(--scheduler_TCB[i].stack_location) = (uint32_t)scheduler_TCB[i].ptask; //the function name

			for (int j = 0; j < 14; j++) {
				*(--scheduler_TCB[i].stack_location) = 0xA;
			}

			__enable_irq();

			if (scheduler_TCB[i].timestamp < scheduler_TCB[current_task].timestamp && current_task != 0) { //IF NEW TASK HAS SHORTER DEADLINE

				__disable_irq();

				scheduler_TCB[current_task].state = READY; //SET CURRENT TASK BACK TO READY

				next_task = i;
				next_ptr = scheduler_TCB[next_task].stack_location;

				svc_number = 2;

				__enable_irq();
				__asm("SVC #2"); //CALL INTERRUPT TO PERFORM CONTEXT SWITCH

			}

			return RTX_OK;

		}

		else if (scheduler_TCB[i].state == DORMANT && scheduler_TCB[i].stack_size >= task->stack_size) { //Thread stack location was already visited
			task->tid = i;
			scheduler_TCB[i].tid = i;
			scheduler_TCB[i].ptask = task->ptask;
			scheduler_TCB[i].stack_size = task->stack_size;
			scheduler_TCB[i].stack_location = scheduler_TCB[i].stack_high;
			scheduler_TCB[i].state = READY;
			scheduler_TCB[i].deadline = 5; //DEFAULT VALUE
			scheduler_TCB[i].timestamp = 5; //DEFAULT VALUE
			scheduler_TCB[i].sleep_count = -1; //DEFAULT VALUE
			++tot_task_size; //Increment the current number of tasks

			// push the task info into the Thread stack
			*(--scheduler_TCB[i].stack_location) = 1<<24; //This is xPSR, setting the chip to Thumb mode
			*(--scheduler_TCB[i].stack_location) = (uint32_t)scheduler_TCB[i].ptask; //the function name

			for (int j = 0; j < 14; j++) {
				*(--scheduler_TCB[i].stack_location) = 0xA;
			}


			if (scheduler_TCB[i].timestamp < scheduler_TCB[current_task].timestamp && current_task != 0) { //IF NEW TASK HAS SHORTER DEADLINE

				scheduler_TCB[current_task].state = READY; //SET CURRENT TASK BACK TO READY

				next_task = i;
				next_ptr = scheduler_TCB[next_task].stack_location;

				svc_number = 2;

				__enable_irq();

				__asm("SVC #2"); //CALL INTERRUPT TO PERFORM CONTEXT SWITCH

			}

			__enable_irq();
			return RTX_OK;
		}

	}

	__enable_irq();

	return RTX_ERR;
}

void osYield(void) {
	__disable_irq();

	scheduler_TCB[current_task].timestamp = scheduler_TCB[current_task].deadline; //Set last task to ready -> We came from osYield

	next_task = osScheduler(); //Gets next task to run
	scheduler_TCB[current_task].state = READY; //Set last task to ready -> We came from osYield
	if (current_task == next_task) {
		__enable_irq();
		return;
	}

	next_ptr = scheduler_TCB[next_task].stack_location; //Update to next stack

	svc_number = 2;

	__enable_irq();
	__asm ("SVC #2"); //Call SVC to change running task
}



void PendSV_Handler_Main(unsigned int* svc_args) {
	svc_number = ( (char*) svc_args[ 6 ]) [ -2 ];
}


int osTaskExit(void) {

	if (tot_task_size){ //If called by running tasks

		--tot_task_size; //Decrement number of running tasks

		next_task = osScheduler();
		scheduler_TCB[current_task].state = DORMANT;

		METADATA* temp = (char*)scheduler_TCB[current_task].stack_high - (char*)scheduler_TCB[current_task].stack_size - META_SIZE; //GET LOCATION OF ALLOCATED MEMORY BLOCK

		k_mem_dealloc((char*)scheduler_TCB[current_task].stack_high - (char*)scheduler_TCB[current_task].stack_size);

		next_ptr = scheduler_TCB[next_task].stack_location;

		svc_number = 3;
		__asm ("SVC #3");
		return RTX_OK;

	} else { //Return error else
		return RTX_ERR;
	}
}

int osTaskInfo (int TID, TCB* task_copy){
	if (TID < 0 || TID > 15) {
		return RTX_ERR;
	}

    if(scheduler_TCB[TID].tid != TID || scheduler_TCB[TID].state == DORMANT){
        return RTX_ERR;
    }

    else{
    	//DO WE NEED TO COPY ALL THESE VARIABLES????
        task_copy->tid = scheduler_TCB[TID].tid;
        task_copy->ptask = scheduler_TCB[TID].ptask;
        task_copy->stack_high = scheduler_TCB[TID].stack_high;
        task_copy->state = scheduler_TCB[TID].state;
        task_copy->stack_size = scheduler_TCB[TID].stack_size;
        task_copy->deadline = scheduler_TCB[TID].deadline;
        task_copy->timestamp = scheduler_TCB[TID].timestamp;
        task_copy->sleep_count = scheduler_TCB[TID].sleep_count;

        return RTX_OK;
    }
}

void setStack(void) {
	scheduler_TCB[current_task].stack_location = __get_PSP(); //Update my current stack high variable
	current_task = next_task; //Set current_task index to next_task index
	curr_ptr = next_ptr; //Set new current pointer to task running
}

void setStackExit(void) {
	current_task = next_task; //Set current_task index to next_task index
	curr_ptr = next_ptr; //Set new current pointer to task running
}

void setStackFirst(void) {
	scheduler_TCB[current_task].stack_location = __get_PSP(); //Update my current stack high variable
}

/*_____________________________________________________________________________________________________
 	 	 	 	 	 	 	 	 	 	 KERNEL MEMORY API
_____________________________________________________________________________________________________*/

int k_mem_init(void) {
	if (kernel_memory_started == 1 || kernel_init != 1) {
		return RTX_ERR;
	}

	HEAP_START = (char*)&_img_end; //Starting location of heap -> Heap starts at bottom and grows up.
	HEAP_END = (char*)&_estack - (char*)&_Min_Stack_Size; //Ending location of heap

	head_meta.allocation_status = FREE;
	head_meta.next = NULL; //Previous pointer initially NULL
	head_meta.prev = NULL; //Next pointer initially NULL
	head_meta.next_free = NULL; //Previous pointer initially NULL
	head_meta.prev_free = NULL; //Next pointer initially NULL
	head_meta.owned_task = -1; //USING CHAR SETS THIS TO 255 INSTEAD OF -1
	head_meta.start_location = (char*)HEAP_START + META_SIZE;
	head_meta.node_size = HEAP_END - HEAP_START - META_SIZE; //Size of memory available. This is reversed since HEAP_START is at the bottom and HEAP_END is at the top.


	*(METADATA*)HEAP_START = head_meta;

	free_head = HEAP_START; //Set head pointer equal to first metadata block;
	head = HEAP_START; //Set head pointer of allocated linked list equal to NULL since no blocks are allocated yet
	//ADD RETURN VALUES HERE PLEASE!!!

	kernel_memory_started = 1;

	return RTX_OK;
}

// Two linked lists one for all blocks in total with next and free pointing to the adjacent next and free blocks

void* k_mem_alloc(unsigned int size) {

	if (kernel_memory_started == 0 || kernel_init != 1 || size <= 0) {
		return NULL;
	}

	size_t allocate;
	allocate = size;

	if (allocate <= 4) {
		allocate = 4;
	}else {
		allocate = (size + 3) & (~3);
	}

	METADATA* curr = free_head;

	while (curr != NULL) { //Use this to traverse through entire linked list
		if (curr->node_size >= allocate + META_SIZE + 4) { //Check if we have enough room to allocate requested bytes + metadata block
			//Always creating METADATA block no matter what
			METADATA temp; //Create new temp block which is new free memory
			temp.allocation_status = FREE; // New metadata block -> Free to use
			temp.owned_task = -1; //Metadata not owned by any task
			temp.node_size = curr->node_size - (allocate + META_SIZE); //Uses total free size of current node.

			temp.next = curr->next; // next is same as curr next
			temp.prev = curr; // next prev is curr prev
			temp.next_free = curr->next_free; // next free
			temp.prev_free = curr->prev_free;

			//Adjusting current metadata block to allocated metadata block
			curr->allocation_status = ALLOCATED; //Set metadata block to allocated now
			curr->owned_task = current_task; //Metadata block owned by current task
			curr->node_size = allocate; //Size of node is equal to requested number of bytes

			METADATA* VALUE = (METADATA*)((int)curr + (int)curr->node_size + (int)META_SIZE);
			temp.start_location = (char*)VALUE + META_SIZE;

			*(METADATA*) VALUE = temp;

			if(curr->next != NULL){
				curr->next->prev = VALUE;
			}
			if(curr->next_free != NULL){
				curr->next_free->prev_free = VALUE;
			}
			if (curr->prev_free != NULL) {
				curr->prev_free->next_free = VALUE;
			}
			curr->next = VALUE;
			curr->next_free = NULL;
			curr->prev_free = NULL;

			if(curr == free_head) {//Curr the head of free linked list and not free anymore (Allocated now)...
				free_head = VALUE; //Setting new head of free linked list.
			}

			return (void*)((char*)curr + META_SIZE);

		} else if (curr->node_size >= allocate ) { //Check if we have enough room to allocate requested bytes
			//Adjusting current metadata block to allocated metadata block
			curr->allocation_status = ALLOCATED; //Set metadata block to allocated now
			curr->owned_task = current_task; //Metadata block owned by current task
			//curr->node_size = allocate; //Size of node is equal to requested number of bytes

			if(curr == free_head){
				if (curr->next_free == NULL){
					free_head = NULL;
				}
				else
				{
					free_head = curr->next_free;
				}

			}

			if (curr->prev_free != NULL){
				curr->prev_free->next_free = curr->next_free;
			}

			if(curr->next_free != NULL){
				curr->next_free->prev_free = curr->prev_free;
			}

			curr->next_free = NULL;
			curr->prev_free = NULL;

			return (void*)((char*)curr + META_SIZE);
		}

		curr = curr->next_free; //Traverse to next metadata block if applicable
	}

	return NULL; //If no available memory found
}

int k_mem_dealloc(void* ptr) {
	// check for errors/do nothings based on the *ptr passed in
		if (kernel_memory_started == 0 || kernel_init != 1) {
			return RTX_ERR;
		}
		if (ptr == NULL) {
			return RTX_OK;
		}
		if ((ptr <= ((char*)HEAP_START + META_SIZE)) && (ptr > (char*)HEAP_END)) {
			return RTX_ERR;
		}

		// Find pointer to metadata from given starting block (subtract *ptr by META_SIZE).
		// Check if ptr passed in is indeed a starting address.

		//Default non-erroneaous ptr_data assignment: (Assuming ptr is valid and non-random)
		METADATA *ptr_data = ptr - META_SIZE;

		if (ptr_data->start_location != ptr){ // checks if the ptr is a start address
			return RTX_ERR;
		}

		// check for more errors specific to ptr's metadata block info
		if (ptr_data->owned_task != current_task  || ptr_data->allocation_status == FREE) {
			return RTX_ERR;
		}

		// update the following ptr_data block's parameters (not the prev. ones):
		ptr_data->allocation_status = FREE;
		ptr_data->owned_task = -1;

		if (ptr_data->prev != NULL) {
			if (ptr_data->prev->allocation_status == FREE) { //coalesce with previous block
				// update the following ptr_data->prev block's parameters:
				ptr_data->prev->node_size += (META_SIZE + ptr_data->node_size);
				ptr_data->prev->next = ptr_data->next;
				ptr_data->next->prev = ptr_data->prev;
				// cleanup dangling pointers in ptr_data by doing the following:
				ptr_data->next = NULL;
				METADATA *temp = ptr_data;
				ptr_data = ptr_data->prev;
				temp->prev = NULL;
			} else {
				// We must update the Free List:
				// Idea is that we must traverse backwards (using prev pointer as long as prev != NULL)
				// and find the first block that has allocation_status == FREE. Send the metadata ptr of
				// the block to a temp ptr holder called last_free.
				// If none are found, last_free = NULL.

				METADATA *last_free = ptr_data->prev;
				while (last_free != NULL) {
					if (last_free->allocation_status == FREE) {
						break;
					}
					last_free = last_free->prev;
				}

				if (last_free == NULL){
					ptr_data->prev_free = NULL;
					ptr_data->next_free = free_head;
					if (free_head != NULL) {
						free_head->prev_free = ptr_data;
					}
					free_head = ptr_data;
				} else {
					ptr_data->prev_free = last_free;
					ptr_data->next_free = last_free->next_free;
					if (ptr_data->next_free != NULL) {
						ptr_data->next_free->prev_free = ptr_data;
					}
					last_free->next_free = ptr_data;
				}
			}
		} else { //First block Special case
			// adjust Free List:
			ptr_data->prev_free = NULL;
			ptr_data->next_free = free_head;
			if (free_head != NULL) {
				free_head->prev_free = ptr_data;
			}
			free_head = ptr_data;
		}

		if (ptr_data->next != NULL) {
			if (ptr_data->next->allocation_status == FREE) { //coalesce with next block
				ptr_data->node_size += (META_SIZE + ptr_data->next->node_size);
				ptr_data->next_free = ptr_data->next_free->next_free;
				if (ptr_data->next_free != NULL) {
					ptr_data->next_free->prev_free = ptr_data;
				}
				ptr_data->next = ptr_data->next->next;
				if (ptr_data->next) {
					ptr_data->next->prev = ptr_data;
				}
			}
		}

		return RTX_OK;
}


int k_mem_count_extfrag(unsigned int size){
	if (kernel_memory_started == 0 || kernel_init != 1) {
		return 0;
	}
	METADATA* curr = free_head;
	unsigned int count = 0;
	while (curr != NULL){
		if(curr->node_size + META_SIZE < size){
			count++;
		}

		curr = curr->next_free;
	}

	return count;
}


/*_____________________________________________________________________________________________________
 	 	 	 	 	 	 	 	 	KERNEL PRE-EMPTIVE API
_____________________________________________________________________________________________________*/

void osSleep(int timeInMs) {
	__disable_irq();

	scheduler_TCB[current_task].state = SLEEPING; //SET CURRENT TASK TO SLEEPING
	scheduler_TCB[current_task].sleep_count = timeInMs;

	next_task = osScheduler(); //Gets next task to run

	if (current_task == next_task) {
		__enable_irq();
		return;
	}

	next_ptr = scheduler_TCB[next_task].stack_location; //Update to next stack
	svc_number = 2;
	__enable_irq();
	__asm ("SVC #2"); //Call SVC to change running task

	return;
}

void osPeriodYield(void) {
	__disable_irq();

	if (scheduler_TCB[current_task].timestamp == 0) {
		osYield();
		__enable_irq();
		return;
	}
	__enable_irq();
	osSleep(scheduler_TCB[current_task].timestamp);
	return;
}


int osCreateDeadlineTask(int deadline, int s_size, TCB* task) {

	__disable_irq();

	if (deadline <= 0 || tot_task_size > MAX_TASKS || task->stack_size < STACK_SIZE) {
		return RTX_ERR;
	}

	for (int i = 1; i < MAX_TASKS; i++) {

		if (scheduler_TCB[i].stack_size == 0x0 && scheduler_TCB[i].state == DORMANT) { //First time creating task in thread stack slot

			char* ptr = (char*)k_mem_alloc(s_size);

			if(ptr == NULL) {
				return RTX_ERR;
			}

			METADATA* temp = ptr - META_SIZE; //GET LOCATION OF ALLOCATED MEMORY BLOCK
			temp->owned_task = i; //SET NEW OWNER TO THE NEXT TASK BEING CREATED. THIS IS LEGAL SINCE WE'RE IN THE KERNEL -> NO VIOLATION OF SECURITY

			task->tid = i;
			scheduler_TCB[i].tid = i;
			scheduler_TCB[i].ptask = task->ptask;
			scheduler_TCB[i].stack_size = s_size;
			scheduler_TCB[i].stack_high = ptr + s_size;
			scheduler_TCB[i].stack_location = scheduler_TCB[i].stack_high;
			scheduler_TCB[i].state = READY;
			scheduler_TCB[i].deadline = deadline; //USER VALUE
			scheduler_TCB[i].timestamp = deadline; //USER VALUE
			scheduler_TCB[i].sleep_count = -1; //DEFAULT VALUE

			++tot_task_size; //Increment the current number of tasks

			// push the task info into the Thread stack
			*(--scheduler_TCB[i].stack_location) = 1<<24; //This is xPSR, setting the chip to Thumb mode
			*(--scheduler_TCB[i].stack_location) = (uint32_t)scheduler_TCB[i].ptask; //the function name

			for (int j = 0; j < 14; j++) {
				*(--scheduler_TCB[i].stack_location) = 0xA;
			}


			if (scheduler_TCB[i].timestamp < scheduler_TCB[current_task].timestamp && current_task != 0) { //IF NEW TASK HAS SHORTER DEADLINE


				scheduler_TCB[current_task].state = READY; //SET CURRENT TASK BACK TO READY

				next_task = i;
				next_ptr = scheduler_TCB[next_task].stack_location;

				svc_number = 2;

				__enable_irq();
				__asm("SVC #2"); //CALL INTERRUPT TO PERFORM CONTEXT SWITCH

			}
			__enable_irq();
			return RTX_OK;

		}

		else if (scheduler_TCB[i].state == DORMANT && scheduler_TCB[i].stack_size >= task->stack_size) { //Thread stack location was already visited
			task->tid = i;
			scheduler_TCB[i].tid = i;
			scheduler_TCB[i].ptask = task->ptask;
			scheduler_TCB[i].stack_size = task->stack_size;
			scheduler_TCB[i].stack_location = scheduler_TCB[i].stack_high;
			scheduler_TCB[i].state = READY;
			scheduler_TCB[i].deadline = deadline; //USER VALUE
			scheduler_TCB[i].timestamp = deadline; //USER VALUE
			scheduler_TCB[i].sleep_count = -1; //DEFAULT VALUE

			++tot_task_size; //Increment the current number of tasks

			// push the task info into the Thread stack
			*(--scheduler_TCB[i].stack_location) = 1<<24; //This is xPSR, setting the chip to Thumb mode
			*(--scheduler_TCB[i].stack_location) = (uint32_t)scheduler_TCB[i].ptask; //the function name

			for (int j = 0; j < 14; j++) {
				*(--scheduler_TCB[i].stack_location) = 0xA;
			}

			if (scheduler_TCB[i].timestamp < scheduler_TCB[current_task].timestamp && current_task != 0) { //IF NEW TASK HAS SHORTER DEADLINE

				scheduler_TCB[current_task].state = READY; //SET CURRENT TASK BACK TO READY

				next_task = i;
				next_ptr = scheduler_TCB[next_task].stack_location;

				svc_number = 2;
				__enable_irq();
				__asm("SVC #2"); //CALL INTERRUPT TO PERFORM CONTEXT SWITCH

			}

			__enable_irq();
			return RTX_OK;
		}

	}
	__enable_irq();
	return RTX_ERR;
}

int osSetDeadline(int deadline, int TID) {
	__disable_irq();

	if (TID < 1 || TID >= MAX_TASKS) {
		__enable_irq();
		return RTX_ERR;
	}

	if (scheduler_TCB[TID].state != READY) {
		__enable_irq();
		return RTX_ERR;
	}

	if(deadline <= 0 || scheduler_TCB[TID].tid == -1) {
		__enable_irq();
		return RTX_ERR;
	}

	scheduler_TCB[TID].deadline = deadline;
	scheduler_TCB[TID].timestamp = deadline;


	if (scheduler_TCB[TID].timestamp < scheduler_TCB[current_task].timestamp && current_task != 0) { //IF NEW TASK HAS SHORTER DEADLINE

		scheduler_TCB[current_task].state = READY; //SET CURRENT TASK BACK TO READY

		next_task = TID;
		next_ptr = scheduler_TCB[next_task].stack_location;

		svc_number = 2;
		__enable_irq();
		__asm("SVC #2"); //CALL INTERRUPT TO PERFORM CONTEXT SWITCH
		return RTX_OK;
	}

	__enable_irq();

	return RTX_OK;

}

void null_task(void) {
	while(1);
}
