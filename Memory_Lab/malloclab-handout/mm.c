/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 *
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your information in the following struct.
 ********************************************************/
team_t team = {
  /* Team name : Your student ID */
  "2013-11438",
  /* Your full name */
  "Jiung Hahm",
  /* Your student ID */
  "2013-11438",
  /* leave blank */
  "",
  /* leave blank */
  ""
};

/* DON'T MODIFY THIS VALUE AND LEAVE IT AS IT WAS */
static range_t **gl_ranges;

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* Tracks allocated and free blocks. - Implicit list method. */

void* lst_current;
void* lst_start;
void* lst_end;
void* lst_tracker; // Tracker for searching block list from start to end, finding allocatable free block.
void* lst_current_free;
void* lst_start_free;
void* lst_end_free;
void* lst_tracker_free;
void* lst_free_next_temp;
void* lst_free_prev_temp;
void* lst_free_root_temp;


/* 
 * remove_range - manipulate range lists
 * DON'T MODIFY THIS FUNCTION AND LEAVE IT AS IT WAS
 */
static void remove_range(range_t **ranges, char *lo)
{
  range_t *p;
  range_t **prevpp = ranges;
  
  if (!ranges)
    return;

  for (p = *ranges;  p != NULL; p = p->next) {
    if (p->lo == lo) {
      *prevpp = p->next;
      free(p);
      break;
    }
    prevpp = &(p->next);
  }
}

/*
 * mm_init - initialize the malloc package.
 */
