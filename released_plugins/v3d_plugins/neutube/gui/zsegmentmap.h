#ifndef ZSEGMENTMAP_H
#define ZSEGMENTMAP_H

class ZSegmentMap
{
public:
  ZSegmentMap();
  ZSegmentMap(int segmentId, int bodyId);
  void set(int segmentId, int bodyId);
  inline int segmentId() const { return m_segmentId; }
  inline int bodyId() const { return m_bodyId; }

  void print() const;

private:
  int m_segmentId;
  int m_bodyId;
};

#endif // ZSEGMENTMAP_H
