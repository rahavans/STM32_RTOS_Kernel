# Lab 2 Grading Report

## Table of Contents
[[_TOC_]]

## test0
Compilation & compatibility with evaluation framework ~ calls osKernelInit, k_mem_init, and printf.

### expected output
```
---- test0 ----
PASS: build successful
<serial timeout>
<end of test>
```
### your output
```
---- test0 ----
PASS: build successful
<serial timeout>
<end of test>

```
### your score: 1.00/1.00

## test1
Allocate function is able to find free space if it exists.

### expected output
```
---- test1 ----
k_mem_alloc 933 bytes, ptr=0x2000a7b0
k_mem_alloc 743 bytes, ptr=0x2000ab68
k_mem_alloc 262 bytes, ptr=0x2000ae60
k_mem_alloc 529 bytes, ptr=0x2000af78
k_mem_alloc 700 bytes, ptr=0x2000b19c
Validating buffer contents... 
PASS: no corrupted buffers 
PASS: all k_mem_alloc calls successful 

allocating large block that fills up rest of the heap...
PASS: last block is 27544 bytes
<serial timeout>
<end of test>
```
### your output
```
---- test1 ----
k_mem_alloc 933 bytes, ptr=0x2000a7e4
k_mem_alloc 743 bytes, ptr=0x2000aba8
k_mem_alloc 262 bytes, ptr=0x2000aeac
k_mem_alloc 529 bytes, ptr=0x2000afd0
k_mem_alloc 700 bytes, ptr=0x2000b200
Validating buffer contents... 
PASS: no corrupted buffers 
PASS: all k_mem_alloc calls successful 

allocating large block that fills up rest of the heap...
PASS: last block is 27432 bytes
<serial timeout>
<end of test>

```
### your score: 1.00/1.00

## test2
Deallocation function makes a block available for allocation again.

### expected output
```
---- test2 ----
allocating 4 bytes...
allocating large block...
deallocating the first block...
PASS: k_mem_dealloc returned OK
allocating 4 bytes again... should fit into the first spot
PASS: freed memory was used again

<serial timeout>
<end of test>
```
### your output
```
---- test2 ----
allocating 4 bytes...
allocating large block...
deallocating the first block...
PASS: k_mem_dealloc returned OK
allocating 4 bytes again... should fit into the first spot
PASS: freed memory was used again

<serial timeout>
<end of test>

```
### your score: 1.00/1.00

## test3
Deallocated blocks are coalesced together correctly, to any free block in front and behind it. (Here the initial allocations are 4 bytes each).

### expected output
```
---- test3 ----
block 0 allocated, ptr=0x2000a3c8
block 1 allocated, ptr=0x2000a3dc
block 2 allocated, ptr=0x2000a3f0
block 3 allocated, ptr=0x2000a404
block 4 allocated, ptr=0x2000a418
block 5 allocated, ptr=0x2000a42c
block 6 allocated, ptr=0x2000a440
block 7 allocated, ptr=0x2000a454
block 8 allocated, ptr=0x2000a468
block 9 allocated, ptr=0x2000a47c
block 10 allocated, ptr=0x2000a490
deallocating 0x2000a3c8
deallocating 0x2000a3f0
deallocating 0x2000a42c
deallocating 0x2000a47c
block 11 allocated, ptr=0x2000a4a4
deallocating 0x2000a418, which is before a free block
testing if 8 bytes can fit at 0x2000a418 
PASS: k_mem_dealloc coalesced with a free block after it
deallocating 0x2000a440, which is after a free block
testing if 12 bytes can fit at 0x2000a418 
PASS: k_mem_dealloc coalesced with a free block before it
deallocating 0x2000a3dc, which is between two free blocks
testing if 12 bytes can fit at 0x2000a3c8 
PASS: k_mem_dealloc coalesced with two free blocks
<serial timeout>
<end of test>
```
### your output
```
---- test3 ----
block 0 allocated, ptr=0x2000a3fc
block 1 allocated, ptr=0x2000a41c
block 2 allocated, ptr=0x2000a43c
block 3 allocated, ptr=0x2000a45c
block 4 allocated, ptr=0x2000a47c
block 5 allocated, ptr=0x2000a49c
block 6 allocated, ptr=0x2000a4bc
block 7 allocated, ptr=0x2000a4dc
block 8 allocated, ptr=0x2000a4fc
block 9 allocated, ptr=0x2000a51c
block 10 allocated, ptr=0x2000a53c
deallocating 0x2000a3fc
deallocating 0x2000a43c
deallocating 0x2000a49c
deallocating 0x2000a51c
block 11 allocated, ptr=0x2000a55c
deallocating 0x2000a47c, which is before a free block
testing if 8 bytes can fit at 0x2000a47c 
PASS: k_mem_dealloc coalesced with a free block after it
deallocating 0x2000a4bc, which is after a free block
testing if 12 bytes can fit at 0x2000a47c 
PASS: k_mem_dealloc coalesced with a free block before it
deallocating 0x2000a41c, which is between two free blocks
testing if 12 bytes can fit at 0x2000a3fc 
PASS: k_mem_dealloc coalesced with two free blocks
<serial timeout>
<end of test>

```
### your score: 1.00/1.00

