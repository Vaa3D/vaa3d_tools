#ifndef ZSTITCHGRID_H
#define ZSTITCHGRID_H

#include "tz_cuboid_i.h"

namespace FlyEm {
/*!
 * \brief A class of a 3D grid
 *
 * The grid defines a 3D matrix of blocks, each with the size of the grid
 * interval.
 */
class ZStitchGrid
{
public:
  ZStitchGrid();
  ZStitchGrid(const ZStitchGrid &grid);

  /*!
   * \brief Set the grid intervals
   *
   * \param ix interval of X grid
   * \param iy interval of Y grid
   * \param iz interval of Z grid
   */
  void setInterval(int ix, int iy, int iz);

  /*!
   * \brief setStart
   *
   * \param sx
   * \param sy
   * \param sz
   */
  void setStart(int sx, int sy, int sz);

  /*!
   * \brief The the number of grid lines
   *
   * The number of grid lines counts from the first border line to the last
   * border line, including the border lines.
   *
   * \param nx Number of grid lines along X
   * \param ny Number of grid lines along Y
   * \param nz Number of grid lines along Z
   */
  void setGridNumber(int nx, int ny, int nz);

  /*!
   * \brief Get the number of blocks
   *
   * The number of block is defines as (nx - 1) * (ny - 1) * (nz - 1), where
   * nx, ny and nz are grid lines along X, Y and Z axes.
   *
   * \return The number of blocks
   */
  int getBlockNumber();

  /*!
   * \brief Get the bounding box of a block
   *
   * \param index Index of the block
   * \param cuboid Structure to store the result.
   */
  void getBlock(int index, Cuboid_I *cuboid);

  /*!
   * \brief Test which block a point is located.
   *
   * \param x X coordinate of the point
   * \param y Y coordinate of the point
   * \param z Z corrdinate of the point
   *
   * \return The index of the block.
   */
  int hitTest(double x, double y, double z);

  inline int getSize(int index)
  {
    return (m_gridNumber[index] > 1)
        ? m_interval[index] * (m_gridNumber[index] - 1) : 0;
  }

  inline int getWidth() { return getSize(0); }
  inline int getHeight() { return getSize(1); }
  inline int getDepth() { return getSize(2); }

private:
  int m_interval[3];
  int m_start[3];
  int m_gridNumber[3];
};
}

#endif // ZSTITCHGRID_H
