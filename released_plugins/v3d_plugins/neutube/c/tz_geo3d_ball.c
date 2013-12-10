#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utilities.h>
#include <draw_lib.h>
#include <math.h>
#include "tz_geo3d_ball.h"
#include "tz_error.h"
#include "tz_math.h"
#include "tz_arrayview.h"
#include "tz_cuboid_i.h"
#include "tz_stack_attribute.h"

#include "private/tzp_geo3d_ball.c"

Geo3d_Ball* New_Geo3d_Ball()
{
  Geo3d_Ball *ball =
    (Geo3d_Ball *) Guarded_Malloc(sizeof(Geo3d_Ball), "New_Geo3d_Ball");

  return ball;
}

void Delete_Geo3d_Ball(Geo3d_Ball *ball)
{
  free(ball);
}

void Kill_Geo3d_Ball(Geo3d_Ball *ball)
{
  Delete_Geo3d_Ball(ball);
}

void Geo3d_Ball_Default(Geo3d_Ball *ball)
{
  ball->r = 1.0;
  ball->center[0] = 0.0;
  ball->center[1] = 0.0;
  ball->center[2] = 0.0;
}

void Geo3d_Ball_Copy(Geo3d_Ball *dst, const Geo3d_Ball *src)
{
  memcpy(dst, src, sizeof(Geo3d_Ball));
}

Geo3d_Ball* Copy_Geo3d_Ball(Geo3d_Ball *src)
{
  Geo3d_Ball *ball = New_Geo3d_Ball();
  memcpy(ball, src, sizeof(Geo3d_Ball));

  return ball;
}

void Print_Geo3d_Ball(const Geo3d_Ball *ball)
{
  printf("3D ball: radius (%g) position(%g %g %g)\n", 
	 ball->r, ball->center[0], ball->center[1], ball->center[2]);
}

void Geo3d_Ball_Draw_Stack(const Geo3d_Ball *ball, Stack *stack,
			   const Stack_Draw_Workspace *ws)
{
  TZ_ASSERT(stack->kind == COLOR, "Invalid stack kind");
  
  int z;
  int zmin = imax2((int) ((ball->center[2] - ball->r) * ws->z_scale), 0);
  int zmax = imin2((int) ((ball->center[2] + ball->r) * ws->z_scale), 
		   stack->depth - 1);

  Paint_Brush brush;
  switch (ws->color_mode) {
  case 0:
    brush.red = 1.0;
    brush.green = 0.0;
    brush.blue = 0.0;
    break;
  case 1:
    brush.red = (double) ws->color.r / 255.0;
    brush.green = (double) ws->color.g / 255.0;
    brush.blue = (double) ws->color.b / 255.0;
    break;
  case 2:
    {
      Rgb_Color color;
      Set_Color_Hsv(&color, ws->h, ws->s, ws->v);
      brush.red = (double) color.r / 255.0;
      brush.green = (double) color.g / 255.0;
      brush.blue = (double) color.b / 255.0;
    }
    break;
  default:
    TZ_ERROR(ERROR_DATA_VALUE);
  }

  brush.op = (Pixel_Op)ws->blend_mode;

  for (z = zmin; z <= zmax; z++) {
    double h = (double) z / ws->z_scale - ball->center[2]; 
    double r = sqrt(ball->r * ball->r - h * h);
    Image canvas = Image_View_Stack_Slice(stack, z);
    Draw_Circle(iround(ball->center[0]), iround(ball->center[1]), iround(r),
		&brush, &(canvas));
  }
}


Geo3d_Scalar_Field* Geo3d_Ball_Field(const Geo3d_Ball *ball,
    Geo3d_Scalar_Field *field)
{
  if (field == NULL) {
    field = Make_Geo3d_Scalar_Field(GEO3D_BALL_FIELD_SIZE);
  }

  geo3d_ball_field_u(field);
  int i;
  for (i = 0; i < GEO3D_BALL_FIELD_SIZE; i++) {
    field->points[i][0] *= ball->r;
    field->points[i][1] *= ball->r;
    field->points[i][2] *= ball->r;
    
    field->points[i][0] += ball->center[0];
    field->points[i][1] += ball->center[1];
    field->points[i][2] += ball->center[2];
  }

  return field;
}

#define GEO3D_BALL_MEAN_SHIFT_EPS 0.01

void Geo3d_Ball_Mean_Shift(Geo3d_Ball *ball, Stack *stack, double z_scale,
    double min_shift)
{
  coordinate_3d_t old_center;
  Coordinate_3d_Copy(old_center, ball->center);

  int check_count = 5;
  int count = 0;
  double eps = 1.0;

  Geo3d_Scalar_Field *field = Geo3d_Ball_Field(ball, NULL);
  while (eps > min_shift) {
    Geo3d_Scalar_Field_Stack_Sampling(field, stack, z_scale, field->values);
    Geo3d_Scalar_Field_Centroid(field, ball->center);
    Geo3d_Ball_Field(ball, field);
    count++;
    if (count > check_count) {
      count = 0;
      eps = Coordinate_3d_Distance(old_center, ball->center);
      Coordinate_3d_Copy(old_center, ball->center);
    }
  }
  Kill_Geo3d_Scalar_Field(field);
}

void Geo3d_Ball_Label_Stack(const Geo3d_Ball *ball, Stack *stack, 
    Geo3d_Ball_Label_Workspace *ws)
{
  Cuboid_I cuboid;

  int i;
  for (i = 0; i < 3; ++i) {
    cuboid.cb[i] = imax2(0, (int) ceil(ball->center[i] - ball->r));
    cuboid.ce[i] = floor(ball->center[i] + ball->r);
  }
  int width = Stack_Width(stack);
  int height = Stack_Height(stack);
  int depth = Stack_Depth(stack);

  cuboid.ce[0] = imin2(cuboid.ce[0], width - 1);
  cuboid.ce[1] = imin2(cuboid.ce[1], height - 1);
  cuboid.ce[2] = imin2(cuboid.ce[2], depth - 1);
  
  int j, k;
  /*
  size_t offset = cuboid.cb[0];
  */
  /*
  offset = Stack_Util_Offset(cuboid.cb[0], cuboid.cb[1], cuboid.cb[2],
      width, height, depth);

  int p2 = width - cuboid.ce[0] + cuboid.cb[0];
  int p3 = p2 + width * height - width * (cuboid.ce[1] - cuboid.ce[1] + 1);
*/
  for (k = cuboid.cb[2]; k <= cuboid.ce[2]; ++k) {
    for (j = cuboid.cb[1]; j <= cuboid.ce[1]; ++j) {
      for (i = cuboid.cb[0]; i <= cuboid.ce[0]; ++i) {
        double dx = (double) i - ball->center[0];
        double dy = (double) j - ball->center[1];
        double dz = (double) k - ball->center[2];
        if (dx * dx + dy * dy + dz * dz <= ball->r * ball->r) {
          Set_Stack_Pixel(stack, i, j, k, 0, ws->sdw.color.r); 
          if (Stack_Kind(stack) == COLOR) {
            Set_Stack_Pixel(stack, i, j, k, 1, ws->sdw.color.g); 
            Set_Stack_Pixel(stack, i, j, k, 2, ws->sdw.color.b); 
          }
        }
        //++offset;
      }
      //offset += p2;
    }
    //offset += p3;
  }
}

void Default_Geo3d_Ball_Label_Workspace(Geo3d_Ball_Label_Workspace *ws)
{
  Default_Stack_Draw_Workspace(&(ws->sdw));
}
