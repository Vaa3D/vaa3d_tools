/**@file tz_testdata.h
 * @brief Testing data generation.
 * @author Ting Zhao
 * @date 02-May-2010
 */

#ifndef _TZ_TESTDATA_H_
#define _TZ_TESTDATA_H_

#include "tz_cdefs.h"
#include "tz_image_lib_defs.h"

__BEGIN_DECLS

Stack* Index_Stack(int kind, int width, int height, int depth);

Stack* Line_Stack(int *start, int *end, int margin);

Stack* Tube_Stack(int *start, int *end, int margin, double *sigma);

Stack* Circle_Image(double radius);
Stack* Ellipse_Mask(double rx, double ry);
Stack* Ellipse_Image(double rx, double ry);
Stack* Noisy_Ellipse_Image(double rx, double ry, double mu);

Stack* Two_Circle_Mask(double r1, int cx1, int cy1, double r2, int cx2, 
    int cy2);
Stack* Two_Circle_Image(double r1, int cx1, int cy1, double r2, int cx2, 
    int cy2);
Stack* Noisy_Two_Circle_Image(double r1, int cx1, int cy1, double r2, int cx2, 
    int cy2, double mu);

void Noisy_Stack(Stack *stack, double mu);

__END_DECLS

#endif