## test4
If a piece of left-over free memory is too small for a block, it is not leaked/lost, and it does not clobber the subsequent block.

### expected output
```
---- test4 ----
PASS: k_mem_dealloc returned OK
PASS: deallocated block was reusable
PASS: sliver of memory was not lost
<serial timeout>
<end of test>
```
### your output
```
---- test4 ----
PASS: k_mem_dealloc returned OK
PASS: deallocated block was reusable
PASS: sliver of memory was not lost
<serial timeout>
<end of test>

```
### your score: 1.00/1.00

## test5
Allocations are 4-byte aligned.

### expected output
```
---- test5 ----
PASS: allocated block for k_mem_alloc(5) is 24 bytes and allocated block for k_mem_alloc(8) is 24 bytes (should be the same)
<serial timeout>
<end of test>
```
### your output
```
---- test5 ----
PASS: allocated block for k_mem_alloc(5) is 36 bytes and allocated block for k_mem_alloc(8) is 36 bytes (should be the same)
<serial timeout>
<end of test>

```
### your score: 1.00/1.00

## test6
Deallocating a random pointer will return an error.

### expected output
```
---- test6 ----
allocating one block, then try to deallocate a pointer outside of this block...
PASS: k_mem_dealloc returned error (expected)
allocating large block...
deallocating a pointer part way in the large block
PASS: k_mem_dealloc returned error (expected)
try to deallocate a valid pointer twice...
PASS: k_mem_dealloc returned error on second call (expected)
<serial timeout>
<end of test>
```
### your output
```
---- test6 ----
allocating one block, then try to deallocate a pointer outside of this block...
PASS: k_mem_dealloc returned error (expected)
allocating large block...
deallocating a pointer part way in the large block
PASS: k_mem_dealloc returned error (expected)
try to deallocate a valid pointer twice...
PASS: k_mem_dealloc returned error on second call (expected)
<serial timeout>
<end of test>

```
### your score: 1.00/1.00

## test7
Only the owner can free a block of memory (this will require using the kernel to run 2 tasks and calling allocate & deallocate from those tasks).

### expected output
```
allocating from first task
deallocating from first task
PASS: k_mem_dealloc successful
allocating again from first task
deallocating from second task
PASS: k_mem_dealloc returned error (expected)
<serial timeout>
<end of test>
```
### your output
```
allocating from first task
deallocating from first task
PASS: k_mem_dealloc successful
allocating again from first task
deallocating from second task
PASS: k_mem_dealloc returned error (expected)
<serial timeout>
<end of test>

```
### your score: 1.00/1.00

## test8
The entire heap space should be back in one piece after deallocating everything (after random usage).

