#ifndef ZSUPERPIXELMAPARRAY_H
#define ZSUPERPIXELMAPARRAY_H

#include <vector>
#include <map>
#include <string>
#include "zsuperpixelmap.h"
#include "zintmap.h"
#include "zstack.hxx"

class ZSuperpixelMapArray : public std::vector<ZSuperpixelMap>
{
public:
  ZSuperpixelMapArray();

public:
  int superpixelToSegment(int superpixelId);
  std::vector<int> segmentToSuperpixel(int segmentId);
  std::vector<int> segmentToSuperpixel(const std::vector<int> &segmentId);
  int load(const std::string &filePath, int planeId = -1);
  void setBodyId(const ZIntMap &segMapArray);

  ZStack* mapStack(ZStack &stack);
  ZStack* mapBoundaryStack(ZStack &stack);

  void compressBodyId(int startId = 1);

  void append(int planeId, int superpixelId, int segmentId, int bodyId);

  int minPlaneId();
public:
  void print() const;

private:
  //std::map<int, int> m_superpixelToSegmentMap;
};

#endif // ZSUPERPIXELMAPARRAY_H
