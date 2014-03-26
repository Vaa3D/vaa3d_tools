#ifndef ZSWCNODEELLIPSOIDRANGESELECTOR_H
#define ZSWCNODEELLIPSOIDRANGESELECTOR_H

#include "zswcnodeselector.h"
#include "zellipsoid.h"
#include "zpoint.h"

class ZSwcNodeEllipsoidRangeSelector : public ZSwcNodeSelector
{
public:
  ZSwcNodeEllipsoidRangeSelector();
  void setRange(const ZEllipsoid &ellipsoid);
  void setRange(const ZPoint& center, double rx, double ry, double rz);

  inline void setExpandFactor(double f) { m_expandFactor = f; }

protected:
  bool isSelected(const Swc_Tree_Node *tn);

private:
  ZEllipsoid m_ellipsoid;
  double m_expandFactor;
};

#endif // ZSWCNODEELLIPSOIDRANGESELECTOR_H
