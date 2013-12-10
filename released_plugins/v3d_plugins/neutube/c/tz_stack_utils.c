/* tz_stack_utils.c
 *
 * 09-Jan-2008 Initial write: Ting Zhao
 */

#include <math.h>
#include "tz_utilities.h"
#include "tz_stack_utils.h"

ssize_t Stack_Util_Offset(int x, int y, int z, int width, int height, int depth)
{
  if ((x < 0) || (y < 0) || (z < 0) || 
      (x >= width) || (y >= height) || (z >= depth)) {
#ifdef _DEBUG_2
    printf("invalid coordinate: %d %d %d\n", x, y, z);
#endif
    return -1;
  }

  return STACK_UTIL_OFFSET(x, y, z, width, width * height);
}

void Stack_Util_Coord(size_t index, int width, int height, 
		      int *x, int *y, int *z)
{
  int area = width * height;
  
  *z = index / area;
  *y = index % area;
  *x = *y % width;
  *y /= width;
}

void Stack_Util_Coord_A(size_t index, int width, int area, 
			int *x, int *y, int *z)
{
  *z = index / area;
  *y = index % area;
  *x = *y % width;
  *y = *y / width;
}

double Stack_Util_Voxel_Distance(size_t index1, size_t index2, 
				 int width, int height)
{
  if (index1 == index2) {
    return 0;
  }

  int tmp;

  if (index1 < index2) {
    SWAP2(index1, index2, tmp);
  }

  int area = width * height;

  int dz = (index1 - index2) / area;
  
  index1 %= area;
  index2 %= area;

  if (index1 == index2) {
#ifdef _DEBUG_
    printf("%d, %d, %d\n", 0, 0, dz);
#endif
    return dz;
  }
  
  int dx, dy;

  if (index1 < index2) {
    dz++;
    SWAP2(index1, index2, tmp);
  }

  dy = (index1 - index2) / width;

  index1 %= width;
  index2 %= width;

  if (index1 == index2) {
#ifdef _DEBUG_
    printf("%d, %d, %d\n", 0, dy, dz);
#endif
    return sqrt(dz * dz + dy * dy);
  }

  if (index1 < index2) {
    dy++;
  }

  dx = index2 - index1;

#ifdef _DEBUG_
    printf("%d, %d, %d\n", dx, dy, dz);
#endif
  return sqrt(dx * dx + dy * dy + dz * dz);
}


size_t Stack_Subindex(size_t sindex, int x0, int y0, int z0, 
		      int swidth, int sarea, int width, int area)
{
  int x, y, z;
  STACK_UTIL_COORD(sindex, swidth, sarea, x, y, z);
  x += x0;
  y += y0;
  z += z0;

  return STACK_UTIL_OFFSET(x, y, z, width, area);
}

uint32_t Color_To_Value(const uint8_t* color)
{
  uint32_t value = color[2];
  value *= 256;
  value += color[1];
  value *= 256;
  value += color[0];

  return value;
}

uint8_t Value_To_Color(uint32_t value, uint8_t* color)
{
  uint8_t overflow = 0;

  overflow = value >> 24;

  value <<= 8;
  value >>= 8;

  int color2 = value / 256;
  int color1 = color2 % 256;
  color2 /= 256;
  /*
     if (color2 > 255) {
     success = FALSE;
     }
     */
  int color0 = value % 256;
  color[0] = color0;
  color[1] = color1;
  color[2] = color2;

  return overflow;
}
