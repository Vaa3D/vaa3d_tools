#include "zsegmentmap.h"
#include <iostream>

using namespace std;

ZSegmentMap::ZSegmentMap()
{
  set(-1, -1);
}

ZSegmentMap::ZSegmentMap(int segmentId, int bodyId)
{
  set(segmentId, bodyId);
}

void ZSegmentMap::set(int segmentId, int bodyId)
{
  m_segmentId = segmentId;
  m_bodyId = bodyId;
}

void ZSegmentMap::print() const
{
  cout << m_segmentId << ", " << m_bodyId << endl;
}
