#ifndef ZVOXELARRAY_H
#define ZVOXELARRAY_H

#include <vector>
#include "zvoxel.h"
#include "tz_stack_lib.h"
#include "tz_swc_tree.h"

/*!
 * \brief The voxel array class
 */
class ZVoxelArray : public std::vector<ZVoxel>
{
public:
  ZVoxelArray();

public:
  /*!
   * \brief Append a voxel to the back
   * \param voxel The voxel to be added
   */
  void append(const ZVoxel &voxel);

  /*!
   * \brief prepend Add a voxel to the front
   * \param voxel The voxel to be added
   */
  void prepend(const ZVoxel &voxel);

  /*!
   * \brief addValue Add value of each pixel.
   *
   * Add the value of each pixel by \a delta.
   *
   * \param delta The value to add.
   */
  void addValue(double delta);

  /*!
   * \brief minimizeValue Set the bound of the voxel values.
   *
   * The value of a voxel is set to \a v if it is less than \a v.
   *
   * \param v The cutting value.
   */
  void minimizeValue(double v);


  inline double value(size_t index) { return (*this)[index].value(); }
  inline void setValue(size_t index, double v) {
    (*this)[index].setValue(v);
  }

  size_t findClosest(double x, double y);
  size_t findClosest(double x, double y, double z);

  //untested
  /*!
   * \brief Compute the curve length of the voxel Array
   *
   * It assumes that voxels form a polyline.
   *
   * \return The total length of the polyline.
   */
  double getCurveLength() const;

public:
  void setStackValue(Stack *stack) const;
  void labelStack(Stack *stack, double value) const;
  void labelStackWithBall(Stack *stack, double value) const;
  void sample(const Stack *stack);
  void sample(const Stack *stack, double (*f)(double));
  Swc_Tree *toSwcTree() const;
  Swc_Tree *toSwcTree(size_t startIndex, size_t endIndex) const;
};

#endif // ZVOXELARRAY_H
