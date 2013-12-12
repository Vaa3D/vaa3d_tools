/**@file tz_objdetect.h
 * @brief object extraction from stack
 * @author Ting Zhao
 * @date 28-Nov-2007
 */

#ifndef _TZ_OBJDETECT_H_
#define _TZ_OBJDETECT_H_

#include "tz_cdefs.h"
#include "tz_image_lib.h"
#include "tz_object_3d_linked_list.h"

__BEGIN_DECLS

/* functions for object detection 
 *
 * Note: the input stack might be modified by an object detection function.
 */

/**@brief Find an object from a seed.
 *
 * Stack_Grow_Object() finds an object from a seed. The object consists of the
 * contiguous voxels with value <flag> and <seed> is included in the object.
 */
Object_3d* Stack_Grow_Object(Stack *stack, int flag, const Voxel_t seed);

/**@brief Find an object from a seed.
 *
 * Stack_Grow_Object_N() is similar to Stack_Grow_Object(), but it does not 
 * depend on internal neighborhood settings. The connectivity is specified by
 * <conn>, which is described in tz_stack_neighborhood.h. It also
 * allows a user to provide his own <chord> for storing intermediate result.it
 * could be NULL if you want the function to handle it; otherwise it should have
 * the same dimensions as <stack>.
 */
Object_3d* Stack_Grow_Object_N(Stack *stack, IMatrix *chord, int flag, 
			       const Voxel_t seed, int conn);

/**@brief Find all objects in a stack.
 *
 * Stack_Fine_Object() finds all objects with voxel value <flag> in a stack.
 * <min_size> is the threshold to filter out small objects. All object in the
 * list will have the size of at least <min_size>.
 */
Object_3d_List* Stack_Find_Object(Stack *stack, int flag, int min_size);

/**@brief Find all objects in a stack.
 *
 * Stack_Fine_Object_N() is similar to Stack_Find_Object(), but it does not 
 * depend on internal neighborhood settings. The connectivity is specified by
 * <conn>, which is described in tz_stack_neighborhood.h. It also
 * allows a user to provide his own <chord> for storing intermediate result.it
 * could be NULL if you want the function to handle it; otherwise it should have
 * the same dimensions as <stack>.
 *
 * Note: <stack> will be changed after return of the functions.
 */
Object_3d_List* Stack_Find_Object_N(Stack *stack, IMatrix *chord, int flag, 
				    int min_size, int conn);

/**@brief Find objects to a certain level
 */
Object_3d_List* Stack_Find_Object_Level(Stack *stack, IMatrix *chord, int flag, 
					int min_size, int conn, 
					int max_level, Stack *code);



Object_3d* Stack_Find_Largest_Object_N(Stack *stack, IMatrix *chord, int flag,
				       int conn);

/* 
 * Stack_Grow_Object_Constraint() grows an object  
 */
Object_3d* Stack_Grow_Object_Constraint(Stack *stack, int flag, 
					const Voxel_t seed, IMatrix *chord,
					Stack *code, int max);

/*
 * Stack_Grow_Object_Dist_N() (under development)
 */
Object_3d* Stack_Grow_Object_Dist_N(Stack *stack, int flag, 
				    const Voxel_t seed, IMatrix *chord,
				    double dist,  int n_nbr);

/*
 * Stack_Label_Background() labels background in a stack with the value <value>.
 * Here the background is defined as dark regions not surrounded by non-zero
 * voxels.
 *
 * Stack_Label_Background_N() is the new version of Stack_Label_Background().
 * It supports customizing connectivity and taking intermediate result storage.
 */
void Stack_Label_Background(Stack *stack, int value);
void Stack_Label_Background_N(Stack *stack, int value, int conn, 
			      IMatrix *chord);

/*
 * Stack_Find_Hole() and Stack_Find_2dhole() finds holes in a stack.
 * Object_3d_Find_Hole() and Object_3d_Find_2dhole() finds holes in an object.
 * But Stack_Find_2dhole() and  Object_3d_Find_2dhole() will label the holes 
 * slice by slice and then find the labeled 3D holes.
 *
 * Notice: <stack> will be changed after functin call.
 *
 * Stack_Find_Hole_N() and Object_3d_Find_Hole_N() are new versions of finding
 * holes. They support customizing connectivity. Stack_Find_Hole_N() also takes
 * the intermediate storage.
 */
Object_3d_List* Stack_Find_Hole(Stack *stack);
Object_3d_List* Object_3d_Find_Hole(Object_3d *obj);
Object_3d_List* Stack_Find_2dhole(Stack *stack, int iscon1);
Object_3d_List* Object_3d_Find_2dhole(Object_3d *obj, int iscon1);

Object_3d_List* Stack_Find_Hole_N(Stack *stack, int conn, IMatrix *chord);
Object_3d_List* Object_3d_Find_Hole_N(Object_3d *obj, int conn);

__END_DECLS

#endif
