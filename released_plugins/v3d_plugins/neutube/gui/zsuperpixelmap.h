#ifndef ZSUPERPIXELMAP_H
#define ZSUPERPIXELMAP_H

class ZSuperpixelMap
{
public:
  ZSuperpixelMap();
  ZSuperpixelMap(int planeId, int superpixelId, int segmentId, int bodyId = -1);

  void set(int planeId, int superpixelId, int segmentId, int bodyId = -1);

  inline int planeId() const { return m_planeId; }
  inline int superpixelId() const { return m_superpixelId; }
  inline int segmentId() const { return m_segmentId; }
  inline int bodyId() const { return m_bodyId; }

  inline void setBodyId(int bodyId) { m_bodyId = bodyId; }

public:
  void print() const;

private:
  int m_planeId;
  int m_superpixelId;
  int m_segmentId;
  int m_bodyId;
};

#endif // ZSUPERPIXELMAP_H