### expected output
```
---- test8 ----
Total allocatable heap space at the start = 31448 bytes

itr=0, alloc 933 bytes, ptr=0x2000a528
itr=1, alloc 743 bytes, ptr=0x2000a8e0
itr=2, alloc 262 bytes, ptr=0x2000abd8
also dealloc mem from itr 1, ptr=0x2000a8e0, ~743 bytes
itr=3, alloc 700 bytes, ptr=0x2000a8e0
...
now deallocate everything...
dealloc mem from itr 7, ptr=0x2000a748, ~351 bytes
dealloc mem from itr 8, ptr=0x2000aefc, ~843 bytes
dealloc mem from itr 14, ptr=0x2000b9e8, ~868 bytes
...

Total allocatable heap space at the end = 31448 bytes
PASS: heap space is back in one piece
<serial timeout>
<end of test>

```
### your output
```
---- test8 ----
Total allocatable heap space at the start = 31396 bytes

itr=0, alloc 933 bytes, ptr=0x2000a55c
itr=1, alloc 743 bytes, ptr=0x2000a920
itr=2, alloc 262 bytes, ptr=0x2000ac24
also dealloc mem from itr 1, ptr=0x2000a920, ~743 bytes
itr=3, alloc 700 bytes, ptr=0x2000a920
itr=4, alloc 508 bytes, ptr=0x2000ad48
also dealloc mem from itr 0, ptr=0x2000a55c, ~933 bytes
itr=5, alloc 256 bytes, ptr=0x2000a55c
itr=6, alloc 256 bytes, ptr=0x2000a678
also dealloc mem from itr 3, ptr=0x2000a920, ~700 bytes
itr=7, alloc 351 bytes, ptr=0x2000a794
itr=8, alloc 843 bytes, ptr=0x2000af60
also dealloc mem from itr 4, ptr=0x2000ad48, ~508 bytes
itr=9, alloc 393 bytes, ptr=0x2000a910
itr=10, alloc 330 bytes, ptr=0x2000aab8
also dealloc mem from itr 6, ptr=0x2000a678, ~256 bytes
itr=11, alloc 169 bytes, ptr=0x2000a678
itr=12, alloc 932 bytes, ptr=0x2000b2c8
also dealloc mem from itr 11, ptr=0x2000a678, ~169 bytes
itr=13, alloc 972 bytes, ptr=0x2000b688
itr=14, alloc 868 bytes, ptr=0x2000ba70
also dealloc mem from itr 5, ptr=0x2000a55c, ~256 bytes
itr=15, alloc 549 bytes, ptr=0x2000bdf0
itr=16, alloc 592 bytes, ptr=0x2000c034
also dealloc mem from itr 9, ptr=0x2000a910, ~393 bytes
itr=17, alloc 551 bytes, ptr=0x2000c2a0
itr=18, alloc 427 bytes, ptr=0x2000a55c
also dealloc mem from itr 16, ptr=0x2000c034, ~592 bytes
itr=19, alloc 624 bytes, ptr=0x2000c4e4
itr=20, alloc 635 bytes, ptr=0x2000c770
also dealloc mem from itr 10, ptr=0x2000aab8, ~330 bytes
itr=21, alloc 862 bytes, ptr=0x2000ca08
itr=22, alloc 484 bytes, ptr=0x2000a910
also dealloc mem from itr 13, ptr=0x2000b688, ~972 bytes
itr=23, alloc 301 bytes, ptr=0x2000ad48
itr=24, alloc 710 bytes, ptr=0x2000b688
also dealloc mem from itr 12, ptr=0x2000b2c8, ~932 bytes
itr=25, alloc 876 bytes, ptr=0x2000b2c8
itr=26, alloc 431 bytes, ptr=0x2000c034
also dealloc mem from itr 19, ptr=0x2000c4e4, ~624 bytes
itr=27, alloc 397 bytes, ptr=0x2000c4e4
itr=28, alloc 675 bytes, ptr=0x2000cd84
also dealloc mem from itr 2, ptr=0x2000ac24, ~262 bytes
itr=29, alloc 344 bytes, ptr=0x2000ab10
itr=30, alloc 134 bytes, ptr=0x2000ac84
also dealloc mem from itr 25, ptr=0x2000b2c8, ~876 bytes
itr=31, alloc 629 bytes, ptr=0x2000b2c8
itr=32, alloc 30 bytes, ptr=0x2000a724
also dealloc mem from itr 23, ptr=0x2000ad48, ~301 bytes
itr=33, alloc 126 bytes, ptr=0x2000ad28
itr=34, alloc 743 bytes, ptr=0x2000d044
also dealloc mem from itr 32, ptr=0x2000a724, ~30 bytes
itr=35, alloc 256 bytes, ptr=0x2000adc4
itr=36, alloc 932 bytes, ptr=0x2000d348
also dealloc mem from itr 34, ptr=0x2000d044, ~743 bytes
itr=37, alloc 509 bytes, ptr=0x2000d044
itr=38, alloc 119 bytes, ptr=0x2000b55c
also dealloc mem from itr 31, ptr=0x2000b2c8, ~629 bytes
itr=39, alloc 327 bytes, ptr=0x2000b2c8
itr=40, alloc 719 bytes, ptr=0x2000d708
also dealloc mem from itr 17, ptr=0x2000c2a0, ~551 bytes
itr=41, alloc 824 bytes, ptr=0x2000d9f4
itr=42, alloc 596 bytes, ptr=0x2000c200
also dealloc mem from itr 27, ptr=0x2000c4e4, ~397 bytes
itr=43, alloc 356 bytes, ptr=0x2000c470
itr=44, alloc 184 bytes, ptr=0x2000b42c
also dealloc mem from itr 41, ptr=0x2000d9f4, ~824 bytes
itr=45, alloc 245 bytes, ptr=0x2000c5f0
itr=46, alloc 7 bytes, ptr=0x2000a724
also dealloc mem from itr 40, ptr=0x2000d708, ~719 bytes
itr=47, alloc 352 bytes, ptr=0x2000d708
itr=48, alloc 665 bytes, ptr=0x2000d884
also dealloc mem from itr 47, ptr=0x2000d708, ~352 bytes
itr=49, alloc 738 bytes, ptr=0x2000db3c
now deallocate everything...
dealloc mem from itr 7, ptr=0x2000a794, ~351 bytes
dealloc mem from itr 8, ptr=0x2000af60, ~843 bytes
dealloc mem from itr 14, ptr=0x2000ba70, ~868 bytes
dealloc mem from itr 15, ptr=0x2000bdf0, ~549 bytes
dealloc mem from itr 18, ptr=0x2000a55c, ~427 bytes
dealloc mem from itr 20, ptr=0x2000c770, ~635 bytes
dealloc mem from itr 21, ptr=0x2000ca08, ~862 bytes
dealloc mem from itr 22, ptr=0x2000a910, ~484 bytes
dealloc mem from itr 24, ptr=0x2000b688, ~710 bytes
dealloc mem from itr 26, ptr=0x2000c034, ~431 bytes
dealloc mem from itr 28, ptr=0x2000cd84, ~675 bytes
dealloc mem from itr 29, ptr=0x2000ab10, ~344 bytes
dealloc mem from itr 30, ptr=0x2000ac84, ~134 bytes
dealloc mem from itr 33, ptr=0x2000ad28, ~126 bytes
dealloc mem from itr 35, ptr=0x2000adc4, ~256 bytes
dealloc mem from itr 36, ptr=0x2000d348, ~932 bytes
dealloc mem from itr 37, ptr=0x2000d044, ~509 bytes
dealloc mem from itr 38, ptr=0x2000b55c, ~119 bytes
dealloc mem from itr 39, ptr=0x2000b2c8, ~327 bytes
dealloc mem from itr 42, ptr=0x2000c200, ~596 bytes
dealloc mem from itr 43, ptr=0x2000c470, ~356 bytes
dealloc mem from itr 44, ptr=0x2000b42c, ~184 bytes
dealloc mem from itr 45, ptr=0x2000c5f0, ~245 bytes
dealloc mem from itr 46, ptr=0x2000a724, ~7 bytes
dealloc mem from itr 48, ptr=0x2000d884, ~665 bytes
dealloc mem from itr 49, ptr=0x2000db3c, ~738 bytes

Total allocatable heap space at the end = 31396 bytes
PASS: heap space is back in one piece
<serial timeout>
<end of test>

```
### your score: 1.00/1.00

