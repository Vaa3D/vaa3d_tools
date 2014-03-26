#include "zswcnodezrangeselector.h"
#include "swctreenode.h"

ZSwcNodeZRangeSelector::ZSwcNodeZRangeSelector() :
  m_minZ(0.0), m_maxZ(0.0)
{
}

void ZSwcNodeZRangeSelector::setZRange(double minZ, double maxZ)
{
  m_minZ = minZ;
  m_maxZ = maxZ;
}

bool ZSwcNodeZRangeSelector::isSelected(const Swc_Tree_Node *tn)
{
  return SwcTreeNode::isRegular(tn) && SwcTreeNode::z(tn) <= m_maxZ &&
      SwcTreeNode::z(tn) >= m_minZ;
}
