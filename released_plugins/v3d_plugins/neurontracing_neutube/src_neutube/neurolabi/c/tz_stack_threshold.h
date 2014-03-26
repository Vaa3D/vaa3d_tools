/**@file tz_stack_threshold.h
 * @brief stack thresholding
 * @author Ting Zhao
 * @date 27-Feb-2008
 */

#ifndef _TZ_STACK_THRESHOLD_H_
#define _TZ_STACK_THRESHOLD_H_

#include <image_lib.h>
#include "tz_cdefs.h"

__BEGIN_DECLS

/**@brief Manual Thresholding.
 *
 * Stack_Threshold() thresholds <stack> by comparing its voxel intensity values
 * to <thre>. Any voxel that has intensity value no greater than <thre> is set 
 * to 0 and other voxels are unchanged. It returns 1 if any voxel of <stack>
 * is changed; otherwise it returns 0.
 */
int Stack_Threshold(Stack *stack, int thre);

/**@brief Binarize a stack by a threshold.
 *
 * Stack_Threshold_Binarize() binarizes <stack> by the threshold <thre>. All
 * voxels that have intensity value greater than <thre> are set to 1 and other
 * voxels are set to 0.
 */
void Stack_Threshold_Binarize(Stack *stack, int thre);

/**@brief Binarize a stack slice by slice. 
 *
 * Stack_Threshold_Binarize_S() binarizes <stack> slice by slice. Each slice
 * has its own threshold. The ith slice (i starts from 0) uses thre[i] as its
 * threshold.
 */
void Stack_Threshold_Binarize_S(Stack *stack, const int *thre);


/* Options for thresholding. */
enum {
  THRESHOLD_DEFAULT,
  THRESHOLD_COMMON,
  THRESHOLD_LOCMAX_TRIANGLE, 
  THRESHOLD_MEDIAN,
  THRESHOLD_RC,
  THRESHOLD_TRIANGLE,
  THRESHOLD_SP,
  THRESHOLD_TP,
  THRESHOLD_TP2,
  THRESHOLD_TP3,
  THRESHOLD_OTSU,
};

/**@brief Automatic binarization.
 *
 * Stack_Threshold_Binarize_A binarizes <stack> with the method specified by
 * <meth>.
 */
void Stack_Threshold_Binarize_A(Stack *stack, int meth);

/**@brief Threshold by the most common value.
 */
int Stack_Threshold_Common(Stack* stack, int low, int high);

/**@brief Threshold by the median value.
 */
int Stack_Threshold_Median(Stack* stack);

/**@brief Threshold by a quantile.
 */
int Stack_Threshold_Quantile(Stack* stack, double q);

/**@brief RC thresholding.
 */
int Stack_Threshold_RC(Stack* stack, int low, int high);

/**@brief Triangle thresholding.
 */
int Stack_Threshold_Triangle(Stack* stack, int low, int high);

/* The following routines are for experimental purposes. They do not work very
 * well in real cases. */
void Stack_Threshold_Sp(Stack* stack, int low, int high);
void Stack_Threshold_Tp(Stack* stack, int low, int high);
void Stack_Threshold_Tp2(Stack *stack, int low, int high);
void Stack_Threshold_Tp3(Stack *stack, int low, int high);
/************************/

int Hist_Rcthre_R(int *hist,int low,int high, double *c1, double *c2);
int Stack_Threshold_RC_R(Stack* stack, int low, int high, 
			 double *c1, double *c2);

/*
 * Stack_Find_Threshold_Locmax() returns the threshold found based on the local
 * maximum of <stack>.
 */
int Stack_Find_Threshold_Locmax(Stack *stack, int low, int high);
int Stack_Find_Threshold_Locmax2(Stack *stack, int low, int high, double alpha);
int Stack_Find_Threshold_Locmax_L2(Stack *stack, int low, int high, 
    double alpha);

int Stack_Find_Threshold_A(Stack *stack, int meth);

void Stack_Threshold_Tp4(Stack *stack, int low, int high);
void Stack_Threshold_Dp(Stack* stack,int low,int high);
void Stack_Threshold_Dp2(Stack* stack,int low,int high);

__END_DECLS

#endif
