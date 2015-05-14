/*
 * mm.c - With Red Black tree implementaion for free blocks and prediction algorithm for utilization improvement.
 *
 * In this approach, free blocks are maintained by RB tree and prediction algorithm is used.
 *
 * ========== Free block management - RED BLACK TREE ==========
 * RB tree's characteristics make finding best blocks very fast(time complexity of O(log n)).
 * And, For coalescing of free blocks, RB tree's insert and delete function
 * makes it not much complex. (Insert and Delete takes O(log n) also.)
 * Just see previous and next block, if coalescing is needed,
 * delete block(s) that is going to be coalesced and coalesce and insert the coalesced block.
 *
 * Finding best block - find_block(int size, void* tree)
 * Insertion - insert(void* ptr, void* tree)
 * Deletion - delete(void* ptr)
 * Check of Red Black characteristics - tree_checker();
 * ============================================================
 *
 * ========== Prediction Algorithm ==========
 * If same size of blocks are continuously causes new allocation,
 * It would increase utilization of memory if we allocate block of that
 * size in advance.
 * Let's assume block size of m repeatedly requires to be allocated.
 * Than prediction algorithm will allocated that block in advance with some interval.
 * Even if there would be block size of n to be allocated, (which would have some difference with m)
 * that block can find more fitting block by find_block() or if n > m, newly be allocated.
 * This will make utilization of memory better if repeatedly allocated block will be freed repeatedly,
 * which has reasonable probability in reality.
 *
 * heuristic_size : size that take every block newly allocated with mem_sbrk() with weight of (heuristic_weight)/1
 * alpha : ratio of previous heuristic_size and newly calculated heuristic_size multiplied by weight of (new_alpha_weight)/1 
 * 	   plused by previous alpha with weight of (heuristic_weight_alpha)/1
 * heuristic_pvalue : limit of abs_diff_alpha which is absolute value of alpha. If abs_diff_alpha < heuristic_pvalue and alloc_num condition meets,
 *  		      heuristic_allocation is performed.
 * calculate_heuristic : perform needed calculations to predict next block.
 * heuristic_allocation : allocate new free block who has size of repeatedly call calculate_heuristic(), and put it into RB tree.
 * 
 * Formula for heuristic_size : heuristic_size = (heuristic_size * heuristic_weight) + (size * new_weight)
 * Formula for alpha : alpha = (alpha * heuristic_weight_alpha) + ((heuristic_size(current value) / heuristic_size(previous value)) * new_weight_alpha)
 * Condition for allocation in advance : 1) abs_diff_alpha < heuristic_pvalue, 2) alloc_num == alloc_num_min
 * ==========================================
 *
 * ========== Some effect of prediction algorithm ==========
 * Without prediction algorithm, binary-bal.rep and binary2-bal.rep has very low utilization(53%, 47%).
 * With prediction algorith, utilization of binary-bal.rep and binary2-bal.rep has been increased significantly(83%, 72%), without hurting other test files.
 * (Some files except above two files, they have shown little increase of utilization.)
 * =========================================================
 * 
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

void* lst_start;
void* lst_end;
void* lst_tracker; // Tracker for searching block list from start to end, finding allocatable free block.
void* lst_current;
void* lst_start_free;
void* lst_free_next_temp;
void* lst_free_prev_temp;
void* lst_current_free;
void* lst_end_free;
void* lst_tracker_free;
void* lst_free_root_temp;
void* remaining_block_temp;
void* current_block_end;

int allocation_counter;
int free_counter;

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
void list_print (void* lst)
{
    if (lst == NULL)
    {
	return;
    }
    else
    {
	while (lst < lst_end)
	{
	    printf("Address : %p, size : %d, Allocated? %d\n", lst, (*(int*)lst & -8), (*(int*)lst & 1));
	    lst = lst + (*(int*)lst & -8);
	}
    }
}
// Red Black Tree Implementation : HEADER(4)-LEFT_CHILD(4)-PARENT(4)-...-FOOTER(4)-RIGHT_CHILD(4) : Minimum size : 24
int get_size(void* ptr);
void inorder_traverse(void* ptr);
void* parent (void* ptr)
{
	if (ptr != NULL)
	{
		return ((void*)(*(int*)(ptr + 8)));
	}
	else
	{
		return NULL;
	}
}
void* left_child(void* ptr)
{
	if (ptr != NULL)
	{
		return ((void*)(*(int*)(ptr + 4)));
	}
	else
	{
		return NULL;
	}
}
void* right_child(void* ptr)
{
	if (ptr != NULL)
	{
		return ((void*)(*(int*)(ptr + (*(int*)ptr & -8) - 4)));
	}
	else
	{
		return NULL;
	}
}
void* grandparent (void* ptr)
{
	if (*(int*)(ptr + 8) != 0)
	{
		return parent(parent(ptr));
	}
	else
	{
		return NULL;
	}
}
void* gp_temp_for_uncle;
void* uncle (void* ptr)
{
	gp_temp_for_uncle = grandparent(ptr);
	if (gp_temp_for_uncle == NULL)
	{
		return NULL;
	}
	if ((parent(ptr)) == (left_child(gp_temp_for_uncle)))
	{
		return (right_child(gp_temp_for_uncle));
	}
	else
	{
		return (left_child(gp_temp_for_uncle));
	}
}
void color_red (void* ptr) // Color bit is 1 for RED
{
	if (ptr != NULL)
	{
		*(int*)ptr = (*(int*)ptr) | 4;
		*(int*)(ptr + (get_size(ptr)) - 8) = (*(int*)(ptr + (get_size(ptr)) - 8)) | 4;
	}
}
void color_black (void* ptr) // Color bit is 0 for BLACK
{
	if (ptr != NULL)
	{
		*(int*)ptr = (*(int*)ptr) & -5;
		*(int*)(ptr + (get_size(ptr)) - 8) = (*(int*)(ptr + (get_size(ptr)) - 8)) & -5;
	}
}
int get_color(void* ptr)
{
	if (ptr != NULL)
	{
		return ((*(int*)ptr & 4) >> 2);
	}
	else
	{
		return -1;
	}
}
int get_size(void* ptr)
{
	if (ptr != NULL)
	{
		return (*(int*)ptr & -8);
	}
	else	
	{
		return 0;
	}
}
void insert_case1 (void* ptr);
void insert_case2 (void* ptr);
void insert_case3 (void* ptr);
void insert_case4 (void* ptr);
void insert_case5 (void* ptr);

void* tree_root = NULL;
void insert_bst(void* ptr, void* tree);
void insert(void* ptr, void* tree)
{
    insert_bst(ptr, tree);
    insert_case1(ptr);
}
void insert_bst(void* ptr, void* tree)
{
	if (ptr != NULL)
	{
		if (tree == NULL) // Case 1 : Tree is not constructed.
		{
			tree = ptr;
			tree_root = ptr;
			*(int*)ptr = (*(int*)ptr) | 0; // Set the color to black.
			*(int*)(ptr + 4) = 0;
			*(int*)(ptr + 8) = 0;
			*(int*)(ptr + (*(int*)ptr & -8) - 4) = 0;
		}
		else // Case 2 : Tree exists.
		{
			if ((get_size(ptr)) >= (get_size(tree))) // Case 2-1 : Size of new block is greater or equal to current node. - Insert to right side.
			{
				if (right_child(tree) != NULL) // Case 2-1-1 : Right child exists.
				{
					insert_bst(ptr, (right_child(tree)));
				}
				else // Case 2-1-2 : Right child don't exists.
				{
					*(int*)ptr = (*(int*)ptr) | 4; // Set the color to red.
					*(int*)(ptr + 8) = (int)tree;
					*(int*)(ptr + 4) = 0;
					*(int*)(ptr + (get_size(ptr)) - 4) = 0;
					*(int*)(tree + (get_size(tree)) - 4) = (int)ptr;
				}
			}
			else // Case 2-2 : Size of new block is less than current node. - Insert to left side.
			{
				if (left_child(tree) != NULL) // Case 2-2-1 : Left child exists.
				{
					insert_bst(ptr, (left_child(tree)));
				}
				else // Case 2-2-2 : Left child don't exists.
				{
					*(int*)ptr = (*(int*)ptr) | 4; // Set the color to red.
					*(int*)(ptr + 8) = (int)tree;
					*(int*)(ptr + 4) = 0;
					*(int*)(ptr + (get_size(ptr)) - 4) = 0;
					*(int*)(tree + 4) = (int)ptr;
				}
			}
		}
	}
}

void insert_case1 (void* ptr)
{
	if ((parent(ptr)) == NULL)
	{
		color_black(ptr);
	}
	else
	{
		insert_case2(ptr);
	}
}
void insert_case2 (void* ptr)
{
	if (get_color(parent(ptr)) == 0)
	{
		return;
	}
	else
	{
		insert_case3(ptr);
	}
}
void insert_case3 (void* ptr)
{
	if (uncle(ptr) != NULL)
	{
		if (get_color(uncle(ptr)) == 1)
		{
			color_black(parent(ptr));
			color_black(uncle(ptr));
			color_red(grandparent(ptr));
			insert_case1(grandparent(ptr));
		}
		else
		{
			insert_case4(ptr);
		}
	}
	else
	{
		insert_case4(ptr);
	}
}
void* parent_temp_lr;
void* parent_temp_rr;
void* new_root_temp;
void left_rotation(void* ptr)
{
	parent_temp_lr = parent(ptr);
	new_root_temp = right_child(ptr);
	if (parent_temp_lr == NULL)
	{
		tree_root = new_root_temp;
	}
	else
	{
		if (ptr == (left_child(parent_temp_lr)))
		{
	    	*(int*)(parent_temp_lr + 4) = (int)(new_root_temp);
		}
		else
		{
	    	*(int*)(parent_temp_lr + (get_size(parent_temp_lr)) - 4) = (int)(new_root_temp);
		}
	}
	if (left_child(right_child(ptr)) != NULL)
	{
	    *(int*)((left_child(right_child(ptr))) + 8) = (int)ptr;
	    *(int*)(ptr + (get_size(ptr)) - 4) = (int)(left_child(right_child(ptr)));
	}
	else
	{
	    *(int*)(ptr + (get_size(ptr)) - 4) = 0;
	}
	*(int*)(ptr + 8) = (int)new_root_temp;
	*(int*)(new_root_temp + 4) = (int)ptr;
	*(int*)(new_root_temp + 8) = (int)parent_temp_lr;
}
void right_rotation(void* ptr)
{
	parent_temp_rr = parent(ptr);
	new_root_temp = left_child(ptr);
	if (parent_temp_rr == NULL)
	{
		tree_root = new_root_temp;
	}
	else
	{
		if (ptr == (left_child(parent_temp_rr)))
		{
			*(int*)(parent_temp_rr + 4) = (int)(new_root_temp);
		}
		else
		{
			*(int*)(parent_temp_rr + (get_size(parent_temp_rr)) - 4) = (int)(new_root_temp);
		}
	}
	if (right_child(left_child(ptr)) != NULL)
	{
		*(int*)((right_child(left_child(ptr))) + 8) = (int)ptr;
		*(int*)(ptr + 4) = (int)(right_child(left_child(ptr)));
	}
	else
	{
		*(int*)(ptr + 4) = 0;
	}
	*(int*)(ptr + 8) = (int)new_root_temp;
	*(int*)(new_root_temp + (get_size(new_root_temp)) - 4) = (int)ptr;
	*(int*)(new_root_temp + 8) = (int)parent_temp_rr;
}
void* saved_p_ic4;
void* saved_left_ic4;
void* saved_right_ic4;
void* gp_temp_ic4;
void insert_case4 (void* ptr)
{
	if ((ptr == (right_child(parent(ptr)))) && ((parent(ptr)) == (left_child(grandparent(ptr)))))
	{
		left_rotation(parent(ptr));

		ptr = (void*)(*(int*)(ptr + 4));
	}
	else if ((ptr == (left_child(parent(ptr)))) && ((parent(ptr)) == (right_child(grandparent(ptr)))))
	{
		right_rotation(parent(ptr));

		ptr = (void*)(*(int*)(ptr + (get_size(ptr)) - 4));
	}
	insert_case5(ptr);
}
void insert_case5 (void* ptr)
{
	color_black(parent(ptr));
	color_red(grandparent(ptr));
	if (ptr == (left_child(parent(ptr))))
	{
		right_rotation(grandparent(ptr));
	}
	else
	{
		left_rotation(grandparent(ptr));
	}
}

void* sibling (void* ptr) // Finds the sibling and return it. If sibiling does not exists, it will return NULL.
{
	if (parent(ptr) != NULL)
	{
		if (ptr == (left_child(parent(ptr))))
		{
			return (right_child(parent(ptr)));
		}
		else
		{
			return (left_child(parent(ptr)));
		}
	}
	else
	{
		return NULL;
	}
}

void delete_one_child(void* ptr);
void delete_case1(void* ptr);
void delete_case2(void* ptr);
void delete_case3(void* ptr);
void delete_case4(void* ptr);
void delete_case5(void* ptr);
void delete_case6(void* ptr);
void replace_node(void* ptr1, void* ptr2);
void* replace_node_ptr1_parent_temp;
void* replace_node_ptr1_left_child_temp;
void* replace_node_ptr1_right_child_temp;
void* replace_node_ptr2_child_temp;
void* replace_node_ptr2_parent_temp;
void* copy_node_ptr1_parent_temp;
void* copy_node_ptr1_left_child_temp;
void* copy_node_ptr1_right_child_temp;
void* copy_node_ptr2_parent_temp;
void* copy_node_ptr2_left_child_temp;
void* copy_node_ptr2_right_child_temp;
void copy_node(void* ptr1, void* ptr2) // Just exchange the two nodes.
{
	if (ptr1 == NULL || ptr2 == NULL)
	{
		return;
	}
	else
	{
		if (ptr1 == tree_root)
		{
			tree_root = ptr2;
		}
		else if (ptr2 == tree_root)
		{
			tree_root = ptr1;
		}
		copy_node_ptr1_parent_temp = parent(ptr1);
		copy_node_ptr1_left_child_temp = left_child(ptr1);
		copy_node_ptr1_right_child_temp = right_child(ptr1);
		copy_node_ptr2_parent_temp = parent(ptr2);
		copy_node_ptr2_left_child_temp = left_child(ptr2);
		copy_node_ptr2_right_child_temp = right_child(ptr2);
		// Exception 1 - ptr1's right child is ptr2.
		if (ptr2 == (right_child(ptr1)))
		{
			copy_node_ptr1_right_child_temp = ptr1;
			copy_node_ptr2_parent_temp = ptr2;
		}
		// Exception 2 - ptr1's left child is ptr2.
		if (ptr2 == (left_child(ptr1)))
		{
			copy_node_ptr1_left_child_temp = ptr1;
			copy_node_ptr2_parent_temp = ptr2;
		}
		// Exception 3 - ptr2's right child is ptr1.
		if (ptr1 == (right_child(ptr2)))
		{
			copy_node_ptr2_right_child_temp = ptr2;
			copy_node_ptr1_parent_temp = ptr1;
		}
		// Exception 4 - ptr2's left child is ptr1.
		if (ptr1 == (left_child(ptr2)))
		{
			copy_node_ptr2_left_child_temp = ptr2;
			copy_node_ptr1_parent_temp = ptr1;
		}
		// Ordinary case.
			if (ptr1 == (left_child(copy_node_ptr1_parent_temp)))
			{
				*(int*)(copy_node_ptr1_parent_temp + 4) = (int)ptr2;
			}
			else if (ptr1 == (right_child(copy_node_ptr1_parent_temp)))
			{
				*(int*)(copy_node_ptr1_parent_temp + (get_size(copy_node_ptr1_parent_temp)) - 4) = (int)ptr2;
			}
			if (ptr2 == (left_child(copy_node_ptr2_parent_temp)))
			{
				*(int*)(copy_node_ptr2_parent_temp + 4) = (int)ptr1;
			}
			else if (ptr2 == (right_child(copy_node_ptr2_parent_temp)))
			{
				*(int*)(copy_node_ptr2_parent_temp + (get_size(copy_node_ptr2_parent_temp)) - 4) = (int)ptr1;
			}
		if (copy_node_ptr1_left_child_temp != NULL)
		{
			*(int*)(copy_node_ptr1_left_child_temp + 8) = (int)ptr2;
		}
		if (copy_node_ptr1_right_child_temp != NULL)
		{
			*(int*)(copy_node_ptr1_right_child_temp + 8) = (int)ptr2;
		}
		if (copy_node_ptr2_left_child_temp != NULL)
		{
			*(int*)(copy_node_ptr2_left_child_temp + 8) = (int)ptr1;
		}
		if (copy_node_ptr2_right_child_temp != NULL)
		{
			*(int*)(copy_node_ptr2_right_child_temp + 8) = (int)ptr1;
		}

		*(int*)(ptr1 + 4) = (int)copy_node_ptr2_left_child_temp;
		*(int*)(ptr1 + 8) = (int)copy_node_ptr2_parent_temp;
		*(int*)(ptr1 + (get_size(ptr1)) - 4) = (int)copy_node_ptr2_right_child_temp;
		*(int*)(ptr2 + 4) = (int)(copy_node_ptr1_left_child_temp);
		*(int*)(ptr2 + 8) = (int)(copy_node_ptr1_parent_temp);
		*(int*)(ptr2 + (get_size(ptr2)) - 4) = (int)(copy_node_ptr1_right_child_temp);

	}
}

void replace_node(void* ptr1, void* ptr2) // ptr2 goes into ptr1's place and ptr1 is removed from tree.
{
	if (ptr1 == NULL || ptr2 == NULL)
	{
		return;
	}
	else
	{
		int pred = 0;
		int succ = 0;
		replace_node_ptr1_parent_temp = parent(ptr1);
		replace_node_ptr1_left_child_temp = left_child(ptr1);
		replace_node_ptr1_right_child_temp = right_child(ptr1);
		replace_node_ptr2_parent_temp = parent(ptr2);
		if (left_child(ptr2) != NULL) // Inorder predecessor.
		{
			replace_node_ptr2_child_temp = left_child(ptr2);
			pred = 1;
		}
		else // Inorder successor.
		{
			replace_node_ptr2_child_temp = right_child(ptr2);
			succ = 1;
		}
		*(int*)(ptr1 + 4) = 0;
		*(int*)(ptr1 + 8) = 0;
		*(int*)(ptr1 + (get_size(ptr1)) - 4) = 0;
		*(int*)(ptr2 + 4) = (int)(replace_node_ptr1_left_child_temp);
		*(int*)(ptr2 + 8) = (int)(replace_node_ptr1_parent_temp);
		*(int*)(ptr2 + (get_size(ptr2)) - 4) = (int)(replace_node_ptr1_right_child_temp);
		if (replace_node_ptr2_parent_temp == ptr1) // Successor or predecessor's parent is ptr1. 
		{
			if (pred)
			{
				*(int*)(ptr2 + 4) = (int)(replace_node_ptr2_child_temp);
				if (replace_node_ptr2_child_temp != NULL)
				{
				    *(int*)(replace_node_ptr2_child_temp + 8) = (int)ptr2;
				}
			}
			else if (succ)
			{
				*(int*)(ptr2 + (get_size(ptr2)) - 4) = (int)(replace_node_ptr2_child_temp);
				if (replace_node_ptr2_child_temp != NULL)
				{
				    *(int*)(replace_node_ptr2_child_temp + 8) = (int)ptr2;
				}
			}
		}
		else // Successor or predecossor's parent is not ptr1. Ordinary case.
		{
			if (pred)
			{
				*(int*)(replace_node_ptr2_parent_temp + (get_size(replace_node_ptr2_parent_temp)) - 4) = (int)(replace_node_ptr2_child_temp);
				if (replace_node_ptr2_child_temp != NULL)
				{
				    *(int*)(replace_node_ptr2_child_temp + 8) = (int)replace_node_ptr2_parent_temp;
				}
			}
			else if (succ)
			{
				*(int*)(replace_node_ptr2_parent_temp + 4) = (int)(replace_node_ptr2_child_temp);
				if (replace_node_ptr2_child_temp != NULL)
				{
				    *(int*)(replace_node_ptr2_child_temp + 8) = (int)replace_node_ptr2_parent_temp;
				}
			}
		}
	}
}
void* delete_one_child_temp;
void* delete_one_child_parent_temp;
void* delete_one_child_parent_temp_freeing;
void delete_one_child(void* ptr)
{
	// Assumption : ptr should not have two children.
	if (ptr == NULL) // Invalid case.
	{
		return;
	}
	else
	{
		if (left_child(ptr) != NULL)
		{
			delete_one_child_temp = left_child(ptr);
		}
		else
		{
			delete_one_child_temp = right_child(ptr);
		}
		
		int color_child = -1;
		if (delete_one_child_temp != NULL)
		{
			color_child = get_color(delete_one_child_temp);
		}
		else
		{
			color_child = 0;
		}
		if (get_color(ptr) == 1)
		{
			delete_one_child_parent_temp = parent(ptr);
			if (ptr == left_child(delete_one_child_parent_temp))
			{
				*(int*)(delete_one_child_parent_temp + 4) = 0;
			}
			else if (ptr == right_child(delete_one_child_parent_temp))
			{
				*(int*)(delete_one_child_parent_temp + (get_size(delete_one_child_parent_temp)) - 4) = 0;
			}

			if (ptr == tree_root)
			{
				tree_root = NULL;
			}
			*(int*)(ptr + 4) = 0;
			*(int*)(ptr + 8) = 0;
			*(int*)(ptr + (get_size(ptr)) - 4) = 0;
		}
		else if ((get_color(ptr) == 0) && (color_child == 1))
		{
			color_black(delete_one_child_temp);
			delete_one_child_parent_temp = parent(ptr);
			copy_node(ptr, delete_one_child_temp);
			if (ptr == right_child(delete_one_child_temp))
			{
				*(int*)(delete_one_child_temp + (get_size(delete_one_child_temp)) - 4) = 0;
			}
			else
			{
				*(int*)(delete_one_child_temp + 4) = 0;
			}
			*(int*)(ptr + 4) = 0;
			*(int*)(ptr + 8) = 0;
			*(int*)(ptr + (get_size(ptr)) - 4) = 0;
			
		}
		else
		{
			delete_case1(ptr);
			delete_one_child_parent_temp = parent(ptr);
			if (delete_one_child_parent_temp == NULL)
			{
				tree_root = NULL;
			}
			else
			{
				if (ptr == left_child(delete_one_child_parent_temp))
				{
					*(int*)(delete_one_child_parent_temp + 4) = 0;
				}
				else
				{
					*(int*)(delete_one_child_parent_temp + (get_size(delete_one_child_parent_temp)) - 4) = 0;
				}
			}
			*(int*)(ptr + 4) = 0;
			*(int*)(ptr + 8) = 0;
			*(int*)(ptr + (get_size(ptr)) - 4) = 0;
		}
	}
}
void* find_inorder_pred_res;
void* find_inorder_pred(void* ptr)
{
	if (ptr == NULL) // Invalid case
	{
		return NULL;
	}
	else
	{
		if (left_child(ptr) == NULL) // Left child of ptr is NULL. - There is no inorder predecessor.
		{
			return NULL;
		}
		else
		{
			find_inorder_pred_res = left_child(ptr); // The default inorder predecessor is left child of ptr.
			while(right_child(find_inorder_pred_res) != NULL) // If left child of ptr is not the right most node of left child, Search the rigntmost.
			{
				find_inorder_pred_res = right_child(find_inorder_pred_res);
			}
			return find_inorder_pred_res;
		}
	}
}
void* find_inorder_succ_res;
void* find_inorder_succ(void* ptr)
{
	if (ptr == NULL) // Invalid case
	{
		return NULL;
	}
	else
	{
		if (right_child(ptr) == NULL) // Right child of ptr is NULL. - There is no inorder successor.
		{
			return NULL;
		}
		else
		{
			find_inorder_succ_res = right_child(ptr); // The default inorder successor is right child of ptr.
			while(right_child(find_inorder_succ_res) != NULL)
			{
				find_inorder_succ_res = left_child(find_inorder_succ_res);
			}
			return find_inorder_succ_res;
		}
	}
}
void* delete_inorder_pred_temp;
void* delete_inorder_succ_temp;
void delete(void* ptr)
{
	if (ptr == NULL) // Invalid case.
	{
		return;
	}
	else
	{
		if ((parent(ptr) == NULL) && (left_child(ptr) == NULL) && (right_child(ptr) == NULL)) // Case 1 : ptr is root and only node.
		{
			tree_root = NULL;
		}
		else if ((parent(ptr) == NULL) && (left_child(ptr) == NULL) && (right_child(ptr) != NULL)) // Case 2 : ptr is root and left child is NULL.
		{
			delete_one_child(ptr);
		}
		else if ((parent(ptr) == NULL) && (right_child(ptr) == NULL) && (left_child(ptr) != NULL)) // Case 3 : ptr is root and right child is NULL.
		{
			delete_one_child(ptr);
		}
		else if ((parent(ptr) == NULL) && (right_child(ptr) != NULL) && (left_child(ptr) != NULL)) // Case 4 : ptr is root and left, right are not NULL.
		{
			delete_inorder_pred_temp = find_inorder_pred(ptr);
			int color_ptr = get_color(ptr);
			int color_delete_inorder_pred_temp = get_color(delete_inorder_pred_temp);
			copy_node(ptr, delete_inorder_pred_temp);
		    	if (color_ptr == 0)
			{
			    color_black(delete_inorder_pred_temp);
			}
			else
			{
			    color_red(delete_inorder_pred_temp);
			}
			if (color_delete_inorder_pred_temp == 0)
			{
			    color_black(ptr);
			}
			else
			{
			    color_red(ptr);
			}
			tree_root = delete_inorder_pred_temp;
			delete_one_child(ptr);
		}
		else if ((right_child(ptr) == NULL) && (left_child(ptr) == NULL)) // Case 5 : ptr is not root and has no child.
		{
			delete_one_child(ptr);
		}
		else if ((right_child(ptr) != NULL) && (left_child(ptr) == NULL)) // Case 6 : ptr is not root and has right child.
		{
			delete_one_child(ptr);
		}
		else if ((right_child(ptr) == NULL) && (left_child(ptr) != NULL)) // Case 7 : ptr is not root and has left child.
		{
			delete_one_child(ptr);
		}
		else // Case 8 : ptr is not root and has both child.
		{
			delete_inorder_pred_temp = find_inorder_pred(ptr);
			int color_ptr = get_color(ptr);
			int color_delete_inorder_pred_temp = get_color(delete_inorder_pred_temp);
			copy_node(ptr, delete_inorder_pred_temp);
			if (color_ptr == 0)
			{
			    color_black(delete_inorder_pred_temp);
			}
			else
			{
			    color_red(delete_inorder_pred_temp);
			}
			if (color_delete_inorder_pred_temp == 0)
			{
			    color_black(ptr);
			}
			else
			{
			    color_red(ptr);
			}
			delete_one_child(ptr);
		}
	}
}

void delete_case1(void* ptr) // If ptr is root, done.
{
	if (parent(ptr) != NULL)
	{
		delete_case2(ptr);
	}
}
void* delete_case2_sibling;
void delete_case2(void* ptr)
{
	delete_case2_sibling = sibling(ptr);
	int color_sibling = -1;
	if (delete_case2_sibling == NULL)
	{
		color_sibling = 0;
	}
	else
	{
		color_sibling = get_color(delete_case2_sibling);
	}
	if (color_sibling == 1)
	{
		color_red(parent(ptr));
		color_black(delete_case2_sibling); // If color_sibling == 1, that implies existence of delete_case2_sibling.
		if (ptr == (left_child(parent(ptr))))
		{
			left_rotation(parent(ptr));
		}
		else
		{
			right_rotation(parent(ptr));
		}
	}
	delete_case3(ptr);
}
void* delete_case3_sibling;
void delete_case3(void* ptr)
{
	delete_case3_sibling = sibling(ptr);
	int color_sibling = -1; // Color of sibling. Exists and black - 0, NULL - 0, Exists and red - 1
	if (delete_case3_sibling == NULL)
	{
		color_sibling = 0;
	}
	else
	{
		color_sibling = get_color(delete_case3_sibling);
	}
	if ((get_color(parent(ptr)) == 0) && (color_sibling == 0)) // First two conditions.
	{
		if (delete_case3_sibling == NULL) // Color of sibling is black, but it is NULL leaf of parent.
		{
			delete_case4(ptr);
			return;
		}
		if ((left_child(delete_case3_sibling) == NULL) && (right_child(delete_case3_sibling) == NULL)) // if sibling has no children, their children are NULL, but thier color is black.
		{
			color_red(delete_case3_sibling);
			delete_case1(parent(ptr));
		}
		else if ((left_child(delete_case3_sibling) == NULL) && (get_color(right_child(delete_case3_sibling)) == 0)) // their is no left child(left child color - black), right child exists and color is black.
		{
			color_red(delete_case3_sibling);
			delete_case1(parent(ptr));
		}
		else if ((get_color(left_child(delete_case3_sibling)) == 0) && (right_child(delete_case3_sibling) == NULL)) // There is no right child(right child color - black), left child exists and color is black.
		{
			color_red(delete_case3_sibling);
			delete_case1(parent(ptr));
		}
		else if ((get_color(left_child(delete_case3_sibling)) == 0) && (get_color(right_child(delete_case3_sibling)) == 0)) // There are two children and both color is black.
		{
			color_red(delete_case3_sibling);
			delete_case1(parent(ptr));
		}
		else
		{
			delete_case4(ptr);
		}
	}
	else
	{
		delete_case4(ptr);
	}
}
void* delete_case4_sibling;
void delete_case4(void* ptr)
{
	delete_case4_sibling = sibling(ptr);
	int color_sibling = -1;
	if (delete_case4_sibling == NULL)
	{
		color_sibling = 0;
	}
	else
	{
		color_sibling = get_color(delete_case4_sibling);
	}
	if ((get_color(parent(ptr)) == 1) & (color_sibling == 0))
	{
		if (delete_case4_sibling == NULL)
		{
			delete_case5(ptr);
			return;
		}
		if ((left_child(delete_case4_sibling) == NULL) && (right_child(delete_case4_sibling) == NULL))
		{
			color_red(delete_case4_sibling);
			color_black(parent(ptr));
		}
		else if ((left_child(delete_case4_sibling) == NULL) && (get_color(right_child(delete_case4_sibling)) == 0))
		{
			color_red(delete_case4_sibling);
			color_black(parent(ptr));
		}
		else if ((get_color(left_child(delete_case4_sibling)) == 0) && (right_child(delete_case4_sibling) == NULL))
		{
			color_red(delete_case4_sibling);
			color_black(parent(ptr));
		}
		else if ((get_color(left_child(delete_case4_sibling)) == 0) && (get_color(right_child(delete_case4_sibling)) == 0))
		{
			color_red(delete_case4_sibling);
			color_black(parent(ptr));
		}
		else
		{
			delete_case5(ptr);
		}
	}
	else
	{
		delete_case5(ptr);
	}
}
void* delete_case5_sibling;
void delete_case5(void* ptr)
{
	delete_case5_sibling = sibling(ptr);
	int color_sibling = -1; // Get the color of sibling to make no problem when sibling does not exists.(NULL leaf)
	if (delete_case5_sibling == NULL)
	{
		color_sibling = 0;
	}
	else
	{
		color_sibling = get_color(delete_case5_sibling);
	}
	// Get the color of sibling's left and right child to make no problem when they does not exist.(-1 : no sibling, 0 : NULL leaf or black node, 1 : red node)
	int color_sibling_left = -1;
	int color_sibling_right = -1;
	if (delete_case5_sibling != NULL)
	{
		if (left_child(delete_case5_sibling) == NULL)
		{
			color_sibling_left = 0;
		}
		else
		{
			color_sibling_left = get_color(left_child(delete_case5_sibling));
		}
		
		if (right_child(delete_case5_sibling) == NULL)
		{
			color_sibling_right = 0;
		}
		else
		{
			color_sibling_right = get_color(right_child(delete_case5_sibling));
		}
	}
	if (color_sibling == 0)
	{
		if ((ptr == (left_child(parent(ptr)))) && (color_sibling_right == 0) && (color_sibling_left == 1))
		{
			color_red(delete_case5_sibling);
			color_black(left_child(delete_case5_sibling));
			right_rotation(delete_case5_sibling);
		}
		else if ((ptr == (right_child(parent(ptr)))) && (color_sibling_left == 0) && (color_sibling_right == 1))
		{
			color_red(delete_case5_sibling);
			color_black(right_child(delete_case5_sibling));
			left_rotation(delete_case5_sibling);
		}
	}
	delete_case6(ptr);
}
void* delete_case6_sibling;
void delete_case6(void* ptr)
{
	// Need to verify that sibling always exists and its left or right child(accordance with appropriate case) exists.
	delete_case6_sibling = sibling(ptr);
	int ptr_parent_color = get_color(parent(ptr));
	if (ptr_parent_color == 0)
	{
		color_black(delete_case6_sibling);
	}
	else
	{
		color_red(delete_case6_sibling);
	}
	color_black(parent(ptr));	
	if (ptr == (left_child(parent(ptr))))
	{
		color_black(right_child(delete_case6_sibling));
		left_rotation(parent(ptr));
	}
	else
	{
		color_black(left_child(delete_case6_sibling));
		right_rotation(parent(ptr));
	}
}
void* return_block;		
void* tree_root_temp;
void* find_block(int size, void* tree) // Find the best block(block size is same or greater than given size, and it should be same of smaller than size+24. If no smaller block than size is size+24, return smallest size block and split it in mm_malloc.
{
    if (tree == NULL) // Tree does not exist.
    {
	return NULL;
    }
    else // Tree exists.
    {
	return_block = NULL;
	tree_root_temp = tree;
	while(tree != NULL)
	{
	    int node_size = get_size(tree);
	    if ((node_size < size) && ((right_child(tree)) != NULL)) // size of node is smaller than size. must find larger block.
	    {
		tree = right_child(tree);
	    }
	    else if ((node_size < size) && (right_child(tree) == NULL)) // size of node is smaller than size, but there is no larger block, return.
	    {
		break;
	    }
	    else if ((node_size > size + 24) && (left_child(tree) != NULL)) // size of node is larger than size+24. must find smaller block.
	    {
		return_block = tree;
		tree = left_child(tree);
	    }
	    else if ((node_size > size + 24) && (left_child(tree) == NULL)) // size of node is larger than size+24, but there is no smaller block, return and split.
	    {
		return_block = tree;
		break;
	    }
	    else //((node_size >= size) && (node_size <= size + 24)) - Best block is found. allocate without splitting.
	    {
		return_block = tree;
		break;
	    }
	}
	tree = tree_root_temp;
	if (return_block != NULL)
	{
	    delete(return_block);
	}
	return return_block;
    }
}
void* found_node;

void inorder_traverse(void* tree)
{
    if (tree == NULL)
    {
	return;
    }
    else
    {
	if (left_child(tree) == NULL)
	{
	    printf("Color : %d, Size : %d, Address : %p, left child : %p, right child : %p, parent : %p\n", get_color(tree), get_size(tree), tree, left_child(tree), right_child(tree), parent(tree));
	    if (right_child(tree) != NULL)
	    {
		inorder_traverse(right_child(tree));
	    }
	}
	else
	{
	    inorder_traverse(left_child(tree));
	    printf("Color : %d, Size : %d, Address : %p, left child : %p, right child : %p, parent : %p\n", get_color(tree), get_size(tree), tree, left_child(tree), right_child(tree), parent(tree));
	    if (right_child(tree) != NULL)
	    {
		inorder_traverse(right_child(tree));
	    }
	}
    }
}
static int black_num = 0;
void* tree_check_temp;
void* tree_check_root_temp;
void tree_check(void* ptr, int black);
void tree_checker()
{
    tree_check_root_temp = tree_root;
    tree_check_temp = tree_root;
    while (tree_check_temp != NULL)
    {
		if (get_color(tree_check_temp) == 0)
		{
	    	black_num++;
		}
		tree_check_temp = left_child(tree_check_temp);
    }
    tree_root = tree_check_root_temp;

    tree_check(tree_root, 0);
    black_num = 0;
}
void tree_check(void* ptr, int black)
{
    if (tree_root == ptr && get_color(ptr) != 0)
    {
		printf("\n==================================================\n");
		printf("VIOLATED!!! (ROOT IS NOT BLACK) - tree root : %p\n", tree_root);
		inorder_traverse(tree_root);
		printf("==================================================\n");
		return;
    }
    if (get_color(ptr) == 0)
    {
		if (left_child(ptr) != NULL && right_child(ptr) != NULL)
		{
	    	tree_check((left_child(ptr)), (black + 1));
	    	tree_check((right_child(ptr)), (black + 1));
		}
		else if (left_child(ptr) != NULL && right_child(ptr) == NULL)
		{
	    	tree_check((left_child(ptr)), (black + 1));
		}
		else if (left_child(ptr) == NULL && right_child(ptr) != NULL)
		{
	    	tree_check((right_child(ptr)), (black + 1));
		}
		else
		{
	    	if (black_num != black+1)
	    	{
				printf("\n==================================================\n");
				printf("VIOLATED!!! (BLACK NODE NUMBER TO LEAFS ARE DIFFERENT)- tree_root : %p\n", tree_root);
				inorder_traverse(tree_root);
				printf("==================================================\n");
				return;
	    	}
	    	else
	    	{
			return;
	    	}
		}
    }
    else
    {
		int left_child_color = -1;
		int right_child_color = -1;
		if (left_child(ptr) != NULL)
		{
	    	left_child_color = get_color((left_child(ptr)));
		}
		if (right_child(ptr) != NULL)
		{
	    	right_child_color = get_color((right_child(ptr)));
		}

		if ((left_child_color == -1 || left_child_color == 0) && (right_child_color == -1 || right_child_color == 0))
		{
	    	if (left_child_color == 0 && right_child_color == 0)
	    	{
				tree_check(left_child(ptr), black);
				tree_check(right_child(ptr), black);
	    	}
	    	else if (left_child_color == 0 && right_child_color == -1)
	    	{
				tree_check(left_child(ptr), black);
	    	}
	    	else if (left_child_color == -1 && right_child_color == 0)
	    	{
				tree_check(right_child(ptr), black);
	    	}
	    	else
	    	{
				if (black_num != black)
				{
				printf("\n==================================================\n");
		    		printf("VIOLATED!!! (BLACK NODE NUMBER TO LEAFS ARE DIFFERENT) - tree_root : %p\n", tree_root);
		    		inorder_traverse(tree_root);
				printf("==================================================\n");
		    		return;
				}
				else
				{
		    		return;
				}
	    	}
		}
		else
		{
		printf("\n==================================================\n");
	    	printf("VIOLATED!!! (RED NODE HAS CHILD WHICH IS NOT BLACK)- tree_root : %p\n", tree_root);
	    	inorder_traverse(tree_root);
		printf("==================================================\n");
	    	return;
		}
    }
    return;
}

static float alpha = 1.0;
static float heuristic_size = 0.0;
static float heuristic_size_prev = 0.0;
static float heuristic_weight_prev = 0.0;
static float heuristic_weight = 0.75;
static float heuristic_weight_alpha = 0.75;
static float new_weight = 0.25;
static float new_weight_alpha = 0.25;
static float heuristic_pvalue = 0.045;
static int alloc_num = 0;
static int alloc_num_min = 4;
static int alloc_num_min_init = 8;
static int heuristic_init = 1;
static int size_multiplier = 1;
void* heuristic_alloc_temp;
void heuristic_allocation (int alloc_size)
{
    //printf("heuristic_allocation\n");
    int num = 1;
	int size = alloc_size * size_multiplier;
    while (num > 0)
    {
		heuristic_alloc_temp = mem_sbrk(size);
		*(int*)heuristic_alloc_temp = size + 0;
		*(int*)(heuristic_alloc_temp + size - 8) = size + 0;
		//if (lst_start != lst_end)
		if (heuristic_alloc_temp != lst_start)
		{
	    	int prev_size = (*(int*)(heuristic_alloc_temp - 8)) & -8;
	    	*(int*)(heuristic_alloc_temp - prev_size) = (*(int*)(heuristic_alloc_temp - prev_size)) & -3;
	    	*(int*)(heuristic_alloc_temp - 8) = (*(int*)(heuristic_alloc_temp - 8)) & -3;
		}
		lst_end = heuristic_alloc_temp + get_size(heuristic_alloc_temp);
		insert(heuristic_alloc_temp, tree_root);
		num = num - 1;
    }
    heuristic_alloc_temp = NULL;
}
void calculate_heuristic (float size, float heur)
{
	//printf("calculate_heuristic START\n");
    alloc_num = alloc_num + 1;
    float heuristic_size_prev_temp = heuristic_size;
    heuristic_size = ((heuristic_size * heuristic_weight) + (heuristic_size_prev * heuristic_weight_prev) + (size * new_weight));
    alpha = (alpha * heuristic_weight_alpha) + ((heuristic_size / heuristic_size_prev_temp) * new_weight_alpha);
    float abs_diff_alpha = 1.0 - alpha;
    if (abs_diff_alpha < 0.0)
    {
		abs_diff_alpha = 0.0 - abs_diff_alpha;
    }

	if (heuristic_init == 1)
	{
		if (abs_diff_alpha < heuristic_pvalue && alloc_num >= alloc_num_min_init)
		{
			heuristic_allocation((int)size);
			alloc_num = 0;
			heuristic_init = 0;
		}
	}
	else
	{
    	if (abs_diff_alpha < heuristic_pvalue && alloc_num >= alloc_num_min)
   	 	{
			heuristic_allocation((int)size);
			alloc_num = 0;
    	}
	}
	heuristic_size_prev = heuristic_size_prev_temp;
	//printf("calculate_heuristic END\n");
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

  /*
  lst_end_free = NULL;
  lst_tracker_free = NULL;
  lst_current_free = NULL;
  lst_free_next_temp = NULL;
  lst_free_prev_temp = NULL;
  lst_free_root_temp = NULL;
  */
  lst_start_free = NULL;
  lst_start = NULL;
  lst_end = NULL;
  lst_tracker = NULL;
  lst_current = NULL;
  remaining_block_temp = NULL;
  tree_root = NULL;
  heuristic_size = 0.0;
  alloc_num = 0;

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
  // For debuging.
  //printf("ALLOCATION - %d\n", (allocation_counter++));
  //int newsize = ALIGN(size + SIZE_T_SIZE);
  //printf("mm_malloc START\n");
  //list_print(lst_start);
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
    heuristic_size = newsize - 24;
    calculate_heuristic((float)newsize, heuristic_size);
    //printf("mm_malloc - new allocation - lst_start : %p, lst_end : %p, allocated : %d\n", lst_start, lst_end, ((*(int*)lst_start) & 1));
  }
  else // Allocating case 2 : Allocated blocks exists. Free or allocated. Find 'free and allocatable size' block. If not exists, allocate new block at the end of list.
  {
    //int allocated = 0;
    //for (lst_tracker_free = lst_start_free; ((lst_tracker_free != NULL) && ((*(int*)(lst_tracker_free + 8)) != 0)); lst_tracker_free = ((void*)(*((int*)lst_tracker_free + 2))))
    //lst_tracker_free = lst_start_free;
    //printf("Allocating case 2 - block size - %d\n", newsize);
    //while((lst_tracker_free != NULL)) //&& ((*(int*)(lst_tracker_free + 8)) != 0))
    found_node = find_block(newsize, tree_root);
	//printf("Allocating case2 - found_node : %p, found_node size : %d\n", found_node, get_size(found_node));
    if (found_node != NULL)
    {
	if (get_size(found_node) <= newsize + 24) // Case of best block.
	{
		//printf("Allocating case2 - get_size(found_node) <= newsize + 24 - lst_start : %p, found_node : %p\n", lst_start, found_node);
	    if (found_node != lst_start) // If found_node is not the start of block list, prev block's next block allocation bit of header and footer should be changed.
	    {
		int prev_size = *(int*)(found_node - 8) & -8;
		//printf("Allocating case2 - prev_size : %d\n", prev_size);
		*(int*)(found_node - prev_size) = (*(int*)(found_node - prev_size)) | 2;
		*(int*)(found_node - 8) = (*(int*)(found_node - 8)) | 2;
	    }
		//printf("Allocating case2 - done with prev block.\n");
	    *(int*)found_node = (*(int*)found_node | 1);
	    *(int*)(found_node + (get_size(found_node)) - 8) = (*(int*)(found_node + (get_size(found_node)) - 8) | 1);
	    lst_current = found_node;
	}
	else
	{
	    /*
	    //printf("Allocating case2 - get_size(found_node) > newsize + 24\n");
	    if (found_node != lst_start) // If found_node is not the start of block list, pre block's next block allocation bit of header and footer should be changed.
	    {
		int prev_size = *(int*)(found_node - 8) & -8;
		*(int*)(found_node - prev_size) = (*(int*)(found_node - prev_size)) | 2;
		*(int*)(found_node - 8) = (*(int*)(found_node - 8)) | 2;
	    }
	    lst_current = found_node;
	    int remaining_block_size = (get_size(found_node)) - newsize;
	    remaining_block_temp = lst_current + newsize;

	    *(int*)lst_current = (newsize & -8) + 1;
	    *(int*)(lst_current + newsize - 8) = (newsize & -8) + 1;
	    
	    *(int*)remaining_block_temp = remaining_block_size + (*(int*)(remaining_block_temp + remaining_block_size - 8) & 2);
	    *(int*)(remaining_block_temp + remaining_block_size - 8) = remaining_block_size + (*(int*)(remaining_block_temp + remaining_block_size - 8) & 2);

	    insert(remaining_block_temp, tree_root);
	    */
	    int remaining_block_size = (get_size(found_node)) - newsize;
	    lst_current = found_node + remaining_block_size;
	    remaining_block_temp = found_node;

	    *(int*)lst_current = (newsize & -8) + 1;
	    *(int*)(lst_current + newsize - 8) = (newsize & -8) + 1;

	    *(int*)remaining_block_temp = remaining_block_size + 2;
	    *(int*)(remaining_block_temp + remaining_block_size - 8) = remaining_block_size + 2;
	    if ((lst_current + get_size(lst_current)) != lst_end) 
	    {
		int next_allocated = (*(int*)(lst_current + (get_size(lst_current))) & 1);
		if (next_allocated == 1)
		{
		    *(int*)lst_current = (*(int*)lst_current) | 2;
		    *(int*)(lst_current + (get_size(lst_current)) - 8) = (*(int*)(lst_current + (get_size(lst_current)) - 8)) | 2;
		}
	    }
	    insert(remaining_block_temp, tree_root);
	}
    }
    else // Block is not found.
    {
	lst_current = mem_sbrk(newsize);
	if (lst_current == (void*)-1)
	{
	    return NULL;
	}
	int prev_size = *(int*)(lst_current - 8) & -8;
	*(int*)lst_current = (newsize & -8) + 1;
	*(int*)(lst_current + newsize - 8) = (newsize & -8) + 1;
	*(int*)(lst_current - 8) = (*(int*)(lst_current - 8)) | 2;
	*(int*)(lst_current - prev_size) = (*(int*)(lst_current - prev_size)) | 2;

	lst_end = lst_current + newsize;
	calculate_heuristic((float)newsize, heuristic_size);
    }
	//calculate_heuristic((float)newsize, heuristic_size);
  }
  //calculate_heuristic((float)newsize, heuristic_size);
  //printf("mm_malloc END\n");
  //list_print(lst_start);
  //printf("Allocation - address : %p, size : %d, actual size : %d\n", lst_current, newsize, get_size(lst_current));
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

