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
void free_list_print (void* lst)
{
	if (lst == NULL)
	{
		return;
	}
	else
	{
		printf("FREE LIST PRINTER\n");
		while (lst != NULL)
		{
			printf("Address : %p, Size : %d, next free block : %p, prev free block : %p\n", lst, *(int*)lst, (void*)(*(int*)(lst + 8)), (void*)(*(int*)(lst + 12)));
			lst = (void*)(*(int*)(lst + 8));
		}
	}
}




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

  lst_start_free = NULL;
  lst_end_free = NULL;
  lst_tracker_free = NULL;
  lst_current_free = NULL;
  lst_free_next_temp = NULL;
  lst_free_prev_temp = NULL;
  lst_free_root_temp = NULL;
  lst_start = NULL;
  lst_end = NULL;
  lst_tracker = NULL;
  lst_current = NULL;

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
    printf("Allocating case 1 - block size : %d\n", newsize);
    lst_current = mem_sbrk(newsize);
    if (lst_current == (void *)-1)
    {
      return NULL;
    }
    (*(int*)lst_current) = (newsize & -8) + 1;
    (*(int*)(lst_current + (newsize - 8))) = (newsize & -8) + 1;
    lst_end = lst_current + newsize;
    lst_start = lst_current;
    printf("mm_malloc - new allocation - lst_start : %p, lst_end : %p, allocated : %d\n", lst_start, lst_end, ((*(int*)lst_start) & 1));
  }
  else // Allocating case 2 : Allocated blocks exists. Free or allocated. Find 'free and allocatable size' block. If not exists, allocate new block at the end of list.
  {
    int allocated = 0;
    //for (lst_tracker_free = lst_start_free; ((lst_tracker_free != NULL) && ((*(int*)(lst_tracker_free + 8)) != 0)); lst_tracker_free = ((void*)(*((int*)lst_tracker_free + 2))))
    lst_tracker_free = lst_start_free;
    printf("Allocating case 2 - lst_start_free : %p, lst_tracker_free : %p, lst_end_free : %p, block size - %d\n", lst_start_free, lst_tracker_free, lst_end_free, newsize);
    while((lst_tracker_free != NULL)) //&& ((*(int*)(lst_tracker_free + 8)) != 0))
    {
      printf("In for loop - lst_start_free : %p, lst_tracker_free : %p, next free block : %x, lst_end_free : %p, free block size : %d\n", lst_start_free, lst_tracker_free, *(int*)(lst_tracker_free + 8), lst_end_free, ((*(int*)lst_tracker_free) & -8));
      printf("allocated = %d\n", allocated);
      if (((*(int*)lst_tracker_free & -8) >= newsize + 24)) // Free and allocatable size block has been found.
      {
	printf("debug 1\n");
	int lst_tracker_free_size = (*(int*)lst_tracker_free & -8); // Size of block that lst_tracker points. In bytes.
	printf("debug 2\n");
	lst_free_next_temp = (void*)(*((int*)lst_tracker_free + 2));
	printf("debug 3\n");
	lst_free_prev_temp = (void*)(*((int*)lst_tracker_free + 3));
	lst_current = lst_tracker_free; // Gets the address of current tracking block.
	(*(int*)lst_current) = (newsize & -8) + 1; // Set the header.
	(*(int*)(lst_current + (newsize - 8))) = (newsize & -8) + 1; // Set the footer.
	(*(int*)(lst_current + newsize)) = ((lst_tracker_free_size - newsize) & -8) + ((*(int*)(lst_current + lst_tracker_free_size - 8)) & 3); // Set the header of remaining block.
	(*(int*)(lst_tracker_free + lst_tracker_free_size - 8)) = ((lst_tracker_free_size - newsize) & -8) + ((*(int*)(lst_current + lst_tracker_free_size - 8)) & 3); // Set the footer of remaining block.
	(*(int*)(lst_current + newsize + 8)) = (int)lst_free_next_temp; // Set the next pointer of remaining block.
	(*(int*)(lst_current + newsize + 12)) = (int)lst_free_prev_temp; // Set the prev pointer of remaining block.
	(*(int*)(lst_current - 8)) = ((*(int*)(lst_current - 8)) | 2); // Set the previous block's footer to recognize its next block(current) as allocated.
	(*(int*)(lst_current - ((*(int*)(lst_current - 8)) & -8))) = ((*(int*)(lst_current - 8)) | 2); // Set the previous block's header to recognize its next block(current) as allocated.
	// Setting free list order.
	if (lst_tracker_free == lst_start_free) // Selected free block is the first free block on list.
	{
	  printf("lst_tracker_free = lst_start_free\n");
	  lst_start_free = (lst_current + newsize);
	  lst_tracker_free = lst_start_free;
	  /*if ((*(int*)(lst_tracker_free + 8)) != 0)
	  {
	    lst_tracker_free = ((void*)(*((int*)lst_tracker_free + 2)));
	  }*/
	}
	else
	{
	  lst_tracker_free = lst_current + newsize;
	}

	if (*(int*)(lst_tracker_free + 12) != 0)
	{
	  /*if (*(int*)(lst_start_free + 8) != 0)
	  {
	    *((int*)(*(int*)(lst_start_free + 12)) + 2) = (int)(lst_current + newsize); // Set the next pointer of prev block's previous free block to prev block's next free block.
	  }
	  else
	  {
		*((int*)(*(int*)(lst_start_free + 12)) + 2) = 0;
	  }*/
	  *((int*)(*(int*)(lst_tracker_free + 12)) + 2) = (int)(lst_current + newsize);
	}
	if (*(int*)(lst_tracker_free + 8) != 0)
	{
	  /*if (*(int*)(lst_start_free + 12) != 0)
	  {
	    *((int*)(*(int*)(lst_start_free + 8)) + 3) = (int)(lst_current + newsize); // Set the prev pointer of prev block's next free block to prev block's previous free block.
	  }
	  else
	  {
		*((int*)(*(int*)(lst_start_free + 8)) + 3) = 0;
	  }*/
	  *((int*)(*(int*)(lst_tracker_free + 8)) + 3) = (int)(lst_current + newsize);
	}

	// In this case, we do not add a new allocated block at the end of list, so we don't have to modify lst_end.
	allocated = 1; // Indicates block is allocated.
	printf("Allocated using free block - lst_start_free : %p, lst_tracker_free : %p, next free block : %x, free block size : %d, allocated : %d\n", lst_start_free, lst_tracker_free, ((*(int*)(lst_tracker_free + 8))), ((*(int*)lst_tracker_free) & -8), ((*(int*)lst_current) & 1));
	break;
      }
      printf("IF in while ended, checking lst_tracker_free == lst_start_free\n");
      if (lst_tracker_free == lst_start_free)
      {
	printf("lst_tracker_free == lst_start_free entered\n");
	if ((*(int*)(lst_tracker_free + 8)) != 0)
	{
	  printf("lst_stracker_Free == lst_start_free IF\n");
	  lst_tracker_free = ((void*)(*((int*)lst_tracker_free + 2)));
	  printf("lst_tracker_free == lst_start_free - new lst_tracker_free value : %p\n", lst_tracker_free);
	}
	else
	{
	  break;
	}
      }
	  else
	  {
      printf("debug 4 - lst_start_free : %p, lst_tracker_free : %p, lst_end_free : %p, (*(int)(lst_tracker_free + 8)) : %x, free block size : %d\n", lst_start_free, lst_tracker_free, lst_end_free, (*(int*)(lst_tracker_free + 8)), (*(int*)lst_tracker_free)&-8);
      if (((*(int*)(lst_tracker_free + 8)) == 0) || ((*(int*)(lst_tracker_free + 8)) == (int)lst_start_free))
      {
	break;
      }
	  else
	  {
		lst_tracker_free = ((void*)(*((int*)lst_tracker_free + 2)));
	  }
	  }
    }
    printf("debug 5\n");
    if (!allocated) // Block is not allocated while tracking block list.
    {
      lst_current = mem_sbrk(newsize);
      if (lst_current == (void *)-1)
      {
	return NULL;
      }
      (*(int*)lst_current) = (newsize & -8) + 1; // Set the header.
      (*(int*)(lst_current + (newsize - 8))) = (newsize & -8) + 1; // Set the footer.
      (*(int*)(lst_current - 8)) = ((*(int*)(lst_current - 8)) | 2); // Set the previous block's footer to recognize its next block(current) as allocated.
      (*(int*)(lst_current - ((*(int*)(lst_current - 8)) & -8))) = ((*(int*)(lst_current - 8)) | 2); // Set the previous block's header to recognize its next block(current) as allocated.
      lst_end = lst_current + newsize;
      printf("mm_malloc - new allocation - lst_current : %p, lst_end : %p, block size : %d, allocated : %d\n", lst_current, lst_end, (*(int*)lst_current), ((*(int*)lst_current) & 1));

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
  ptr = ptr - 8;
  printf("mm_free check : current block allocated : %d, ptr == lst_start : %d, current block is not end : %d, next block is allocated : %d\n", ((*(int*)ptr & 1) == 1), (ptr == lst_start), ((ptr + (*(int*)ptr & -8)) != lst_end), (((*(int*)(ptr + (*(int*)ptr & -8))) & 1) == 1));
  free_list_print(lst_start_free);
  /* YOUR IMPLEMENTATION */
  if ((((*(int*)ptr) & 3) == 3) && (((*(int*)(ptr - 8)) & 3) == 3) && ((ptr + (*(int*)ptr & -8)) != lst_end) && (ptr != lst_start)) // Case 1 : block next and prev are all allocated.
  {
    printf("mm_free case1 - ptr : %p, lst_start_free : %p, lst_end_free : %p\n", ptr, lst_start_free, lst_end_free);
    int prev_size = ((*(int*)(ptr - 8)) & -8);
    int current_size = ((*(int*)ptr) & -8);
    (*(int*)(ptr - 8)) = ((*(int*)(ptr - 8)) & -3); // Set the footer of previous block so that it could recognize its next block(current) as freed.
    (*(int*)(ptr - prev_size)) = ((*(int*)(ptr - 8)) & -3); // Set the header of previous block so that it could recognize its next block(current) as freed.
    lst_free_root_temp = lst_start_free;
    lst_start_free = ptr; // Set the freeing block as root of free list.
    *(int*)(lst_start_free + 8) = (int)lst_free_root_temp;
    *(int*)(lst_start_free + 12) = 0; // Because freed block is inserted at the root of the list, its prev block is NULL.
	if (lst_free_root_temp != NULL)
	{
	  (*(int*)(lst_free_root_temp + 12)) = (int)lst_start_free; // Set the previous pointer of next free block to point to currently freed block.
	}
    (*(int*)ptr) = ((*(int*)ptr) & -2); // Set the header of freed block that it should recognized as ptr is free and its next block is allocated.
    (*(int*)(ptr + current_size - 8)) = ((*(int*)ptr) & -2); // Set the footer of freed block that it should recognized as ptr is free and its next block is allocated.
  }
  else if ((((*(int*)ptr) & 3) == 3) && (((*(int*)(ptr - 8)) & 3) == 2) && ((ptr + (*(int*)ptr & -8)) != lst_end) && (ptr != lst_start)) // Case 2 : block next is allocated and block prev is free.
  {
    printf("mm_free case2 - ptr : %p, lst_start_free : %p, lst_end_free : %p\n", ptr, lst_start_free, lst_end_free);
    // In this case, previous block will be coalesced, hence setting footer of it is not needed.
    int prev_size = ((*(int*)(ptr - 8)) & -8);
    int current_size = ((*(int*)ptr) & -8);
    (*(int*)(ptr - prev_size)) = ((prev_size + current_size) & -8) + 2; // Set the header of previous block so that it could recognize its next block(current's next) as allocated.
    lst_free_root_temp = lst_start_free;
    lst_start_free = ptr - prev_size;
	if (*(int*)(lst_start_free + 12) != 0)
	{
	  if (*(int*)(lst_start_free + 8) != 0)
	  {
	    *((int*)(*(int*)(lst_start_free + 12)) + 2) = *(int*)(lst_start_free + 8); // Set the next pointer of prev block's previous free block to prev block's next free block.
	  }
	  else
	  {
		*((int*)(*(int*)(lst_start_free + 12)) + 2) = 0;
	  }
	}
	printf("mm_free case2 - Debug1\n");
	if (*(int*)(lst_start_free + 8) != 0)
	{
	  if (*(int*)(lst_start_free + 12) != 0)
	  {
	    *((int*)(*(int*)(lst_start_free + 8)) + 3) = *(int*)(lst_start_free + 12); // Set the prev pointer of prev block's next free block to prev block's previous free block.
	  }
	  else
	  {
		*((int*)(*(int*)(lst_start_free + 8)) + 3) = 0;
	  }
	}
	printf("mm_free case2 - Debug2\n");
	if (lst_free_root_temp >= ptr - prev_size && lst_free_root_temp <= ptr) // In this case, since the prev pointer of lst_free_root_temp's next free block(if exists) is already pointing at current(merged) block, we don't have to modify it.
	{
	  /*if (*(int*)(lst_free_root_temp + 8) != 0)
	  {
		*(int*)(lst_start_free + 8) = *((int*)((void*)(*(int*)(lst_free_root_temp + 8)) + 8));
	  }
	  else
	  {
		*(int*)(lst_start_free + 8) = *(int*)(lst_start_free + 8);
	  }*/
	  *(int*)(lst_start_free + 8) = *(int*)(lst_start_free + 8);
	  if ((void*)(*(int*)(lst_free_root_temp + 8)) != NULL)
	  {
	    *(int*)((void*)(*(int*)(lst_free_root_temp + 8)) + 12) = (int)lst_start_free;
	  }
	}
	else
	{
      *(int*)(lst_start_free + 8) = (int)lst_free_root_temp;
	  if (lst_free_root_temp != NULL)
	  {
		*(int*)(lst_free_root_temp + 12) = (int)lst_start_free; // Set the prev pointer of next free block to point to the currently freed block.
	  }
	}
    printf("mm_free case2 - Debug3\n");
	*(int*)(lst_start_free + 12) = 0;
	/*if (lst_free_root_temp != NULL)
	{
	  *(int*)(lst_free_root_temp + 12) = (int)lst_start_free; // Set the prev pointer of next free block to point to the currently freed block.
	}*/
    (*(int*)(ptr + current_size - 8)) = ((prev_size + current_size) & -8) + 2; // Set the footer of freed block that it should recognized as ptr is free and its next block is allocated.
    printf("mm_free case2 - FREE Block Validating - prev free block : %x, next free block : %x\n", *(int*)(lst_start_free + 12), *(int*)(lst_start_free + 8));
  }
  else if ((((*(int*)ptr) & 3) == 1) && (((*(int*)(ptr - 8)) & 3) == 3) && ((ptr + (*(int*)ptr & -8)) != lst_end) && (ptr != lst_start)) // Case 3 : block next is free and block prev is allocated.
  {
    printf("mm_free case3 - ptr : %p, lst_start_free : %p, lst_end_free : %p\n", ptr, lst_start_free, lst_end_free);
    // In this case, next block will be coalesced
    int next_size = ((*(int*)(ptr + ((*(int*)ptr) & -8))) & -8);
    int current_size = ((*(int*)ptr) & -8);
    (*(int*)(ptr - 8)) = ((*(int*)(ptr - 8)) & -3); // Set the footer of prev block so that it could recognize its next block(current) as freed.
    (*(int*)(ptr - ((*(int*)(ptr - 8)) & -8))) = ((*(int*)(ptr - 8)) & -3); // Set the header of prev block so that it could recognize its next block(current) as freed.
    lst_free_root_temp = lst_start_free;
    lst_start_free = ptr;
	if (*(int*)(lst_start_free + current_size + 12) != 0)
	{
	  if (*(int*)(lst_start_free + current_size + 8) != 0)
	  {
		*((int*)(*(int*)(lst_start_free + current_size + 12)) + 2) = *(int*)(lst_start_free + current_size + 8);
	  }
	  else
	  {
		*((int*)(*(int*)(lst_start_free + current_size + 12)) + 2) = 0;
	  }
	}
	if (*(int*)(lst_start_free + current_size + 8) != 0)
	{
      if (*(int*)(lst_start_free + current_size + 12) != 0)
	  {
		*((int*)(*(int*)(lst_start_free + current_size + 8)) + 3) = *(int*)(lst_start_free + current_size + 12);
	  }
	  else
	  {
		*((int*)(*(int*)(lst_start_free + current_size + 8)) + 3) = 0;
	  }
	}
	if (lst_free_root_temp >= ptr && lst_free_root_temp <= ptr + current_size + next_size) // When the next block is the first free block in the list.
	{
	  *(int*)(lst_start_free + 8) = *(int*)(ptr + current_size + 8);
	  if (*(int*)(lst_free_root_temp + 8) != 0) // In this case, we have to check next block's next block is not NULL, and if not, set the prev pointer of that block to point to current block.
	  {
		*(int*)((void*)(*(int*)(lst_free_root_temp + 8)) + 12) = (int)lst_start_free;
	  }
	}
	else // When the next block is not the first free block on list.
	{
	  *(int*)(lst_start_free + 8) = (int)lst_free_root_temp;
	  if (lst_free_root_temp != NULL)
	  {
		*(int*)(lst_free_root_temp + 12) = (int)lst_start_free;
	  }
	}
	/*if (lst_free_root_temp != NULL)
	{
	  printf("mm_free case3 : lst_free_root_temp : %p\n", lst_free_root_temp);
      *(int*)(lst_free_root_temp + 12) = (int)lst_start_free;  // Set the prev pointer of next free block to point to currently freed block.
	}*/
    *(int*)(lst_start_free + 12) = 0;
    (*(int*)ptr) = ((next_size + current_size) & -8) + ((*(int*)(ptr + current_size)) & 3); // Set the header of current block(after coalesced).
    (*(int*)(ptr + next_size + current_size - 8)) = ((next_size + current_size) & -8) + ((*(int*)(ptr + current_size)) & 3); // Set the footer of current block(after coalesced).
    printf("mm_free case3 - FREE Block Validating - prev free block : %x, next free block : %x\n", *(int*)(lst_start_free + 12), *(int*)(lst_start_free + 8));
	free_list_print(lst_start_free);
  }
  
  else if ((((*(int*)ptr) & 3) == 1) && (((*(int*)(ptr - 8)) & 3) == 2) && ((ptr + (*(int*)ptr & -8)) != lst_end) && (ptr != lst_start)) // Case 4 : block next and prev are both free.
  {
    printf("mm_free case4 - ptr : %p, lst_start_free : %p, lst_end_free : %p\n", ptr, lst_start_free, lst_end_free);
    // In this case, prev and next block will be both coalesced.
    int prev_size = ((*(int*)(ptr - 8)) & -8);
    int current_size = ((*(int*)ptr) & -8);
    int next_size = ((*(int*)(ptr + current_size)) & -8);
    printf("mm_free case4 debug1\n");
    (*(int*)(ptr - prev_size)) = ((prev_size + current_size + next_size) & -8) + ((*(int*)(ptr + current_size)) & 3); // Set the header of prev block.
    (*(int*)(ptr + current_size + next_size - 8)) = ((prev_size + current_size + next_size) & -8) + ((*(int*)(ptr + current_size)) & 3); // Set the footer of next block.
    printf("mm_free case4 debug2\n");
    lst_free_root_temp = lst_start_free;
    //lst_start_free = (ptr - (*(int*)(ptr - prev_size)));
    lst_start_free = (ptr - prev_size);
    if (*(int*)(lst_start_free + 12) != 0)
	{
	  if (*(int*)(lst_start_free + 8) != 0)
	  {
	    *((int*)(*(int*)(lst_start_free + 12)) + 2) = *(int*)(lst_start_free + 8); // Set the next pointer of prev block's previous free block to prev block's next free block.
	  }
	  else
	  {
		*((int*)(*(int*)(lst_start_free + 12)) + 2) = 0;
	  }
	}
	if (*(int*)(lst_start_free + 8) != 0)
	{
	  if (*(int*)(lst_start_free + 12) != 0)
	  {
	    *((int*)(*(int*)(lst_start_free + 8)) + 3) = *(int*)(lst_start_free + 12); // Set the prev pointer of prev block's next free block to prev block's previous free block.
	  }
	  else
	  {
		*((int*)(*(int*)(lst_start_free + 8)) + 3) = 0;
	  }
	}

    if (*(int*)(ptr + current_size + 12) != 0)
	{
	  if (*(int*)(ptr + current_size + 8) != 0)
	  {
		*((int*)(*(int*)(ptr + current_size + 12)) + 2) = *(int*)(ptr + current_size + 8);
	  }
	  else
	  {
		*((int*)(*(int*)(ptr + current_size + 12)) + 2) = 0;
	  }
	}
	if (*(int*)(ptr + current_size + 8) != 0)
	{
      if (*(int*)(ptr + current_size + 12) != 0)
	  {
		*((int*)(*(int*)(ptr + current_size + 8)) + 3) = *(int*)(ptr + current_size + 12);
	  }
	  else
	  {
		*((int*)(*(int*)(ptr + current_size + 8)) + 3) = 0;
	  }
	}


    printf("mm_free case4 debug3\n");
	/*if ((lst_start_free >= ptr && lst_start_free <= ptr + current_size + next_size) || (lst_start_free >= ptr - prev_size && lst_start_free <= ptr + current_size))
	{
	  *(int*)(lst_start_free + 8) = */
	/*if (lst_free_root_temp != NULL)
	{
      *(int*)(lst_free_root_temp + 12) = (int)lst_start_free;
	}*/

	if (lst_free_root_temp >= ptr && lst_free_root_temp <= ptr + current_size + next_size) // When lst_free_root_temp is third block merged.
	{
	  int temp_value = *(int*)(ptr + current_size + 8);
	  if ((void*)temp_value >= ptr - prev_size && (void*)temp_value <= ptr) // When lst_free_root_temp is third block but, its next is first block.
	  {
		*(int*)(lst_start_free + 8) = *(int*)(ptr - prev_size + 8);
	  }
	  else
	  {
	    *(int*)(lst_start_free + 8) = *(int*)(ptr + current_size + 8);
	  }
	  
	  if (*(int*)(lst_free_root_temp + 8) != 0) // In this case, lst_free_root_temp is third block, we set the prev pointer of that block's next block to point current(merged) block.
	  {
		*(int*)((void*)(*(int*)(lst_free_root_temp + 8)) + 12) = (int)lst_start_free;
	  }
	}
	else if (lst_free_root_temp >= ptr - prev_size && lst_free_root_temp <= ptr) // when lst_free_root_temp is first block merged.
	{
	  int temp_value = *(int*)(ptr - prev_size + 8);
	  if ((void*)temp_value >= ptr && (void*)temp_value <= ptr + current_size + next_size) // When lst_free_root_temp is first block but, its next is third block.
	  {
		*(int*)(lst_start_free + 8) = *(int*)((void*)temp_value + 8);
	  }
	  else
	  {
	  	*(int*)(lst_start_free + 8) = *(int*)(ptr - prev_size + 8);
	  }

	  if (*(int*)(lst_free_root_temp + 8) != 0) // In this case, lst_free_root_temp is first block, we set the prev pointer of that block's next block to point current(merged) block.
	  {
		*(int*)((void*)(*(int*)(lst_free_root_temp + 8)) + 12) = (int)lst_start_free;
	  }
	}
	else
	{
	  *(int*)(lst_start_free + 8) = (int)lst_free_root_temp;
	  if (lst_free_root_temp != NULL)
	  {
		*(int*)(lst_free_root_temp + 12) = (int)lst_start_free;
	  }
	}
    *(int*)(lst_start_free + 12) = 0;
    printf("mm_free case4 - FREE Block Validating - prev free block : %x, next free block : %x\n", *(int*)(lst_start_free + 12), *(int*)(lst_start_free + 8));
  }
  else if ((((*(int*)ptr) & 3) == 1) && (((*(int*)(ptr - 8)) & 3) == 3) && ((ptr + (*(int*)ptr & -8)) == lst_end) && (ptr != lst_start)) // Case 5 : block prev is allocated and ptr is end of list.
  {
    printf("mm_free case5 - ptr : %p, lst_start_free : %p, lst_end_free : %p\n", ptr, lst_start_free, lst_end_free);
    int current_size = ((*(int*)ptr) & -8);
    int prev_size = ((*(int*)(ptr - 8)) & -8);
    lst_free_root_temp = lst_start_free;
    lst_start_free = ptr;
    *(int*)(lst_start_free + 8) = (int)lst_free_root_temp;
    *(int*)(lst_start_free + 12) = 0;
    (*(int*)ptr) = current_size + 0;
    (*(int*)(ptr + current_size - 8)) = current_size + 0;
    (*(int*)(ptr - prev_size)) = ((*(int*)(ptr - prev_size)) & -3);
    (*(int*)(ptr - 8)) = ((*(int*)(ptr - 8)) & -3);
	if (lst_free_root_temp != NULL)
	{
	  *(int*)(lst_free_root_temp + 12) = (int)lst_start_free;
	}
    printf("Free check : %d, prev_block allocated : %d\n", (*(int*)ptr)&1, (*(int*)(ptr - prev_size) & 1));
  }
  else if ((((*(int*)ptr) & 3) == 1) && (((*(int*)(ptr - 8)) & 3) == 2) && ((ptr + (*(int*)ptr & -8)) == lst_end) && (ptr != lst_start)) // Case 6 : block prev is free and ptr is end of list.
  {
    printf("mm_free case6 - ptr : %p, lst_start_free : %p, lst_end_free : %p\n", ptr, lst_start_free, lst_end_free);
    int current_size = ((*(int*)ptr) & -8);
    int prev_size = ((*(int*)(ptr - 8)) & -8);
    lst_free_root_temp = lst_start_free;
    lst_start_free = ptr - prev_size;
    if (*(int*)(lst_start_free + 12) != 0)
	{
	  if (*(int*)(lst_start_free + 8) != 0)
	  {
	    *((int*)(*(int*)(lst_start_free + 12)) + 2) = *(int*)(lst_start_free + 8); // Set the next pointer of prev block's previous free block to prev block's next free block.
	  }
	  else
	  {
		*((int*)(*(int*)(lst_start_free + 12)) + 2) = 0;
	  }
	}
	if (*(int*)(lst_start_free + 8) != 0)
	{
	  if (*(int*)(lst_start_free + 12) != 0)
	  {
	    *((int*)(*(int*)(lst_start_free + 8)) + 3) = *(int*)(lst_start_free + 12); // Set the prev pointer of prev block's next free block to prev block's previous free block.
	  }
	  else
	  {
		*((int*)(*(int*)(lst_start_free + 8)) + 3) = 0;
	  }
	}
    
    (*(int*)(ptr - prev_size)) = (prev_size + current_size + 0);
    (*(int*)(ptr + current_size - 8)) = (prev_size + current_size + 0);
    /*if (lst_free_root_temp >= ptr - prev_size && lst_free_root_temp <= ptr)
    {
      *(int*)(lst_start_free + 8) = *(int*)(lst_start_free + 8);
      if ()
    }*/
    
    if (lst_free_root_temp != NULL)
    {
      if (lst_free_root_temp >= ptr - prev_size && lst_free_root_temp <= ptr) // In this case, prev block is lst_free_root_temp.
      {
        if ((void*)(*(int*)(lst_free_root_temp + 8)) != NULL) // If next pointer of prev block is not null, set the current(merged) block's next pointer to that value. Since that next free block's prev pointer is already pointing to prev block, we don't have to modify it.
        {
          *(int*)(lst_start_free + 8) = *(int*)(*(int*)(lst_free_root_temp + 8) + 8);
		  *(int*)((void*)(*(int*)(lst_free_root_temp + 8)) + 12) = (int)lst_start_free;
        }
        else // Next pointer of prev block is null.
        {
          *(int*)(lst_start_free + 8) = 0;
        }
      }
      else // In this case, prev block is not lst_free_root_temp;
      {
        *(int*)(lst_start_free + 8) = (int)lst_free_root_temp; // Set the next pointer of current(merged) block.
        *(int*)(lst_free_root_temp + 12) = (int)lst_start_free; // Set the prev pointer of lst_free_root_temp to current(merged) block.
      }
    }
    *(int*)(lst_start_free + 12) = 0; // Set the prev pointer of current(merged) block to NULL.
    
    printf("mm_free case6 - FREE Block Validating - prev free block : %x, next free block : %x\n", *(int*)(lst_start_free + 12), *(int*)(lst_start_free + 8));
  }
  else if (((*(int*)ptr & 1) == 1) && (ptr == lst_start) && ((ptr + (*(int*)ptr & -8)) != lst_end) && (((*(int*)(ptr + (*(int*)ptr & -8))) & 1) == 1))  // Case 7 : ptr is start of list and next block is allocated.
  {
    printf("mm_free case7 - ptr : %p, lst_start_free : %p, lst_end_free : %p\n", ptr, lst_start_free, lst_end_free);
    int current_size = ((*(int*)ptr) & -8);
    lst_free_root_temp = lst_start_free;
    lst_start_free = ptr;
    lst_end_free = NULL;
    *(int*)(ptr + 8) = (int)lst_free_root_temp;
    *(int*)(ptr + 12) = 0;
	if (lst_free_root_temp != NULL)
	{
	  *(int*)(lst_free_root_temp + 12) = (int)lst_start_free;
	}
    (*(int*)ptr) = current_size + 2;
    (*(int*)(ptr + current_size - 8)) = current_size + 2;
  }
  else if (((*(int*)ptr & 3) == 1) && (ptr == lst_start) && ((ptr + (*(int*)ptr & -8)) != lst_end) && (((*(int*)(ptr + (*(int*)ptr & -8))) & 1) == 0)) // Case 8 :ptr is start of list and next block is free.
  {
    printf("mm_free case8 - ptr : %p, lst_start_free : %p, lst_end_free : %p\n", ptr, lst_start_free, lst_end_free);
    int current_size = ((*(int*)ptr) & -8);
    int next_size = ((*(int*)(ptr + current_size)) & -8);
	lst_free_root_temp = lst_start_free;
    lst_start_free = ptr;
	printf("mm_free case8 - Debug 1\n");
	if (*(int*)(lst_start_free + current_size + 12) != 0)
	{
	  if (*(int*)(lst_start_free + current_size + 8) != 0)
	  {
		*((int*)(*(int*)(lst_start_free + current_size + 12)) + 2) = *(int*)(lst_start_free + current_size + 8);
	  }
	  else
	  {
		*((int*)(*(int*)(lst_start_free + current_size + 12)) + 2) = 0;
	  }
	}
	printf("mm_free case8 - Debug 2\n");
	if (*(int*)(lst_start_free + current_size + 8) != 0)
	{
	  printf("mm_free case8 - Debug 2-1\n");
      if (*(int*)(lst_start_free + current_size + 12) != 0)
	  {
		*((int*)(*(int*)(lst_start_free + current_size + 8)) + 3) = *(int*)(lst_start_free + current_size + 12);
	  }
	  else
	  {
	    printf("mm_free case8 - Debug 2-2, Value of *(int*)(lst_start_free + current_size + 8) : %x\n", *(int*)(lst_start_free + current_size + 8));
		*((int*)(*(int*)(lst_start_free + current_size + 8)) + 3) = 0;
	  }
	}
	printf("mm_free case8 - Debug 3\n");
    *(int*)ptr = (current_size + next_size) + (*(int*)(ptr + current_size) & 2); // Set the header of coalesced block.
    *(int*)(ptr + current_size + next_size - 8) = (current_size + next_size) + (*(int*)(ptr + current_size) & 2); // Set the footer of coalesced block.
	if (lst_free_root_temp >= ptr && lst_free_root_temp <= ptr + current_size + next_size) // When the next block(free) is the starting free block.
	{
	  *(int*)(ptr + 8) = *(int*)(ptr + current_size + 8);
	  if (*(int*)(lst_free_root_temp + 8) != 0) // In this case, set the prev pointer of next block's next block to point to lst_start_free
	  {
		*(int*)((void*)(*(int*)(lst_free_root_temp + 8)) + 12) = (int)lst_start_free;
	  }
	}
	else // When the next block(free) is not the starting free block.
	{
	  *(int*)(ptr + 8) = (int)lst_free_root_temp;
	  if (lst_free_root_temp != NULL)
	  {
		*(int*)(lst_free_root_temp + 12) = (int)lst_start_free;
	  }
	}
	/*if (lst_free_root_temp != NULL)
	{
	  *(int*)(lst_free_root_temp + 12) = (int)lst_start_free;
	}*/
    *(int*)(ptr + 12) = 0;
    printf("mm_free case8 - FREE Block Validating - prev free block : %x, next free block : %x\n", *(int*)(lst_start_free + 12), *(int*)(lst_start_free + 8));
  }
  else // Case 9 : ptr is the only block allocated.
  {
    printf("mm_free case9 - ptr : %p, lst_start_free : %p, lst_end_Free : %p\n", ptr, lst_start_free, lst_end_free);
    int current_size = ((*(int*)ptr) & -8);
    lst_start_free = ptr;
    *(int*)ptr = current_size + 0;
    *(int*)(ptr + current_size - 8) = current_size + 0;
    *(int*)(ptr + 8) = 0;
    *(int*)(ptr + 12) = 0;
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
  for(lst_tracker = lst_start; lst_tracker < lst_end; lst_tracker = lst_tracker + (*(int*)lst_tracker & -8))
  {
    printf("mm_exit for loop - lst_tracker : %p, lst_start : %p, lst_end : %p, current block allocated? : %d\n", lst_tracker, lst_start, lst_end, ((*(int*)lst_tracker & 1)));
    printf("First free list freed?(0 : freed) : %d, free block size : %d\n", (*(int*)lst_start_free & 1), (*(int*)lst_start_free & -8));
    if ((*(int*)lst_tracker & 1) == 1)
    {
      printf("mm_exit - now freeing the unfreed block - lst_tracker : %p, block size : %d\n", lst_tracker, (*(int*)lst_tracker & -8));
      mm_free((void*)((char*)lst_tracker + 8));
    }
  }
  lst_start_free = NULL;
  lst_end_free = NULL;
  lst_tracker_free = NULL;
  lst_current_free = NULL;
  lst_free_next_temp = NULL;
  lst_free_prev_temp = NULL;
  lst_free_root_temp = NULL;
  lst_start = NULL;
  lst_end = NULL;
  lst_tracker = NULL;
  lst_current = NULL;
}

