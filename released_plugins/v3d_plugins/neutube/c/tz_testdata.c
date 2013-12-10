#include <stdio.h>
#include <time.h>
#ifndef _MSC_VER
#include <unistd.h>
#else
#include <process.h>
#define getpid _getpid
#endif
#include "tz_testdata.h"
#include "tz_stack_attribute.h"
#include "tz_stack.h"
#include "tz_object_3d.h"
#include "tz_voxel_graphics.h"
#include "tz_stack_draw.h"
#include "tz_stack_lib.h"
#include "tz_fmatrix.h"
#include "tz_math.h"
#include "tz_random.h"
#include "tz_error.h"

#define INDEX_STACK(stack_array) \
  for (i = 0; i < nvoxel; i++) { \
    stack_array[i] = i; \
  }

Stack *Index_Stack(int kind, int width, int height, int depth)
{
  Stack *stack = Make_Stack(kind, width, height, depth);
  int i;
  int nvoxel = Stack_Voxel_Number(stack);

  SCALAR_STACK_OPERATION(stack, INDEX_STACK);

  return stack;
}

Stack* Line_Stack(int *start, int *end, int margin)
{
  Stack *stack = Make_Stack(GREY, imax2(start[0], end[0]) + margin + 1,
      imax2(start[1], end[1]) + margin + 1, 
      imax2(start[2], end[2]) + margin + 1);
  Object_3d *obj = Line_To_Object_3d(start, end);
  Stack_Draw_Object_Bw(stack, obj, 1);

  return stack;
}

Stack* Tube_Stack(int *start, int *end, int margin, double *sigma)
{
  Stack *stack = Line_Stack(start, end, margin);
  FMatrix *filter = Gaussian_3D_Filter_F(sigma, NULL);
  FMatrix *f = Filter_Stack_Fast_F(stack, filter, NULL, 0);
  Kill_Stack(stack);
  Stack *out = Scale_Float_Stack(f->array, f->dim[0], f->dim[1], f->dim[2], 
				 GREY);
  Kill_FMatrix(filter);
  Kill_FMatrix(f);
  
  return out; 
}

Stack* Circle_Image(double radius)
{
  int width = iround(radius) * 3 + 10;
  Stack *stack = Make_Stack(GREY, width, width, 1);

  int i, j;
  int offset = 0;
  int center = width / 2;
  for (j = 0; j < Stack_Height(stack); j++) {
    for (i = 0; i < Stack_Width(stack); i++) {
      if ((i-center) * (i-center) + (j-center) * (j-center) <= 
	  radius * radius) {
	stack->array[offset] = 1;
      } else {
	stack->array[offset] = 0;
      }
      offset++;
    }
  }
  double sigma[3] = {1.0, 1.0, 1.0};
  FMatrix *filter = Gaussian_3D_Filter_F(sigma, NULL); 
  FMatrix *f = Filter_Stack_Fast_F(stack, filter, NULL, 0);

  Kill_Stack(stack);
  Stack *out = Scale_Float_Stack(f->array, f->dim[0], f->dim[1], f->dim[2], 
				 GREY);
  Kill_FMatrix(filter);
  Kill_FMatrix(f);

  return out;
}

Stack* Ellipse_Mask(double rx, double ry)
{
  int width = iround(rx) * 3 + 10;
  int height = iround(ry) * 3 + 10;
  Stack *stack = Make_Stack(GREY, width, height, 1);

  int i, j;
  int offset = 0;
  int center[2];
  center[0] = width / 2;
  center[1] = height / 2;
  double scale = rx / ry;
  for (j = 0; j < Stack_Height(stack); j++) {
    for (i = 0; i < Stack_Width(stack); i++) {
      if ((i-center[0]) * (i-center[0]) + 
	  (j-center[1]) * (j-center[1]) * scale * scale <= 
	  rx * rx) {
	stack->array[offset] = 1;
      } else {
	stack->array[offset] = 0;
      }
      offset++;
    }
  }
  return stack;
}

