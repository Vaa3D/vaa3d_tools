/**@file tz_stack_neighborhood.h
 * @brief neighborhood utilities
 * @author Ting Zhao
 * @date 14-Jan-2008
 */

#ifndef _TZ_STACK_NEIGHBORHOOD_H_
#define _TZ_STACK_NEIGHBORHOOD_H_

#include <image_lib.h>
#include "tz_cdefs.h"

__BEGIN_DECLS

/**@brief  Print neighborhood offsets.
 *
 * Print_Neighborhood() prints the neighborhood array to the screen. <conn>
 * is the connectivity of the neighborhood system, which coudle be 4, 8 (2D), 
 * 6, 10, 18 or 26 (3D).
 *
 * Note: the connectivity rule applies to other neighborhood routines in this
 * file.
 */
void Print_Neighborhood(int neighbor[], int conn);

/**@brief Print double neighborhood array.
 * 
 * Print_Neighborhood_F() takes a double array to print.
 */
void Print_Neighborhood_F(double neighbor[], int conn);

/*
 * Stack_Neighbor_Offset() calculates the offsets of the neighborhood and store
 * it in the array <neighbor>. Each slice of the stack is supposed to have size
 * <width> x <height>.
 *
 * Stack_Neighbor_Positive_Offset() only considers the neighbors that have
 * positive offset.
 */
void Stack_Neighbor_Offset(int conn, int width, int height, int neighbor[]);
void Stack_Neighbor_Positive_Offset(int conn, int width, int height, 
				    int neighbor[]);

/**@brief  Neighborhood distance.
 *
 * Stack_Neighbor_Dist() returns a constant array of the distance of each
 * neighbor.
 */
const double* Stack_Neighbor_Dist(int conn);

void Stack_Neighbor_Dist_R(int conn, const double *res, double *dist);

/**@brief x offsets of the neighbors */
const int* Stack_Neighbor_X_Offset(int conn);
/**@brief y offsets of the neighbors */
const int* Stack_Neighbor_Y_Offset(int conn);
/**@brief z offsets of the neighbors */
const int* Stack_Neighbor_Z_Offset(int conn);

/*@brief bound test of a voxel
 *
 * Stack_Neighbor_Bound_Test() stores boundary mask in <is_in_bound> for the
 * neighbors of the position (<x>, <y>, <z>) in the bounding box with size
 * <cwidth + 1> x <cheight + 1> x <cdepth + 1>. It returns the number of 
 * eighbors that are in bound. 
 *
 * NOTE: If the returned value is 0 or <conn>, do NOT use <is_in_bound> 
 *   for boundary check because its values have no meaning.
 * 
 * NOTE: <cwdith>, <cheight> and <depth> are NOT exact stack sizes. Instead, 
 *   they are 1 voxel smaller.
 */
int Stack_Neighbor_Bound_Test(int conn, int cwidth, int cheight, int cdepth, 
			      int x, int y, int z, int is_in_bound[]);

/*@brief bound test of a voxel
 *
 * Stack_Neighbor_Bound_Test_S() is similiar to Stack_Neighbor_Bound_Test(). 
 * But it returns 0 when (<x>, <y>, <z>) is out of bound.
 */
int Stack_Neighbor_Bound_Test_S(int conn, int cwidth, int cheight, 
			       int cdepth, int x, int y, int z, 
				int *is_in_bound);

/**@brief Bound test for an array index.
 *
 * Stack_Neighbor_Bound_Test_I() does boundary test for \a index in the
 * bounding box with size \a width x \a height x \a depth.
 *
 * @note Unlike Stack_Neighbor_Bound_Test(), this function takes exact
 * stack size as input arguments.
 */
int Stack_Neighbor_Bound_Test_I(int conn, int width, int height, int depth, 
				size_t index, int is_in_bound[]);

/**@brief sample the neighbors of a voxel
 *
 * Stack_Neighbor_Sampling() samples the neighbors of a (<x>, <y>, <z>) in 
 * <stack> and stores the values in <s>. The values of out-bound neighbors 
 * are set to <out_bound_value>.
 */
void Stack_Neighbor_Sampling(const Stack *stack, int conn, 
			     int x, int y, int z, double out_bound_value, 
			     double s[]);

/**@brief sample the neighbors of a voxel
 *
 * Stack_Neighbor_Sampling_I() is similar to Stack_Neighbor_Sampling(), except
 * that this function takes the index of the voxel instead of coordinates.
 */
void Stack_Neighbor_Sampling_I(const Stack *stack, int n_nbr, 
			       size_t index, double out_bound_value, double s[]);

/**@brief minimal value of a voxel and its neighborhood 
 *
 * Stack_Neighbor_Min() returns the minimal value of the voxel (<x>, <y>, <z>)
 * and its <n_nbr> neighbor voxels in <stack>.
 */
double Stack_Neighbor_Min(const Stack *stack, int n_nbr, int x, int y, int z);

/**@brief mean value of a voxel and its neighborhood 
 *
 * Stack_Neighbor_Mean() returns the mean value of the voxel (<x>, <y>, <z>)
 * and its <n_nbr> neighbor voxels in <stack>.
 */
double Stack_Neighbor_Mean(const Stack *stack, int n_nbr, int x, int y, int z);

/**@brief Set the neighbor values.
 *
 * Stack_Neighbor_Set_I() sets the neighbors of the voxels with its index
 * <index> to <value>.
 */
void Stack_Neighbor_Set_I(const Stack *stack, int n_nbr, size_t index, 
    double value);

//int Stack_Neighbor_Pattern_26(const Stack *stack, int *neighbor_offset, size_t index);
void Stack_Label_Neighbor_26(Stack *stack, int index);

__END_DECLS

#endif 
