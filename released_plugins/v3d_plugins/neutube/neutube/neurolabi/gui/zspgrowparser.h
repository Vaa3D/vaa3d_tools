#ifndef ZSPGROWPARSER_H
#define ZSPGROWPARSER_H

#include <vector>
#include "tz_sp_grow.h"

#include "zvoxel.h"
#include "zvoxelarray.h"

class ZSpGrowParser
{
public:
  ZSpGrowParser();
  ZSpGrowParser(Sp_Grow_Workspace *workspace);
  ~ZSpGrowParser();

public:
  size_t voxelNumber();
  Stack* createDistanceStack();
  ZVoxelArray extractPath(ssize_t index);
  ZVoxelArray extractLongestPath(double *length);
  int pathSize(ssize_t index);
  double pathLength(ssize_t index);
  std::vector<ZVoxelArray> extractAllPath(double lengthThreshold,
                                          Stack *ballStack = NULL);

private:
  Sp_Grow_Workspace *m_workspace;
  Stack *m_regionMask;
  Stack *m_checkedMask;
  Stack *m_pathMask;
  std::vector<size_t> fgArray;

};

#endif // ZSPGROWPARSER_H
