#ifndef ZSWCRESAMPLER_H
#define ZSWCRESAMPLER_H

#include "tz_swc_tree.h"

class ZSwcTree;

/*!
 * \brief A class for resampling swc tree
 */
class ZSwcResampler
{
public:
  ZSwcResampler();

  /*!
   * \brief suboptimalDownsample
   *
   * \return Number of nodes that are removed
   */
  int suboptimalDownsample(ZSwcTree *tree);

  /*!
   * \brief optimalDownsample
   * \param tree
   * \return Number of nodes that are removed
   */
  void optimalDownsample(ZSwcTree *tree);

private:
  /*!
   * \brief Optimize continuous parents of critical nodes
   */
  int optimizeCriticalParent(ZSwcTree *tree);

  /*!
   * \brief Check if a node is inter-redundant
   *
   * \a tn is inter-redundant iff \a tn:
   *   1) is a continuation; and
   *   2) has overlap with \a master, which must be the parent or child of \a tn;
   *      and
   *   3) is close to its interpolation replacement.
   */
   bool isInterRedundant(const Swc_Tree_Node *tn, const Swc_Tree_Node *master) const;
};

#endif // ZSWCRESAMPLER_H