## test9
Execution time over a series of allocations; excution time over a series of deallocations. (Thresholds are calculated as the midpoint between the compiler's implementation and linear search through all blocks.)

### expected output
```
---- test9 ----
Using DWT for timing

k_mem_alloc runtime=80901 [this is the threshold]
Time per iteration:
... 

k_mem_dealloc runtime=65369 [this is the threshold]
Time per iteration:
... 

<serial timeout>
<end of test>

```
### your output
```
---- test9 ----
Using DWT for timing

k_mem_alloc runtime=23110
Time per iteration:
242, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 231, 230, 

k_mem_dealloc runtime=102491
Time per iteration:
1830, 1801, 1785, 1769, 1753, 1737, 1721, 1705, 1689, 1673, 1657, 1641, 1625, 1609, 1593, 1577, 1561, 1545, 1529, 1513, 1497, 1481, 1465, 1449, 1433, 1417, 1401, 1385, 1369, 1353, 1337, 1321, 1305, 1289, 1273, 1257, 1241, 1225, 1209, 1193, 1177, 1161, 1145, 1129, 1113, 1097, 1081, 1065, 1049, 1033, 1017, 1001, 985, 969, 953, 937, 921, 905, 889, 873, 857, 841, 825, 809, 793, 777, 761, 745, 729, 713, 697, 681, 665, 649, 633, 617, 601, 585, 569, 553, 537, 521, 505, 489, 473, 457, 441, 425, 409, 393, 377, 361, 345, 329, 313, 297, 281, 265, 249, 211, 

<serial timeout>
<end of test>

```
### your score: 0.50/1.00

## test10
Execution time performance over random usage, i.e. allocations with random sizes interspersed with deallocations with random addresses. (Thresholds are calculated as the midpoint between the compiler's implementation and linear search through all blocks.)

### expected output
```
---- test10 ----
Using DWT for timing

Total runtime=69459.5 [this is the threshold]
Time per iteration (^ denotes deallocations):
...

<serial timeout>
<end of test>
```
### your output
```
---- test10 ----
Using DWT for timing

Total runtime=38017
Time per iteration (^ denotes deallocations):
270, 240^, 268, 268, 268, 261, 302^, 268, 268, 268, 257, 239^, 211, 268, 268, 257, 391^, 268, 268, 268, 257, 300^, 208, 268, 268, 257, 300^, 276, 273, 299, 285, 252^, 323, 323, 208, 285, 371^, 276, 304, 323, 312, 314^, 323, 323, 323, 312, 239^, 228, 323, 323, 217, 495^, 323, 304, 323, 312, 303^, 304, 350, 205, 312, 319^, 273, 350, 208, 312, 236^, 276, 276, 350, 339, 205^, 350, 276, 208, 312, 252^, 350, 228, 208, 285, 684^, 323, 273, 276, 312, 364^, 228, 323, 208, 285, 684^, 323, 323, 323, 312, 527^, 350, 228, 323, 312, 271^, 255, 228, 296, 285, 703^, 323, 304, 323, 312, 252^, 350, 350, 350, 197, 364^, 350, 350, 350, 339, 287^, 331, 377, 376, 

<serial timeout>
<end of test>

```
### your score: 1.00/1.00





