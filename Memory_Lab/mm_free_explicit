void mm_free(void *ptr)
{
  // For debuging.
  //printf("FREE - %d\n", (free_counter++));
  //printf("mm_free - block size : %d\n", ((*(int*)(ptr - 8)) & -8));
  ptr = ptr - 8;
  //printf("mm_free check : current block allocated : %d, ptr == lst_start : %d, current block is not end : %d, next block is allocated : %d\n", ((*(int*)ptr & 1) == 1), (ptr == lst_start), ((ptr + (*(int*)ptr & -8)) != lst_end), (((*(int*)(ptr + (*(int*)ptr & -8))) & 1) == 1));
  //printf("mm_free check : prev block header : %d, current block header : %d\n", (*(int*)(ptr - 8) & 3), (*(int*)ptr & 3)); 
  //free_list_print(lst_start_free);
  if ((*(int*)ptr & 1) != 1)
  {
    return;
  }
  /*if (double_free_checker(ptr) == 0)
  {
	  return;
  }*/
  /* YOUR IMPLEMENTATION */
  int current_header = (*(int*)ptr) & 3;
  int prev_header = (*(int*)(ptr - 8)) & 3;
  current_block_end = ptr + (*(int*)ptr & -8);
  //if ((((*(int*)ptr) & 3) == 3) && (((*(int*)(ptr - 8)) & 3) == 3) && ((ptr + (*(int*)ptr & -8)) != lst_end) && (ptr != lst_start)) // Case 1 : block next and prev are all allocated.
  if ((current_header == 3) && (prev_header == 3) && (current_block_end != lst_end) && (ptr != lst_start))
  {
    //printf("mm_free case1 - ptr : %p, lst_start_free : %p, lst_end_free : %p\n", ptr, lst_start_free, lst_end_free);
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
  //else if ((((*(int*)ptr) & 3) == 3) && (((*(int*)(ptr - 8)) & 3) == 2) && ((ptr + (*(int*)ptr & -8)) != lst_end) && (ptr != lst_start)) // Case 2 : block next is allocated and block prev is free.
  else if ((current_header == 3) && (prev_header == 2) && (current_block_end != lst_end) && (ptr != lst_start))
  {
    //printf("mm_free case2 - ptr : %p, lst_start_free : %p, lst_end_free : %p\n", ptr, lst_start_free, lst_end_free);
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
	//printf("mm_free case2 - Debug1\n");
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
	//printf("mm_free case2 - Debug2\n");
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
    //printf("mm_free case2 - Debug3\n");
	*(int*)(lst_start_free + 12) = 0;
	/*if (lst_free_root_temp != NULL)
	{
	  *(int*)(lst_free_root_temp + 12) = (int)lst_start_free; // Set the prev pointer of next free block to point to the currently freed block.
	}*/
    (*(int*)(ptr + current_size - 8)) = ((prev_size + current_size) & -8) + 2; // Set the footer of freed block that it should recognized as ptr is free and its next block is allocated.
    //printf("mm_free case2 - FREE Block Validating - prev free block : %x, next free block : %x\n", *(int*)(lst_start_free + 12), *(int*)(lst_start_free + 8));
  }
  //else if ((((*(int*)ptr) & 3) == 1) && (((*(int*)(ptr - 8)) & 3) == 3) && ((ptr + (*(int*)ptr & -8)) != lst_end) && (ptr != lst_start)) // Case 3 : block next is free and block prev is allocated.
  else if ((current_header == 1) && (prev_header == 3) && (current_block_end != lst_end) && (ptr != lst_start))
  {
    //printf("mm_free case3 - ptr : %p, lst_start_free : %p, lst_end_free : %p\n", ptr, lst_start_free, lst_end_free);
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
	  //printf("mm_free case3 : lst_free_root_temp : %p\n", lst_free_root_temp);
      *(int*)(lst_free_root_temp + 12) = (int)lst_start_free;  // Set the prev pointer of next free block to point to currently freed block.
	}*/
    *(int*)(lst_start_free + 12) = 0;
    (*(int*)ptr) = ((next_size + current_size) & -8) + ((*(int*)(ptr + current_size)) & 3); // Set the header of current block(after coalesced).
    (*(int*)(ptr + next_size + current_size - 8)) = ((next_size + current_size) & -8) + ((*(int*)(ptr + current_size)) & 3); // Set the footer of current block(after coalesced).
    //printf("mm_free case3 - FREE Block Validating - prev free block : %x, next free block : %x\n", *(int*)(lst_start_free + 12), *(int*)(lst_start_free + 8));
	//free_list_print(lst_start_free);
  }
  
  //else if ((((*(int*)ptr) & 3) == 1) && (((*(int*)(ptr - 8)) & 3) == 2) && ((ptr + (*(int*)ptr & -8)) != lst_end) && (ptr != lst_start)) // Case 4 : block next and prev are both free.
  else if ((current_header == 1) && (prev_header == 2) && (current_block_end != lst_end) && (ptr != lst_start))
  {
    //printf("mm_free case4 - ptr : %p, lst_start_free : %p, lst_end_free : %p\n", ptr, lst_start_free, lst_end_free);
    // In this case, prev and next block will be both coalesced.
    int prev_size = ((*(int*)(ptr - 8)) & -8);
    int current_size = ((*(int*)ptr) & -8);
    int next_size = ((*(int*)(ptr + current_size)) & -8);
    //printf("mm_free case4 debug1\n");
    (*(int*)(ptr - prev_size)) = ((prev_size + current_size + next_size) & -8) + ((*(int*)(ptr + current_size)) & 3); // Set the header of prev block.
    (*(int*)(ptr + current_size + next_size - 8)) = ((prev_size + current_size + next_size) & -8) + ((*(int*)(ptr + current_size)) & 3); // Set the footer of next block.
    //printf("mm_free case4 debug2\n");
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


    //printf("mm_free case4 debug3\n");
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
    //printf("mm_free case4 - FREE Block Validating - prev free block : %x, next free block : %x\n", *(int*)(lst_start_free + 12), *(int*)(lst_start_free + 8));
  }
  //else if ((((*(int*)ptr) & 3) == 1) && (((*(int*)(ptr - 8)) & 3) == 3) && ((ptr + (*(int*)ptr & -8)) == lst_end) && (ptr != lst_start)) // Case 5 : block prev is allocated and ptr is end of list.
  else if ((current_header == 1) && (prev_header == 3) && (ptr != lst_start))
  {
    //printf("mm_free case5 - ptr : %p, lst_start_free : %p, lst_end_free : %p\n", ptr, lst_start_free, lst_end_free);
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
    //printf("Free check : %d, prev_block allocated : %d\n", (*(int*)ptr)&1, (*(int*)(ptr - prev_size) & 1));
  }
  //else if ((((*(int*)ptr) & 3) == 1) && (((*(int*)(ptr - 8)) & 3) == 2) && ((ptr + (*(int*)ptr & -8)) == lst_end) && (ptr != lst_start)) // Case 6 : block prev is free and ptr is end of list.
  else if ((current_header == 1) && (prev_header == 2) && (ptr != lst_start))
  {
    //printf("mm_free case6 - ptr : %p, lst_start_free : %p, lst_end_free : %p\n", ptr, lst_start_free, lst_end_free);
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
          //*(int*)(lst_start_free + 8) = *(int*)(*(int*)(lst_free_root_temp + 8) + 8);
		  *(int*)(lst_start_free + 8) = *(int*)(lst_free_root_temp + 8);
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
    
    //printf("mm_free case6 - FREE Block Validating - prev free block : %x, next free block : %x\n", *(int*)(lst_start_free + 12), *(int*)(lst_start_free + 8));
  }
  else if (((*(int*)ptr & 1) == 1) && (ptr == lst_start) && ((ptr + (*(int*)ptr & -8)) != lst_end) && (((*(int*)(ptr + (*(int*)ptr & -8))) & 1) == 1))  // Case 7 : ptr is start of list and next block is allocated.
  //else if ((current_header == 1) && (current_block_end != lst_end) && (ptr == lst_start) && (((*(int*)(ptr + (*(int*)ptr & -8))) & 1) == 1))
  {
    //printf("mm_free case7 - ptr : %p, lst_start_free : %p, lst_end_free : %p\n", ptr, lst_start_free, lst_end_free);
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
    //printf("mm_free case8 - ptr : %p, lst_start_free : %p, lst_end_free : %p\n", ptr, lst_start_free, lst_end_free);
    int current_size = ((*(int*)ptr) & -8);
    int next_size = ((*(int*)(ptr + current_size)) & -8);
	lst_free_root_temp = lst_start_free;
    lst_start_free = ptr;
	//printf("mm_free case8 - Debug 1\n");
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
	//printf("mm_free case8 - Debug 2\n");
	if (*(int*)(lst_start_free + current_size + 8) != 0)
	{
	  //printf("mm_free case8 - Debug 2-1\n");
      if (*(int*)(lst_start_free + current_size + 12) != 0)
	  {
		*((int*)(*(int*)(lst_start_free + current_size + 8)) + 3) = *(int*)(lst_start_free + current_size + 12);
	  }
	  else
	  {
	    //printf("mm_free case8 - Debug 2-2, Value of *(int*)(lst_start_free + current_size + 8) : %x\n", *(int*)(lst_start_free + current_size + 8));
		*((int*)(*(int*)(lst_start_free + current_size + 8)) + 3) = 0;
	  }
	}
	//printf("mm_free case8 - Debug 3\n");
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
    //printf("mm_free case8 - FREE Block Validating - prev free block : %x, next free block : %x\n", *(int*)(lst_start_free + 12), *(int*)(lst_start_free + 8));
  }
  else // Case 9 : ptr is the only block allocated.
  {
    //printf("mm_free case9 - ptr : %p, lst_start_free : %p, lst_end_Free : %p\n", ptr, lst_start_free, lst_end_free);
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
