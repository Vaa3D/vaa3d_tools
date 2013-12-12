#ifndef ZSEGMENTMAPARRAY_H
#define ZSEGMENTMAPARRAY_H

#include <vector>
#include <map>
#include <string>
#include "zsegmentmap.h"

class ZSegmentMapArray : public std::vector<ZSegmentMap>
{
public:
  ZSegmentMapArray();
  int segmentToBody(int segmentId) const;
  std::vector<int> bodyToSegment(int bodyId);
  int load(std::string filePath);
  void print() const;

private:
  std::map<int, int> m_segmentToBodyMap;

};

#endif // ZSEGMENTMAPARRAY_H
