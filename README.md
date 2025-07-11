# STM32 RTOS Kernel

Hi! This is a project I worked on as part of my real-time operating systems course at uWaterloo, that involved developing a kernel for an STM32 that could handle task scheduling, dynamic memory allocation, and pre-emptive multitasking.

The RTOS follows a EDF (Earliest Deadline First) based scheduler, and was built in 3 steps: task scheduling, dynamic memory allocation, and then finally pre-emptive multitasking using timer interrupts and context switching (written in ARM Assembly).
