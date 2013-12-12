/**@file tz_stack_code.h
 * @brief stack structural coding
 * @author Ting Zhao
 * @date 19-Feb-2008
 */

#ifndef _TZ_STACK_CODE_H_
#define _TZ_STACK_CODE_H_

#include <image_lib.h>
#include "tz_cdefs.h"
#include "tz_image_lib_defs.h"
#include "tz_stack_objlabel.h"

__BEGIN_DECLS

/*
 * Stack_Level_Code() creates a level code map for <stack>. The value in the
 * code map means how far it grows from the seeds stored in <seed>. Each element
 * of <seed> is the array index of the position of a seed and there are a total
 * of <nseed> seeds. <link> is an array to store intermediate results and it
 * could be NULL, or it should be at least as long as the stack array. The 
 * result will be stored in <code> and the returned pointer is the same as 
 * <code> if * it is not NULL, otherwise it returns a new stack object.
 *
 * Stack_Level_Code_Constraint() creates a level code map for <stack> up to
 * the level no higher than <max_level>.
 */
Stack* Stack_Level_Code(Stack *stack, Stack *code, int *link, 
			const int *seed, int nseed);
Stack* Stack_Level_Code_Constraint(Stack *stack, Stack *code, int *link, 
				   const int *seed, int nseed, int max_level);

Stack* Stack_Boundary_Code(Stack *stack, Stack *code, Objlabel_Workspace *ow);

/*
 * Stack_Branch_Code() creates a branch code map for <stack> from the seed
 * index <seed>. Two pixels have the same code value when and only when they are
 * on the same branch. <parent> is an array to store tree information. The ith
 * element is the parent code of the branch with code i. It should be long
 * enough to hold the result. Unfortunately there is no function to tell you
 * how long it could be, so you should make a guess. The seed has code 0.
 * The result is stored in <code> and the returned pointer is the same as 
 * <code> if it is not NULL, otherwise it returns a new stack object.
 */
Stack* Stack_Branch_Code(Stack *stack, Stack *code, int *link, uint16 *edge,
			 int seed);

/*
 * Stack_Region_Grow() grows a region in a stack from the seed with array index
 * <seed>. <threshold> and <difference> determines when the region stops 
 * growing. A voxel will stop growing when all its neighbors that are not in
 * the region have value lower than threshold or are more than <difference>
 * away from the value of the voxel. <mask> is a binary stack for defining
 * the region of interesting. Set it to NULL if there is no mask. The result
 * is stored in <tree> and the returned pointer is the same as <tree> if it is
 * not NULL, otherwise the function returns a new pointer. 
 */
Level_Code* Stack_Region_Grow(const Stack *stack, Level_Code *tree, int seed, 
			      int threshold, int difference, const Stack *mask);

__END_DECLS

#endif
