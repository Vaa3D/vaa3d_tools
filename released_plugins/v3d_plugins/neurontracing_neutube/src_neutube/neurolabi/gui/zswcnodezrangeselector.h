#ifndef ZSWCNODEZRANGESELECTOR_H
#define ZSWCNODEZRANGESELECTOR_H

#include "zswcnodeselector.h"

class ZSwcNodeZRangeSelector : public ZSwcNodeSelector
{
public:
  ZSwcNodeZRangeSelector();

  void setZRange(double minZ, double maxZ);

  virtual bool isSelected(const Swc_Tree_Node *tn);

private:
  double m_minZ;
  double m_maxZ;
};

#endif // ZSWCNODEZRANGESELECTOR_H