void mm_free(void* ptr)
{
    //printf("mm_free START\n");
    //list_print(lst_start);
    ptr = ptr - 8;
    //printf("FREE - %d\n", free_counter++);
    //printf("mm_free - ptr : %p, size : %d\n", ptr, get_size(ptr));

    if ((*(int*)ptr & 1) != 1)
    {
	return;
    }
    int current_header = (*(int*)ptr) & 3;
    int current_size = (get_size(ptr));
    int prev_header = -1;
    int prev_size = -1;
    if (ptr != lst_start)
    {
	prev_header = (*(int*)(ptr - 8)) & 3;
	prev_size = (*(int*)(ptr - 8) & -8);
    }
    int next_header = -1;
    int next_size = -1;
    if ((ptr + (get_size(ptr))) != lst_end)
    {
	next_header = (*(int*)(ptr + (get_size(ptr))) & 3);
	next_size = (*(int*)(ptr + (get_size(ptr))) & -8);
    }
    
    if ((current_header == 3) && (prev_header == 3) && (next_header != -1)) // Case 1 : prev - allocated, current - allocated, next - allocated
    {
	//printf("mm_free - Case 1\n");
	*(int*)ptr = (get_size(ptr)) + 2;
	*(int*)(ptr + (get_size(ptr)) - 8) = (get_size(ptr)) + 2;
	*(int*)(ptr - prev_size) = prev_size + 1;
	*(int*)(ptr - 8) = prev_size + 1;
	insert(ptr, tree_root);
    }
    else if ((current_header == 3) && (prev_header == 2) && (next_header != -1)) // Case 2 : prev - free, current - allocated, next - allocated
    {
	//printf("mm_free - Case 2\n");
	lst_start_free = ptr - prev_size;
	delete(lst_start_free);
	*(int*)lst_start_free = current_size + prev_size + 2;
	*(int*)(lst_start_free + current_size + prev_size - 8) = current_size + prev_size + 2;
	insert(lst_start_free, tree_root);
    }
    else if ((current_header == 1) && (prev_header == 3) && (next_header != -1)) // Case 3: prev - allocated, current - allocated, next - free
    {
	//printf("mm_free - Case 3\n");
	lst_free_next_temp = ptr + current_size;
	delete(lst_free_next_temp);
	*(int*)ptr = current_size + next_size + next_header;
	*(int*)(ptr + current_size + next_size - 8) = current_size + next_size + next_header;
	*(int*)(ptr - prev_size) = (*(int*)(ptr - prev_size) & -8) + 1;
	*(int*)(ptr - 8) = (*(int*)(ptr - 8) & -8) + 1;
	insert(ptr, tree_root);
    }
    else if ((current_header == 1) && (prev_header == 2) && (next_header != -1)) // Case 4 : prev - free, current - allocated, next - free
    {
	//printf("mm_free - Case 4\n");
	lst_free_prev_temp = ptr - prev_size;
	lst_free_next_temp = ptr + current_size;
	delete(lst_free_prev_temp);
	delete(lst_free_next_temp);
	lst_start_free = ptr - prev_size;
	*(int*)lst_start_free = prev_size + current_size + next_size + next_header;
	*(int*)(lst_start_free + prev_size + current_size + next_size - 8) = prev_size + current_size + next_size + next_header;
	insert(lst_start_free, tree_root);
    }
    else if ((current_header == 1) && (prev_header == 3) && (next_header == -1)) // Case 5 : prev - allocated, current - allocated, next - X
    {
	//printf("mm_free - Case 5\n");
	lst_free_prev_temp = ptr - prev_size;
	*(int*)ptr = current_size + 0;
	*(int*)(ptr + current_size - 8) = current_size + 0;
	*(int*)lst_free_prev_temp = (*(int*)lst_free_prev_temp) & -3;
	*(int*)(lst_free_prev_temp + prev_size - 8) = (*(int*)(lst_free_prev_temp + prev_size - 8) & -3);
	insert(ptr, tree_root);
    }
    else if ((current_header == 1) && (prev_header == 2) && (next_header == -1)) // Case 6 : prev - free, current - allocated, next - X
    {
	//printf("mm_free - Case 6\n");
	lst_free_prev_temp = ptr - prev_size;
	delete(lst_free_prev_temp);
	lst_start_free = lst_free_prev_temp;
	*(int*)lst_start_free = prev_size + current_size + 0;
	*(int*)(lst_start_free + prev_size + current_size - 8) = prev_size + current_size + 0;
	insert(lst_start_free, tree_root);
    }
    else if ((current_header == 3) && (prev_header == -1) && (next_header != -1)) // Case 7 : prev - X, current - allocated, next - allocated
    {
	//printf("mm_free - Case 7\n");
	*(int*)ptr = current_size + 2;
	*(int*)(ptr + current_size - 8) = current_size + 2;
	insert(ptr, tree_root);
    }
    else if ((current_header == 1) && (prev_header == -1) && (next_header != -1)) // Case 8 : prev - X, current - allocated, next - free
    {
	//printf("mm_free - Case 8\n");
	lst_free_next_temp = ptr + current_size;
	delete(lst_free_next_temp);
	lst_start_free = ptr;
	*(int*)lst_start_free = current_size + next_size + next_header;
	*(int*)(lst_start_free + current_size + next_size - 8) = current_size + next_size + next_header;
	insert(lst_start_free, tree_root);
    }
    else // Case 9 : prev - X, current - allocated, next - X
    {
	//printf("mm_free - Case 9\n");
	*(int*)ptr = current_size + 0;
	*(int*)(ptr + current_size - 8) = current_size + 0;
	insert(ptr, tree_root);
    }
    //tree_checker();
    black_num = 0;
	//printf("mm_free END\n");
    //inorder_traverse(tree_root);
	//printf("\ntree_checker ==============================\n");
	//tree_checker();
	//printf("tree_checker ==============================\n\n");
    //printf("mm_free END\n");
    //list_print(lst_start);
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
  //printf("mm_exit entered - lst_start : %p, lst_end : %p\n", lst_start, lst_end);
  for(lst_tracker = lst_start; lst_tracker < lst_end; lst_tracker = lst_tracker + (*(int*)lst_tracker & -8))
  {
    //printf("mm_exit for loop - lst_tracker : %p, lst_start : %p, lst_end : %p, current block allocated? : %d\n", lst_tracker, lst_start, lst_end, ((*(int*)lst_tracker & 1)));
    //printf("First free list freed?(0 : freed) : %d, free block size : %d\n", (*(int*)lst_start_free & 1), (*(int*)lst_start_free & -8));
    if ((*(int*)lst_tracker & 1) == 1)
    {
      //printf("mm_exit - now freeing the unfreed block - lst_tracker : %p, block size : %d\n", lst_tracker, (*(int*)lst_tracker & -8));
      mm_free((void*)((char*)lst_tracker + 8));
    }
  }
  //inorder_traverse(tree_root);
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
  tree_root = NULL;
  heuristic_size = 0.0;
  alloc_num = 0;
  heuristic_init = 1;
}


