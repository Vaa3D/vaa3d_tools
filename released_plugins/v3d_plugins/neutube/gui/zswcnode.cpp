#include "zswcnode.h"

ZSwcNode::ZSwcNode()
{
  ZSwcNode(0, NEURITE, 0.0, 0.0, 0.0, 1.0);
}

ZSwcNode::ZSwcNode(int id, EType type, double x, double y, double z, double r) :
  ZBall(x, y, z, r)
{
  m_id = id;
  m_type = type;
}
