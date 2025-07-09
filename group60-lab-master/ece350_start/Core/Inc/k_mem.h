/*
 * k_mem.h
 *
 *  Created on: Jan 5, 2024
 *      Author: nexususer
 *
 *      NOTE: any C functions you write must go into a corresponding c file that you create in the Core->Src folder
 */

#ifndef INC_K_MEM_H_
#define INC_K_MEM_H_

// TOTAL METADATA STRUCT SIZE = 22 bytes;

typedef struct METADATA METADATA;

struct METADATA{
	unsigned char allocation_status; //1 byte
	char owned_task; //Which task owns this block -> 1 byte
	METADATA* next; //Pointer to previous metadata struct -> 4 Bytes
	METADATA* prev; //Pointer to previous metadata struct -> 4 Bytes
	METADATA* next_free; //Pointer to next metadata struct -> 4 Bytes
	METADATA* prev_free; //Pointer to previous metadata struct -> 4 Bytes
	unsigned int* start_location; //Start location of node -> 4 Bytes
	unsigned int node_size; //Size of current block (Allocated or unallocated) -> 4 bytes
};

int k_mem_init(void); //Initialize memory
void* k_mem_alloc(unsigned int size);
int k_mem_dealloc(void* ptr);
int k_mem_count_extfrag(unsigned int size);

#endif /* INC_K_MEM_H_ */
