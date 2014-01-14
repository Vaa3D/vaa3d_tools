#include "zswcrangeanalyzer.h"
#include "swctreenode.h"
#include "zswcnodezrangeselector.h"
#include "zellipsoid.h"
#include "zswcnodecompositeselector.h"
#include "zswcnodeellipsoidrangeselector.h"

ZSwcRangeAnalyzer::ZSwcRangeAnalyzer() : m_zStep(10), m_zMargin(5)
{
}


std::vector<ZEllipsoid> ZSwcRangeAnalyzer::computeLayerRange(const ZSwcTree &tree)
{
  std::vector<ZEllipsoid> range;

  ZCuboid treeBound = tree.boundBox();

  double zStart = treeBound.firstCorner().z();
  double zEnd = treeBound.lastCorner().z();

  ZSwcNodeZRangeSelector selector;
  for (double z = zStart; z <= zEnd; z += m_zStep) {
    selector.setZRange(z - m_zMargin, z + m_zMargin);
    std::vector<Swc_Tree_Node*> nodeSet = selector.selectFrom(tree);
    if (!nodeSet.empty()) {
      ZCuboid bound = SwcTreeNode::boundBox(nodeSet.begin(), nodeSet.end());
      ZEllipsoid ellipsoid;
      ellipsoid.setCenter(bound.center());
      double rxy = sqrt(bound.width() * bound.width() +
                        bound.height() * bound.height()) / 2.0;
      ellipsoid.setSize(rxy, rxy, bound.depth() / 2.0);
      range.push_back(ellipsoid);
    }
  }

  return range;
}

std::vector<ZPoint> ZSwcRangeAnalyzer::computerLayerCentroid(const ZSwcTree &tree)
{
  std::vector<ZPoint> centroidArray;

  ZCuboid treeBound = tree.boundBox();

  double zStart = treeBound.firstCorner().z();
  double zEnd = treeBound.lastCorner().z();

  ZSwcNodeZRangeSelector selector;
  for (double z = zStart; z <= zEnd; z += m_zStep) {
    selector.setZRange(z - m_zMargin, z + m_zMargin);
    std::vector<Swc_Tree_Node*> nodeSet = selector.selectFrom(tree);
    if (!nodeSet.empty()) {
      centroidArray.push_back(
            SwcTreeNode::centroid(nodeSet.begin(), nodeSet.end()));
    }
  }

  return centroidArray;
}


std::vector<Swc_Tree_Node*> ZSwcRangeAnalyzer::getOutsideNode(
    const ZSwcTree &host, const ZSwcTree &targetTree)
{
  std::vector<Swc_Tree_Node*> nodeArray;

  std::vector<ZEllipsoid> range = computeLayerRange(host);
  ZSwcNodeCompositeSelector selector;
  for (std::vector<ZEllipsoid>::iterator iter = range.begin();
       iter != range.end(); ++iter) {
    iter->setCenterX(0);
    iter->setCenterY(0);
    ZSwcNodeEllipsoidRangeSelector *childSelector =
        new ZSwcNodeEllipsoidRangeSelector;
    childSelector->setExpandFactor(3.0);
    //iter->print();
    /*
    iter->setSize(iter->getXRadius() + sqrt(iter->getXRadius()),
                  iter->getYRadius() + sqrt(iter->getYRadius()),
                  iter->getZRadius() + sqrt(iter->getZRadius()) * 5);
                  */

    childSelector->setRange(*iter);
    selector.addSelector(childSelector);
  }

  std::vector<ZEllipsoid> range2 = computeLayerRange(targetTree);
  std::vector<ZPoint> pointArray = computerLayerCentroid(targetTree);

  Swc_Tree_Node bufferNode;
  targetTree.updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST);
  for (Swc_Tree_Node *tn = targetTree.begin(); tn != NULL;
       tn = targetTree.next()) {
    if (SwcTreeNode::isRegular(tn)) {
      bufferNode = *tn;
      for (size_t i = 0; i < range2.size(); ++i) {
        if (range2[i].containsPoint(
              SwcTreeNode::x(tn), SwcTreeNode::y(tn), SwcTreeNode::z(tn))) {
          SwcTreeNode::translate(&bufferNode, -pointArray[i].x(),
                                 -pointArray[i].y(), 0.0);
          break;
        }
      }
      if (!selector.isSelected(&bufferNode)) {
        nodeArray.push_back(tn);
      }
    }
  }

  return nodeArray;
}
