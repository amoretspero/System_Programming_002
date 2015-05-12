void* mm_malloc(size_t size)
{
  // For debuging.
  //printf("ALLOCATION - %d\n", (allocation_counter++));
  //int newsize = ALIGN(size + SIZE_T_SIZE);
  int newsize = ALIGN(size + 2*SIZE_T_SIZE); // 8 for header and footer of memory block in heap.
  //void *p = mem_sbrk(newsize);
  if (lst_start == lst_end) // Allocating case 1 : There is no block allocated.
  {
    //printf("Allocating case 1 - block size : %d\n", newsize);
    lst_current = mem_sbrk(newsize);
    if (lst_current == (void *)-1)
    {
      return NULL;
    }
    (*(int*)lst_current) = (newsize & -8) + 1;
    (*(int*)(lst_current + (newsize - 8))) = (newsize & -8) + 1;
    lst_end = lst_current + newsize;
    lst_start = lst_current;
    //printf("mm_malloc - new allocation - lst_start : %p, lst_end : %p, allocated : %d\n", lst_start, lst_end, ((*(int*)lst_start) & 1));
  }
  else // Allocating case 2 : Allocated blocks exists. Free or allocated. Find 'free and allocatable size' block. If not exists, allocate new block at the end of list.
  {
    //int allocated = 0;
    //for (lst_tracker_free = lst_start_free; ((lst_tracker_free != NULL) && ((*(int*)(lst_tracker_free + 8)) != 0)); lst_tracker_free = ((void*)(*((int*)lst_tracker_free + 2))))
    //lst_tracker_free = lst_start_free;
    //printf("Allocating case 2 - lst_start_free : %p, lst_tracker_free : %p, lst_end_free : %p, block size - %d\n", lst_start_free, lst_tracker_free, lst_end_free, newsize);
    while((lst_tracker_free != NULL)) //&& ((*(int*)(lst_tracker_free + 8)) != 0))
  {
      //printf("In for loop - lst_start_free : %p, lst_tracker_free : %p, next free block : %x, lst_end_free : %p, free block size : %d\n", lst_start_free, lst_tracker_free, *(int*)(lst_tracker_free + 8), lst_end_free, ((*(int*)lst_tracker_free) & -8));
      //printf("allocated = %d\n", allocated);
      if (((*(int*)lst_tracker_free & -8) >= newsize + 24)) // Free and allocatable size block has been found.
      {
		//printf("debug 1\n");
		int lst_tracker_free_size = (*(int*)lst_tracker_free & -8); // Size of block that lst_tracker points. In bytes.
		int remaining_block_size = (lst_tracker_free_size - newsize) & -8;
		//printf("debug 2\n");
		lst_free_next_temp = (void*)(*((int*)lst_tracker_free + 2));
		//printf("debug 3\n");
		lst_free_prev_temp = (void*)(*((int*)lst_tracker_free + 3));
		lst_current = lst_tracker_free; // Gets the address of current tracking block.
		remaining_block_temp = lst_current + newsize;
		
		(*(int*)lst_current) = (newsize & -8) + 1; // Set the header.
		(*(int*)(lst_current + (newsize - 8))) = (newsize & -8) + 1; // Set the footer.
		
		*(int*)(remaining_block_temp) = remaining_block_size + ((*(int*)(lst_current + lst_tracker_free_size - 8)) & 3); // Set the header of remaining block.
		(*(int*)(lst_tracker_free + lst_tracker_free_size - 8)) = remaining_block_size + ((*(int*)(lst_current + lst_tracker_free_size - 8)) & 3); // Set the footer of remaining block.
		
		(*(int*)(remaining_block_temp + 8)) = (int)lst_free_next_temp; // Set the next pointer of remaining block.
		(*(int*)(remaining_block_temp + 12)) = (int)lst_free_prev_temp; // Set the prev pointer of remaining block.
		
		(*(int*)(lst_current - 8)) = ((*(int*)(lst_current - 8)) | 2); // Set the previous block's footer to recognize its next block(current) as allocated.
		(*(int*)(lst_current - ((*(int*)(lst_current - 8)) & -8))) = ((*(int*)(lst_current - 8)) | 2); // Set the previous block's header to recognize its next block(current) as allocated.
		
		// Setting free list order.
		if (lst_tracker_free == lst_start_free) // Selected free block is the first free block on list.
		{
		  //printf("lst_tracker_free = lst_start_free\n");
		  lst_start_free = (lst_current + newsize);
		  lst_tracker_free = lst_start_free;
		}
		else
		{
		  lst_tracker_free = lst_current + newsize;
		}

		if (*(int*)(lst_tracker_free + 12) != 0)
		{
		  *((int*)(*(int*)(lst_tracker_free + 12)) + 2) = (int)(lst_current + newsize);
		}
		if (*(int*)(lst_tracker_free + 8) != 0)
		{
		  *((int*)(*(int*)(lst_tracker_free + 8)) + 3) = (int)(lst_current + newsize);
		}

		// In this case, we do not add a new allocated block at the end of list, so we don't have to modify lst_end.
		allocated = 1; // Indicates block is allocated.
		//printf("Allocated using free block - lst_start_free : %p, lst_tracker_free : %p, next free block : %x, free block size : %d, allocated : %d\n", lst_start_free, lst_tracker_free, ((*(int*)(lst_tracker_free + 8))), ((*(int*)lst_tracker_free) & -8), ((*(int*)lst_current) & 1));
		break;
      }
	  /*if ((*(int*)lst_tracker_free & -8) == newsize) // Size is exactly same.
	  {
		lst_current = lst_tracker_free;
		*(int*)lst_tracker_free = ((*(int*)lst_tracker_free) | 1); // Set the header of current block.
		*(int*)(lst_tracker_free + newsize - 8) = (*(int*)lst_tracker_free); // Set the footer of current block.(Same as header)
		int prev_size = (*(int*)(lst_tracker_free - 8)) & -8;
		if (lst_tracker_free != lst_start) // If current block is not the lst_start, we have to set the prev block(neighboring)'s header and footer to recognize current block as allocated. If current block is the lst_start, we don't have to modify anything.
		{
		  *(int*)(lst_tracker_free - prev_size) = (*(int*)(lst_tracker_free - prev_size)) | 2;
		  *(int*)(lst_tracker_free - 8) = (*(int*)(lst_tracker_free - 8)) | 2;
	  	}
	  	if ((void*)(*(int*)(lst_tracker_free + 8)) != NULL) // Set the prev pointer of next free block to point to prev block of current block.
	  	{
		  *(int*)((void*)(*(int*)(lst_tracker_free + 8)) + 12) = *(int*)(lst_tracker_free + 12);
	  	}
	  	if ((void*)(*(int*)(lst_tracker_free + 12)) != NULL) // Set the next pointer of prev free block to point to next block of current block.
	  	{
		  *(int*)((void*)(*(int*)(lst_tracker_free + 12)) + 8) = *(int*)(lst_tracker_free + 8);
	  	}

	  	if (lst_current == lst_start_free) // If current block is previously the first free block, set the lst_start_free to next free block.
	  	{
		  if ((void*)(*(int*)(lst_tracker_free + 8)) != NULL)
		  {
		  	lst_start_free = (void*)(*(int*)(lst_tracker_free + 8));
		  }
		  else
		  {
		  	lst_start_free = NULL;
		  }
	  	}
		allocated = 1;
	  	//printf("Allocated using free block(Exact size) - lst_start_free : %p, located block : %p, located block size : %d, allocated : %d\n", lst_start_free, lst_tracker_free, ((*(int*)lst_tracker_free) & -8), ((*(int*)lst_current) & 1));
		lst_tracker_free = (void*)(*(int*)(lst_tracker_free + 8));
	  	break;
	  }*/
      //printf("IF in while ended, checking lst_tracker_free == lst_start_free\n");
      if (lst_tracker_free == lst_start_free)
      {
	//printf("lst_tracker_free == lst_start_free entered\n");
	if ((*(int*)(lst_tracker_free + 8)) != 0)
	{
	  //printf("lst_stracker_Free == lst_start_free IF\n");
	  lst_tracker_free = ((void*)(*((int*)lst_tracker_free + 2)));
	  //printf("lst_tracker_free == lst_start_free - new lst_tracker_free value : %p\n", lst_tracker_free);
	}
	else
	{
	  break;
	}
      }
	  else
	  {
      //printf("debug 4 - lst_start_free : %p, lst_tracker_free : %p, lst_end_free : %p, (*(int)(lst_tracker_free + 8)) : %x, free block size : %d\n", lst_start_free, lst_tracker_free, lst_end_free, (*(int*)(lst_tracker_free + 8)), (*(int*)lst_tracker_free)&-8);
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
    //printf("debug 5\n");
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
      //printf("mm_malloc - new allocation - lst_current : %p, lst_end : %p, block size : %d, allocated : %d\n", lst_current, lst_end, (*(int*)lst_current), ((*(int*)lst_current) & 1));

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
