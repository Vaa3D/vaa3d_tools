#include "zswcnodeellipsoidrangeselector.h"
#include "swctreenode.h"

ZSwcNodeEllipsoidRangeSelector::ZSwcNodeEllipsoidRangeSelector() :
  m_expandFactor(0.0)
{
}

void ZSwcNodeEllipsoidRangeSelector::setRange(const ZEllipsoid &ellipsoid)
{
  m_ellipsoid = ellipsoid;
}

void ZSwcNodeEllipsoidRangeSelector::setRange(
    const ZPoint &center, double rx, double ry, double rz)
{
  m_ellipsoid.setCenter(center);
  m_ellipsoid.setSize(rx, ry, rz);
}

bool ZSwcNodeEllipsoidRangeSelector::isSelected(const Swc_Tree_Node *tn)
{
  ZEllipsoid ellipsoid = m_ellipsoid;
  ellipsoid.setSize(
        ellipsoid.getXRadius() + m_expandFactor * sqrt(ellipsoid.getXRadius()),
        ellipsoid.getYRadius() + m_expandFactor * sqrt(ellipsoid.getYRadius()),
        ellipsoid.getZRadius() + m_expandFactor * sqrt(ellipsoid.getZRadius()));

  //ellipsoid.print();
  return ellipsoid.containsPoint(SwcTreeNode::x(tn), SwcTreeNode::y(tn),
                                   SwcTreeNode::z(tn));
}
