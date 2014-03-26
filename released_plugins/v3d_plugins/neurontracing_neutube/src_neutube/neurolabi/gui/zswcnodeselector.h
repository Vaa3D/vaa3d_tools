#ifndef ZSWCNODESELECTOR_H
#define ZSWCNODESELECTOR_H

#include "zswctree.h"

/*!
 * \brief Base class for selecting swc nodes
 */
class ZSwcNodeSelector
{
public:
  ZSwcNodeSelector();
  virtual ~ZSwcNodeSelector();

  inline void excludeVirtualNode(bool excluding) {
    m_excludingVirtual = excluding;
  }

  /*!
   * \brief Select nodes from a tree
   * \param The input SWC tree
   * \return A set of nodes selected from \a tree
   * \sa antiSelectFrom
   */
  std::vector<Swc_Tree_Node*> selectFrom(const ZSwcTree &tree);

  /*!
   * \brief Reverse mode of \a selectFrom
   *
   * antiSelectFrom() select a node when \a isSelected is false. No
   * virtual node will be selected if it is excluded
   * (set by \a excludeVirtualNode()).
   *
   * \param The input SWC tree
   * \return A set of nodes selected from \a tree
   * \sa selectFrom
   */
  std::vector<Swc_Tree_Node*> antiSelectFrom(const ZSwcTree &tree);

public:
  virtual bool isSelected(const Swc_Tree_Node *tn) = 0;

private:
  bool m_excludingVirtual;
};

#endif // ZSWCNODESELECTOR_H
