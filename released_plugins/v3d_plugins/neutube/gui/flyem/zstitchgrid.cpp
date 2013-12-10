#include "zstitchgrid.h"
#include <math.h>
#include "tz_math.h"

FlyEm::ZStitchGrid::ZStitchGrid()
{
  for (int i = 0; i < 3; ++i) {
    m_interval[i] = 0;
    m_start[i] = 0;
    m_gridNumber[i] = 0;
  }
}

FlyEm::ZStitchGrid::ZStitchGrid(const ZStitchGrid &grid)
{
  for (int i = 0; i < 3; ++i) {
    m_interval[i] = grid.m_interval[i];
    m_start[i] = grid.m_start[i];
    m_gridNumber[i] = grid.m_gridNumber[i];
  }
}

void FlyEm::ZStitchGrid::setInterval(int ix, int iy, int iz)
{
  m_interval[0] = ix;
  m_interval[1] = iy;
  m_interval[2] = iz;
}

void FlyEm::ZStitchGrid::setStart(int sx, int sy, int sz)
{
  m_start[0] = sx;
  m_start[1] = sy;
  m_start[2] = sz;
}

void FlyEm::ZStitchGrid::setGridNumber(int nx, int ny, int nz)
{
  m_gridNumber[0] = nx;
  m_gridNumber[1] = ny;
  m_gridNumber[2] = nz;
}

int FlyEm::ZStitchGrid::getBlockNumber()
{
  if (m_gridNumber[0] < 2 || m_gridNumber[1] < 2 || m_gridNumber[2] < 2) {
    return 0;
  }

  return (m_gridNumber[0] - 1) * (m_gridNumber[1] - 1) * (m_gridNumber[2] - 1);
}

void FlyEm::ZStitchGrid::getBlock(int index, Cuboid_I *cuboid)
{
  int bwidth = m_gridNumber[0] - 1;
  int bheight = m_gridNumber[1] - 1;
  int barea = bwidth * bheight;
  int bCoord[3];
  bCoord[2] = index / barea;
  bCoord[1] = index % barea / bwidth;
  bCoord[0] = index % bwidth;

  for (int i = 0; i < 3; ++i) {
    cuboid->cb[i] = bCoord[i] * m_interval[i] + m_start[i];
    cuboid->ce[i] = cuboid->cb[i] + m_interval[i] - 1;
  }
}

int FlyEm::ZStitchGrid::hitTest(double x, double y, double z)
{
  x -= m_start[0];
  y -= m_start[1];
  z -= m_start[2];

  if (x < 0 || x > getWidth() || y < 0 || y > getHeight() ||
      z < 0 || z > getDepth()) {
    return -1;
  }

  int bx = floor(x) / m_interval[0];
  int by = floor(y) / m_interval[1];
  int bz = floor(z) / m_interval[2];

  int bwidth = m_gridNumber[0] - 1;
  int bheight = m_gridNumber[1] - 1;
  int barea = bwidth * bheight;


  return bz * barea + by * bheight + bx;
}