int mm_init(range_t **ranges)
{
  /* YOUR IMPLEMENTATION */

  /* Initially, lst points 8 bytes away from start of heap. lst_start and lst_end points to same address as lst. */
  lst_current = mem_sbrk(16);
  (*(int*)lst_current) = 16 + 1;
  (*(int*)(lst_current + 8)) = 16 + 1;
  lst_start = lst_current;
  lst_end = lst_current;
  

  /* DON't MODIFY THIS STAGE AND LEAVE IT AS IT WAS */
  gl_ranges = ranges;

  return 0;
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void* mm_malloc(size_t size)
{
  //int newsize = ALIGN(size + SIZE_T_SIZE);
  int newsize = ALIGN(size + 2*SIZE_T_SIZE); // 8 for header and footer of memory block in heap.
  //void *p = mem_sbrk(newsize);
  if (lst_start == lst_end) // Allocating case 1 : There is no block allocated.
  {
    printf("Allocating case 1\n");
    lst_current = mem_sbrk(newsize);
    if (lst_current == (void *)-1)
    {
      return NULL;
    }
    (*(int*)lst_current) = (newsize & -8) + 1;
    (*(int*)(lst_current + (newsize - 8))) = (newsize & -8) + 1;
    lst_end = lst_current + newsize;
    lst_start = lst_current;
  }
  else // Allocating case 2 : Allocated blocks exists. Free or allocated. Find 'free and allocatable size' block. If not exists, allocate new block at the end of list.
  {
    printf("Allocating case 2 - lst_start_free : %p, lst_tracker_free : %p, lst_end_free : %p\n", lst_start_free, lst_tracker_free, lst_end_free);
    int allocated = 0;
    for (lst_tracker_free = lst_start_free; (lst_tracker_free != lst_end_free); lst_tracker_free = ((void*)(*((int*)lst_tracker_free + 2))))
    {
      //printf("In for loop - lst_start_free : %p, lst_tracker_free : %p, lst_end_free : %p\n", lst_start_free, lst_tracker_free, lst_end_free);
      if (((*(int*)lst_tracker_free & -8) > newsize)) // Free and allocatable size block has been found.
      {
	int lst_tracker_free_size = (*(int*)lst_tracker_free & -8); // Size of block that lst_tracker points. In bytes.
	lst_free_next_temp = (void*)(*((int*)lst_tracker_free + 2));
	lst_free_prev_temp = (void*)(*((int*)lst_tracker_free + 3));
	lst_current = lst_tracker_free; // Gets the address of current tracking block.
	(*(int*)lst_current) = (newsize & -8) + 1; // Set the header.
	(*(int*)(lst_current + (newsize - 8))) = (newsize & -8) + 1; // Set the footer.
	(*(int*)(lst_current + newsize)) = ((lst_tracker_free_size - newsize) & -8) + 1; // Set the header of remaining block.
	(*(int*)(lst_tracker_free + lst_tracker_free_size - 8)) = ((lst_tracker_free_size - newsize) & -8) + 1; // Set the footer of remaining block.
	(*(int*)(lst_current + newsize + 8)) = (int)lst_free_next_temp; // Set the next pointer of remaining block.
	(*(int*)(lst_current + newsize + 12)) = (int)lst_free_prev_temp; // Set the prev pointer of remaining block.
	(*(int*)(lst_current - 8)) = ((*(int*)(lst_current - 8)) || 2); // Set the previous block's footer to recognize its next block(current) as allocated.
	(*(int*)(lst_current - ((*(int*)(lst_current - 8)) && -8))) = ((*(int*)(lst_current - 8)) || 2); // Set the previous block's header to recognize its next block(current) as allocated.
	
	// In this case, we do not add a new allocated block at the end of list, so we don't have to modify lst_end.
	allocated = 1; // Indicates block is allocated.
	break;
      }
    }
    if (!allocated) // Block is not allocated while tracking block list.
    {
      lst_current = mem_sbrk(newsize);
      if (lst_current == (void *)-1)
      {
	return NULL;
      }
      (*(int*)lst_current) = (newsize & -8) + 1; // Set the header.
      (*(int*)(lst_current + (newsize - 8))) = (newsize & -8) + 1; // Set the footer.
      (*(int*)(lst_current - 8)) = ((*(int*)(lst_current - 8)) || 2); // Set the previous block's footer to recognize its next block(current) as allocated.
      (*(int*)(lst_current - ((*(int*)(lst_current - 8)) && -8))) = ((*(int*)(lst_current - 8)) || 2); // Set the previous block's header to recognize its next block(current) as allocated.
      lst_end = lst_current + newsize;
      printf("mm_malloc - new allocation - lst_current : %p, lst_end : %p\n", lst_current, lst_end);
    }
  }
  void* p = lst_current+8;
  if (p == (void *)-1)
    return NULL;
  else {
    *(size_t *)p = size;
    return (void*)p; //(void *)((char *)p + SIZE_T_SIZE);
  }
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
  printf("mm_free - block size : %d\n", ((*(int*)(ptr - 8)) & -8));
  /* YOUR IMPLEMENTATION */
  ptr = ptr - 8;
  if ((((*(int*)ptr) & 3) == 3) && (((*(int*)(ptr - 8)) & 3) == 3)) // Case 1 : block next and prev are all allocated.
  {
    //printf("mm_free case1 - ptr : %p, lst_start_free : %p, lst_end_free : %p\n", ptr, lst_start_free, lst_end_free);
    (*(int*)(ptr - 8)) = ((*(int*)(ptr - 8)) & -3); // Set the footer of previous block so that it could recognize its next block(current) as freed.
    (*(int*)(ptr - ((*(int*)(ptr - 8)) & -8))) = ((*(int*)(ptr - 8)) & -3); // Set the header of previous block so that it could recognize its next block(current) as freed.
    lst_free_root_temp = lst_start_free;
    lst_start_free = ptr; // Set the freeing block as root of free list.
    *(int*)(lst_start_free + 8) = (int)lst_free_root_temp;
    *(int*)(lst_start_free + 12) = 0; // Because freed block is inserted at the root of the list, its prev block is NULL.
    (*(int*)ptr) = ((*(int*)ptr) & -2); // Set the header of freed block that it should recognized as ptr is free and its next block is allocated.
    (*(int*)(ptr + ((*(int*)ptr) & -8))) = ((*(int*)ptr) & -2); // Set the footer of freed block that it should recognized as ptr is free and its next block is allocated.
  }
  else if ((((*(int*)ptr) & 3) == 3) && (((*(int*)(ptr - 8)) & 3) == 2)) // Case 2 : block next is allocated and block prev is free.
  {
    //printf("mm_free case2 - ptr : %p, lst_start_free : %p, lst_end_free : %p\n", ptr, lst_start_free, lst_end_free);
    // In this case, previous block will be coalesced, hence setting footer of it is not needed.
    int prev_size = ((*(int*)(ptr - 8)) & -8);
    int current_size = ((*(int*)ptr) & -8);
    (*(int*)(ptr - ((*(int*)(ptr - 8)) & -8))) = ((prev_size + current_size) & -8) + 2; // Set the header of previous block so that it could recognize its next block(current's next) as allocated.
    lst_free_root_temp = lst_start_free;
    lst_start_free = (ptr - ((*(int*)(ptr - 8)) & -8));
    *(int*)(lst_start_free + 8) = (int)lst_free_root_temp;
    *(int*)(lst_start_free + 12) = 0;
    (*(int*)(ptr + ((*(int*)ptr) & -8))) = ((prev_size + current_size) & -8) + 2; // Set the footer of freed block that it should recognized as ptr is free and its next block is allocated.
  }
  else if ((((*(int*)ptr) & 3) == 1) && (((*(int*)(ptr - 8)) & 3) == 3)) // Case 3 : block next is free and block prev is allocated.
  {
    //printf("mm_free case3 - ptr : %p, lst_start_free : %p, lst_end_free : %p\n", ptr, lst_start_free, lst_end_free);
    // In this case, next block will be coalesced
    int next_size = ((*(int*)(ptr + ((*(int*)ptr) & -8))) & -8);
    int current_size = ((*(int*)ptr) & -8);
    (*(int*)(ptr - 8)) = ((*(int*)(ptr - 8)) & -3); // Set the footer of prev block so that it could recognize its next block(current) as freed.
    (*(int*)(ptr - ((*(int*)(ptr - 8)) & -8))) = ((*(int*)(ptr - 8)) & -3); // Set the header of prev block so that it could recognize its next block(current) as freed.
    lst_free_root_temp = lst_start_free;
    lst_start_free = ptr;
    *(int*)(lst_start_free + 8) = (int)lst_free_root_temp;
    *(int*)(lst_start_free + 12) = 0;
    (*(int*)ptr) = ((next_size + current_size) & -8) + ((*(int*)(ptr + current_size)) && 3); // Set the header of current block(after coalesced).
    (*(int*)(ptr + next_size + current_size - 8)) = ((next_size + current_size) & -8) + ((*(int*)(ptr + current_size)) & 3); // Set the footer of current block(after coalesced).
  }
  
  else if ((((*(int*)ptr) & 3) == 1) && (((*(int*)(ptr - 8)) & 3) == 2)) // Case 4 : block next and prev are both free.
  {
    //printf("mm_free case4 - ptr : %p, lst_start_free : %p, lst_end_free : %p\n", ptr, lst_start_free, lst_end_free);
    // In this case, prev and next block will be both coalesced.
    int prev_size = ((*(int*)(ptr - 8)) & -8);
    int current_size = ((*(int*)ptr) & -8);
    int next_size = ((*(int*)(ptr + current_size)) & -8);
    (*(int*)(ptr - prev_size)) = ((prev_size + current_size + next_size) & -8) + ((*(int*)(ptr + current_size)) & 3); // Set the header of prev block.
    (*(int*)(ptr + current_size + next_size - 8)) = ((prev_size + current_size + next_size) & -8) + ((*(int*)(ptr + current_size)) & 3); // Set the footer of next block.
    lst_free_root_temp = lst_start_free;
    lst_start_free = (ptr - (*(int*)(ptr - prev_size)));
    *(int*)(lst_start_free + 8) = (int)lst_free_root_temp;
    *(int*)(lst_start_free + 12) = 0;
  }
  else if ((((*(int*)ptr) & 3) == 1) && (((*(int*)(ptr - 8)) & 3) == 3) && (ptr == lst_end)) // Case 5 : block prev is allocated and ptr is end of list.
  {
    //printf("mm_free case5 - ptr : %p, lst_start_free : %p, lst_end_free : %p\n", ptr, lst_start_free, lst_end_free);
    int current_size = ((*(int*)ptr) & -8);
    lst_free_root_temp = lst_start_free;
    lst_start_free = ptr;
    *(int*)(lst_start_free + 8) = (int)lst_free_root_temp;
    *(int*)(lst_start_free + 12) = 0;
    (*(int*)ptr) = current_size + 0;
    (*(int*)(ptr + current_size - 8)) = current_size + 0;
  }
  else if ((((*(int*)ptr) & 3) == 1) && (((*(int*)(ptr - 8)) & 3) == 2) && (ptr == lst_end)) // Case 6 : block prev is free and ptr is end of list.
  {
    //printf("mm_free case6 - ptr : %p, lst_start_free : %p, lst_end_free : %p\n", ptr, lst_start_free, lst_end_free);
    int current_size = ((*(int*)ptr) & -8);
    int prev_size = ((*(int*)(ptr - 8)) & -8);
    lst_free_root_temp = lst_start_free;
    lst_start_free = ptr;
    *(int*)(ptr - prev_size + 8) = (int)lst_free_root_temp;
    *(int*)(ptr - prev_size + 12) = 0;
    (*(int*)(ptr - prev_size)) = (prev_size + current_size + 0);
    (*(int*)(ptr + current_size - 8)) = (prev_size + current_size + 0);
  }
  else
  {
    //printf("mm_free case7 - ptr : %p, lst_start_free : %p, lst_end_free : %p\n", ptr, lst_start_free, lst_end_free);
    int current_size = ((*(int*)ptr) & -8);
    lst_start_free = ptr;
    lst_end_free = NULL;
    *(int*)(ptr + 8) = 0;
    *(int*)(ptr + 12) = 0;
    (*(int*)ptr) = current_size + 0;
    (*(int*)(ptr + current_size - 8)) = current_size + 0;
  }
  ptr = ptr + 8;
  /* DON't MODIFY THIS STAGE AND LEAVE IT AS IT WAS */
  if (gl_ranges)
    remove_range(gl_ranges, ptr);
}

/*
 * mm_realloc - empty implementation; YOU DO NOT NEED TO IMPLEMENT THIS
 */
void* mm_realloc(void *ptr, size_t t)
{
  return NULL;
}

/*
 * mm_exit - finalize the malloc package.
 */
void mm_exit(void)
{
  printf("mm_exit entered - lst_start : %p, lst_end : %p\n", lst_start, lst_end);
  for(lst_tracker = lst_start; lst_tracker <= lst_end; lst_tracker = lst_tracker + (*(int*)lst_tracker & -8))
  {
    printf("mm_exit for loop - lst_tracker : %p, lst_start : %p, lst_end : %p\n", lst_tracker, lst_start, lst_end);
    mm_free((void*)((char*)lst_tracker + 8));
  }
  //mm_free((void*)((char*)lst_tracker + 8));
}

