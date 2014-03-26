#include <stdlib.h>
#include "tz_cuboid_i.h"
#include "tz_utilities.h"
#include "tz_error.h"

DEFINE_ZOBJECT_INTERFACE(Cuboid_I)

void Default_Cuboid_I(Cuboid_I *cuboid)
{
  cuboid->cb[0] = 0;
  cuboid->cb[1] = 0;
  cuboid->cb[2] = 0;
  cuboid->ce[0] = 0;
  cuboid->ce[1] = 0;
  cuboid->ce[2] = 0;
}

void Clean_Cuboid_I(Cuboid_I *cuboid)
{
  cuboid->cb[0] = 0;
  cuboid->cb[1] = 0;
  cuboid->cb[2] = 0;
  cuboid->ce[0] = 0;
  cuboid->ce[1] = 0;
  cuboid->ce[2] = 0;  
}

void Print_Cuboid_I(Cuboid_I *cuboid)
{
  printf("Cuboid: (%d, %d, %d) -> (%d, %d, %d)\n", 
	 cuboid->cb[0], cuboid->cb[1], cuboid->cb[2],
	 cuboid->ce[0], cuboid->ce[1], cuboid->ce[2]);
}

void Cuboid_I_Size(const Cuboid_I *cuboid, int *width, int *height, int *depth)
{
  if (width != NULL) {
    *width = cuboid->ce[0] - cuboid->cb[0] + 1;
  }

  if (height != NULL) {
    *height = cuboid->ce[1] - cuboid->cb[1] + 1;
  }

  if (depth != NULL) {
    *depth = cuboid->ce[2] - cuboid->cb[2] + 1;
  }
}

void Cuboid_I_Set_S(Cuboid_I *cuboid, int x, int y, int z, int width, 
		    int height, int depth)
{
  cuboid->cb[0] = x;
  cuboid->cb[1] = y;
  cuboid->cb[2] = z;
  cuboid->ce[0] = x + width - 1;
  cuboid->ce[1] = y + height - 1;
  cuboid->ce[2] = z + depth - 1;
}

BOOL Cuboid_I_Is_Valid(const Cuboid_I *cuboid)
{
  if ((cuboid->cb[0] <= cuboid->ce[0]) && (cuboid->cb[1] <= cuboid->ce[1]) &&
      (cuboid->cb[2] <= cuboid->ce[2])) {
    return TRUE;
  }

  return FALSE;
}

int Cuboid_I_Volume(const Cuboid_I *cuboid)
{
  int width, height, depth;
  Cuboid_I_Size(cuboid, &width, &height, &depth);
  
  int v = width * height *depth;
  if ((Cuboid_I_Is_Valid(cuboid) == FALSE) && (v > 0)) {
    v = -v;
  }
						       
  return v;
}

Cuboid_I* Cuboid_I_Intersect(const Cuboid_I *c1, const Cuboid_I *c2, 
			     Cuboid_I *c3)
{
  if (c3 == NULL) {
    c3 = New_Cuboid_I();
  }

  int i;
  for (i = 0; i < 3; i++) {
    c3->cb[i] = imax2(c1->cb[i], c2->cb[i]);
    c3->ce[i] = imin2(c1->ce[i], c2->ce[i]);
  }

  return c3;
}

Cuboid_I* Cuboid_I_Union(const Cuboid_I *c1, const Cuboid_I *c2, Cuboid_I *c3)
{
  if (c3 == NULL) {
    c3 = New_Cuboid_I();
  }

  int i;
  for (i = 0; i < 3; i++) {
    c3->cb[i] = imin2(c1->cb[i], c2->cb[i]);
    c3->ce[i] = imax2(c1->ce[i], c2->ce[i]);
  }

  return c3;
}

int Cuboid_I_Overlap_Volume(const Cuboid_I *c1, const Cuboid_I *c2)
{
  Cuboid_I c3;
  Cuboid_I_Intersect(c1, c2, &c3);

  return Cuboid_I_Volume(&c3);
}

void Cuboid_I_Expand_P(Cuboid_I *cuboid, const int *pt)
{
  int i;
  for (i = 0; i < 3; i++) {
    if (cuboid->cb[i] > pt[i]) {
      cuboid->cb[i] = pt[i];
    }
    if (cuboid->ce[i] < pt[i]) {
      cuboid->ce[i] = pt[i];
    }
  }
}

void Cuboid_I_Label_Stack(const Cuboid_I *cuboid, int v, Stack *stack)
{
  TZ_ASSERT(stack->kind == GREY, "Unsupported stack");

  Cuboid_I in_cuboid;
  Cuboid_I stack_cuboid;
  Cuboid_I_Set_S(&stack_cuboid, 0, 0, 0, stack->width, stack->height,
      stack->depth);
  
  Cuboid_I_Intersect(cuboid, &stack_cuboid, &in_cuboid);
  int x, y, z;
  for (z = in_cuboid.cb[2]; z <= in_cuboid.ce[2]; ++z) {
    for (y = in_cuboid.cb[1]; y <= in_cuboid.ce[1]; ++y) {
      for (x = in_cuboid.cb[0]; x <= in_cuboid.ce[0]; ++x) {
        Set_Stack_Pixel(stack, x, y, z, 0, v);
      }
    }
  }
}

BOOL Cuboid_I_Hit(const Cuboid_I *cuboid, int x, int y, int z)
{
  return IS_IN_CLOSE_RANGE3(x, y, z, cuboid->cb[0], cuboid->ce[0],
      cuboid->cb[1], cuboid->ce[1], cuboid->cb[2], cuboid->ce[2]);
}

BOOL Cuboid_I_Hit_Internal(const Cuboid_I *cuboid, int x, int y, int z)
{
  return IS_IN_OPEN_RANGE3(x, y, z, cuboid->cb[0], cuboid->ce[0],
      cuboid->cb[1], cuboid->ce[1], cuboid->cb[2], cuboid->ce[2]);
}

void Cuboid_I_Expand_X(Cuboid_I *cuboid, int margin)
{
  cuboid->cb[0] -= margin;
  cuboid->ce[0] += margin;
}

void Cuboid_I_Expand_Y(Cuboid_I *cuboid, int margin)
{
  cuboid->cb[1] -= margin;
  cuboid->ce[1] += margin;
}

void Cuboid_I_Expand_Z(Cuboid_I *cuboid, int margin)
{
  cuboid->cb[2] -= margin;
  cuboid->ce[2] += margin;
}