Stack* Ellipse_Image(double rx, double ry)
{
  Stack *stack = Ellipse_Mask(rx, ry);
  double sigma[3] = {1.0, 1.0, 1.0};
  FMatrix *filter = Gaussian_3D_Filter_F(sigma, NULL); 
  FMatrix *f = Filter_Stack_Fast_F(stack, filter, NULL, 0);

  Kill_Stack(stack);
  Stack *out = Scale_Float_Stack(f->array, f->dim[0], f->dim[1], f->dim[2], 
				 GREY);
  Kill_FMatrix(filter);
  Kill_FMatrix(f);

  return out;
}

Stack* Noisy_Ellipse_Image(double rx, double ry, double mu)
{
  Stack *out = Ellipse_Image(rx, ry);
  int i;
  int nvoxel = Stack_Voxel_Number(out);
  Random_Seed(time(NULL) - getpid());
  for (i = 0; i < nvoxel; i++) {
    unsigned int noise = Poissonrnd(mu+out->array[i]/20);

    //noise += out->array[i];
    if (noise > 255) {
      noise = 255;
    }
    out->array[i] = noise;
  }

  return out;
}

Stack* Two_Circle_Mask(double r1, int cx1, int cy1, double r2, int cx2, 
    int cy2)
{
  int width = imax2(cx1 + iround(r1), cx2 + iround(r2)) + 50;
  int height = imax2(cy1 + iround(r1), cy2 + iround(r2)) + 50;
  Stack *stack = Make_Stack(GREY, width, height, 1);
  int i, j;
  int offset = 0;
  for (j = 0; j < Stack_Height(stack); j++) {
    for (i = 0; i < Stack_Width(stack); i++) {
      if (((i-cx1) * (i-cx1) + (j-cy1) * (j-cy1) <= r1 * r1) ||
	  ((i-cx2) * (i-cx2) + (j-cy2) * (j-cy2) <= r2 * r2)) {
	stack->array[offset] = 1;
      } else {
	stack->array[offset] = 0;
      }
      offset++;
    }
  }

  return stack;
}

Stack* Two_Circle_Image(double r1, int cx1, int cy1, double r2, int cx2, 
    int cy2)
{
  Stack *stack = Two_Circle_Mask(r1, cx1, cy1, r2, cx2, cy2);
  double sigma[3] = {1.0, 1.0, 1.0};
  FMatrix *filter = Gaussian_3D_Filter_F(sigma, NULL); 
  FMatrix *f = Filter_Stack_Fast_F(stack, filter, NULL, 0);

  Kill_Stack(stack);
  Stack *out = Scale_Float_Stack(f->array, f->dim[0], f->dim[1], f->dim[2], 
				 GREY);
  Kill_FMatrix(filter);
  Kill_FMatrix(f);

  return out;
}

Stack* Noisy_Two_Circle_Image(double r1, int cx1, int cy1, double r2, int cx2, 
    int cy2, double mu)
{
  Stack *out = Two_Circle_Image(r1, cx1, cy1, r2, cx2, cy2);
  int i;
  int nvoxel = Stack_Voxel_Number(out);
  Random_Seed(time(NULL) - getpid());
  for (i = 0; i < nvoxel; i++) {
    unsigned int noise = Poissonrnd(mu+out->array[i]/(iround(mu/10)));
    if (noise > 255) {
      noise = 255;
    }
    out->array[i] = noise;
  }

  return out;
}

void Noisy_Stack(Stack *stack, double mu)
{
  int i;
  int nvoxel = Stack_Voxel_Number(stack);
  Random_Seed(time(NULL) - getpid());
  for (i = 0; i < nvoxel; i++) {
    unsigned int noise = Poissonrnd(mu+stack->array[i]/(iround(mu/10)));
    if (noise > 255) {
      noise = 255;
    }
    stack->array[i] = noise;
  }
}
