#include "zswcnodecompositeselector.h"

using namespace std;

ZSwcNodeCompositeSelector::ZSwcNodeCompositeSelector() :
  m_compositeMode(ZSwcNodeCompositeSelector::OR)
{
}

ZSwcNodeCompositeSelector::~ZSwcNodeCompositeSelector()
{
  for (vector<ZSwcNodeSelector*>::iterator iter = m_selectorList.begin();
       iter != m_selectorList.end(); ++iter) {
    delete (*iter);
  }
}

bool ZSwcNodeCompositeSelector::isSelected(const Swc_Tree_Node *tn)
{
  switch (m_compositeMode) {
  case AND:
    for (vector<ZSwcNodeSelector*>::iterator iter = m_selectorList.begin();
         iter != m_selectorList.end(); ++iter) {
      if (!(*iter)->isSelected(tn)) {
        return false;
      }
    }

    return true;
  case OR:
    for (vector<ZSwcNodeSelector*>::iterator iter = m_selectorList.begin();
         iter != m_selectorList.end(); ++iter) {
      if ((*iter)->isSelected(tn)) {
        return true;
      }
    }

    return false;
  }

  return false;
}

void ZSwcNodeCompositeSelector::addSelector(ZSwcNodeSelector *selector)
{
  m_selectorList.push_back(selector);
}
