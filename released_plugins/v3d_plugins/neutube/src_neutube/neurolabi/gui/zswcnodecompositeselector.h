#ifndef ZSWCNODECOMPOSITESELECTOR_H
#define ZSWCNODECOMPOSITESELECTOR_H

#include <vector>
#include "zswcnodeselector.h"

class ZSwcNodeCompositeSelector : public ZSwcNodeSelector
{
public:
  ZSwcNodeCompositeSelector();
  ~ZSwcNodeCompositeSelector();

  void addSelector(ZSwcNodeSelector *selector);

  enum ECompositeMode {
    AND, OR
  };

public:
  bool isSelected(const Swc_Tree_Node *tn);

private:
  std::vector<ZSwcNodeSelector*> m_selectorList;
  ECompositeMode m_compositeMode;
};

#endif // ZSWCNODECOMPOSITESELECTOR_H
