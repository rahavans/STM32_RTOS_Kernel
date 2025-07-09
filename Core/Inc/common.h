/*
 * common.h
 *
 *  Created on: Jan 5, 2024
 *      Author: nexususer
 *
 *      NOTE: If you feel that there are common
 *      C functions corresponding to this
 *      header, then any C functions you write must go into a corresponding c file that you create in the Core->Src folder
 */

#ifndef INC_COMMON_H_
#define INC_COMMON_H_

void SVC_Handler(void);
void SVC_Handler_Main(void);
void PendSV_Handler(void);
void PendSV_Handler_Main(unsigned int* svc_args);
void PendSV_Setup(void);

#define TID_NULL 0 //predefined Task ID for the NULL task
#define MAX_TASKS 16 //maximum number of tasks in the system
#define STACK_SIZE 0x400 //min. size of each task's stack
#define DORMANT 0 //state of terminated task
#define READY 1 //state of task that can be scheduled but is not running
#define RUNNING 2 //state of running task
#define SLEEPING 3 //STATE OF SLEEPING TASK
#define MAIN_STACK_SIZE 0x400
#define _ICSR (*((volatile uint32_t *) 0xE000ED04))

#define RTX_ERR 0
#define RTX_OK 1

#define ALLOCATED 1 // BLOCK IS ALLOCATED
#define FREE 0 //BLOCK IS FREE

#define META_SIZE 28 // Metadata struct is 28 bytes for byte alignment
#define MAX_TIMESTAMP 2147483647

typedef unsigned char U8;
typedef unsigned int U32;


#endif /* INC_COMMON_H_ */
