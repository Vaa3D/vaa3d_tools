#include "zsuperpixelmap.h"
#include <iostream>

using namespace std;

ZSuperpixelMap::ZSuperpixelMap()
{
  set(-1, -1, -1);
}

ZSuperpixelMap::ZSuperpixelMap(int planeId, int superpixelId, int segmentId,
                               int bodyId)
{
  set(planeId, superpixelId, segmentId, bodyId);
}

void ZSuperpixelMap::set(int planeId, int superpixelId, int segmentId,
                         int bodyId)
{
  m_planeId = planeId;
  m_superpixelId = superpixelId;
  m_segmentId = segmentId;
  m_bodyId = bodyId;
}

void ZSuperpixelMap::print() const
{
  cout << m_planeId << ", " << m_superpixelId << ", " << m_segmentId <<
          ", " << m_bodyId << endl;
}
