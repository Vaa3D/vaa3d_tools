#include "zvoxel.h"
#include <assert.h>
#include <math.h>
#include "tz_stack_utils.h"
#include "tz_stack_lib.h"
#include "tz_utilities.h"
#include "tz_math.h"
#include "tz_stack_attribute.h"

ZVoxel::ZVoxel()
{
  set(0.0, 0.0, 0.0, 0.0);
}

ZVoxel::ZVoxel(int x, int y, int z, double value)
{
  set(x, y, z, value);
}

double ZVoxel::distanceTo(const ZVoxel &voxel) const
{
  int dx = m_x - voxel.x();
  int dy = m_y - voxel.y();
  int dz = m_z - voxel.z();

  return sqrt(dx * dx + dy * dy + dz * dz);
}

ssize_t ZVoxel::toIndex(int width, int height, int depth) const
{
  return Stack_Util_Offset(m_x, m_y, m_z, width, height, depth);
}

bool ZVoxel::isInBound(int width, int height, int depth) const
{
  return ((m_x >= 0) && (m_x < width) && (m_y >= 0) && (m_y < height) &&
          (m_z >= 0) && (m_z < depth));
}

void ZVoxel::setFromIndex(size_t index, int width, int height)
{
  Stack_Util_Coord(index, width, height, &m_x, &m_y, &m_z);
}

void ZVoxel::sample(const Stack *stack)
{
  m_value = Get_Stack_Pixel((Stack*) stack, m_x, m_y, m_z, 0);
}

void ZVoxel::sample(const Stack *stack, double (*f)(double))
{
  m_value = f(Get_Stack_Pixel((Stack*) stack, m_x, m_y, m_z, 0));
}

void ZVoxel::setStackValue(Stack *stack) const
{
  Set_Stack_Pixel(stack, m_x, m_y, m_z, 0, m_value);
}

void ZVoxel::labelStackWithBall(Stack *stack, int label) const
{
  int width = Stack_Width(stack);
  int height = Stack_Height(stack);
  int depth = Stack_Depth(stack);

  assert(Stack_Kind(stack) == GREY);
  assert(isInBound(width, height, depth));

  int r = iround(m_value);
  int zRange = r;
  int zStart = imax2(0, m_z - zRange);
  int zEnd = imin2(m_z + zRange, Stack_Depth(stack) - 1);

  uint8_t *zArrayStart = stack->array + Stack_Util_Offset(m_x, m_y, zStart,
                                                         width, height, depth);
  uint8_t v = (uint8_t) label;
  int area = width * height;

  for (int z = zStart; z <= zEnd; z++) {
    int dz = abs(z - m_z);
    double y_r = sqrt(m_value * m_value - dz * dz);
    int yRange = iround(y_r);
    int yStart = imax2(0, m_y - yRange);
    int yEnd = imin2(m_y + yRange, height - 1);
    uint8_t *yArrayStart = zArrayStart - width * imin2(m_y, yRange);

    for (int y = yStart; y <= yEnd; y++) {
      int dy = abs(y - m_y);
      int xRange = iround(sqrt(y_r * y_r - dy * dy));
      int xStart = imax2(0, m_x - xRange);
      int xEnd = imin2(m_x + xRange, width - 1);
      uint8_t *xArrayStart = yArrayStart - imin2(m_x, xRange);
      for (int x = xStart; x <= xEnd; x++) {
        *(xArrayStart++) = v;
      }
      yArrayStart += width;
    }
    zArrayStart += area;
  }
}

ZVoxel& ZVoxel::operator =(const ZVoxel &voxel)
{
  set(voxel.x(), voxel.y(), voxel.z(), voxel.value());

  return *this;
}

void ZVoxel::translate(int dx, int dy, int dz)
{
  m_x += dx;
  m_y += dy;
  m_z += dz;
}
