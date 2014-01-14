#ifndef ZVOXEL_H
#define ZVOXEL_H

#include <stdlib.h>
#include "tz_stack_lib.h"

class ZVoxel
{
public:
  ZVoxel();
  ZVoxel(int x, int y, int z, double value = 0.0);

  inline int x() const { return m_x; }
  inline int y() const { return m_y; }
  inline int z() const { return m_z; }
  inline double value() const { return m_value; }

  inline void set(int x, int y, int z, double value = 0.0) {
    m_x = x; m_y = y; m_z = z; m_value = value;
  }

  inline void setValue(double value) { m_value = value; }

  double distanceTo(const ZVoxel &voxel) const;

  void setFromIndex(size_t index, int width, int height);

  ssize_t toIndex(int width, int height, int depth) const;

  bool isInBound(int width, int height, int depth) const;

  void sample(const Stack *stack);
  void sample(const Stack *stack, double (*f)(double));
  void setStackValue(Stack *stack) const;

  void labelStackWithBall(Stack *stack, int label) const;

  ZVoxel& operator=(const ZVoxel &voxel);

  /*!
   * \brief Translate a voxel.
   *
   * Translates voxel by (\a dx, \a dy, \a dz).
   */
  void translate(int dx, int dy, int dz);

private:
  int m_x;
  int m_y;
  int m_z;
  double m_value;
};

#endif // ZVOXEL_H
