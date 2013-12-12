#ifndef ZSWCANALYZERTEST_H
#define ZSWCANALYZERTEST_H

#include "ztestheader.h"
#include "neutubeconfig.h"
#include "zswcrangeanalyzer.h"
#include "zswcnodezrangeselector.h"
#include "zswcnodeellipsoidrangeselector.h"
#include "zswcnodecompositeselector.h"
#include "swctreenode.h"
#include "zswcglobalfeatureanalyzer.h"
#include "zswcsubtreefeatureanalyzer.h"
#include "swc/zswcsubtreeanalyzer.h"

#ifdef _USE_GTEST_

TEST(ZSwcRangeAnalyzer, computeLayerRange)
{
  ZSwcRangeAnalyzer analyzer;
  ZSwcTree tree;
  tree.load(GET_TEST_DATA_DIR + "/benchmark/swc/range1.swc");
  analyzer.setZStep(10.0);
  analyzer.setZMargin(5.0);
  std::vector<ZEllipsoid> range = analyzer.computeLayerRange(tree);

  EXPECT_DOUBLE_EQ(10.0, range[0].getCenter().x());
  EXPECT_DOUBLE_EQ(10.0, range[0].getCenter().y());
  EXPECT_DOUBLE_EQ(10.0, range[0].getCenter().z());
  //EXPECT_DOUBLE_EQ(3.0, range[0].getXRadius());

  EXPECT_DOUBLE_EQ(10.0, range[1].getCenter().x());
  EXPECT_DOUBLE_EQ(10.0, range[1].getCenter().y());
  EXPECT_DOUBLE_EQ(20.0, range[1].getCenter().z());
  //EXPECT_DOUBLE_EQ(3.0, range[1].getYRadius());

  EXPECT_DOUBLE_EQ(10.0, range[2].getCenter().x());
  EXPECT_DOUBLE_EQ(10.0, range[2].getCenter().y());
  EXPECT_DOUBLE_EQ(30.0, range[2].getCenter().z());
  //EXPECT_DOUBLE_EQ(3.0, range[2].getZRadius());

  //tree.load(GET_TEST_DATA_DIR + "/benchmark/swc/range2.swc");
}

TEST(ZSwcNodeZRangeSelector, selectFrom)
{
  ZSwcTree tree;
  tree.load(GET_TEST_DATA_DIR + "/benchmark/swc/range1.swc");

  ZSwcNodeZRangeSelector selector;
  selector.setZRange(0.0, 15.0);
  std::vector<Swc_Tree_Node*> nodeArray = selector.selectFrom(tree);
  EXPECT_EQ(1, (int) nodeArray.size());

  nodeArray = selector.antiSelectFrom(tree);
  EXPECT_EQ(2, (int) nodeArray.size());

  selector.setZRange(15.0, 25.0);
  nodeArray = selector.selectFrom(tree);
  EXPECT_EQ(1, (int) nodeArray.size());

  nodeArray = selector.antiSelectFrom(tree);
  EXPECT_EQ(2, (int) nodeArray.size());

  selector.excludeVirtualNode(false);
  nodeArray = selector.antiSelectFrom(tree);
  EXPECT_EQ(3, (int) nodeArray.size());

  selector.excludeVirtualNode(true);
  selector.setZRange(15.0, 35.0);
  nodeArray = selector.selectFrom(tree);
  EXPECT_EQ(2, (int) nodeArray.size());

  nodeArray = selector.antiSelectFrom(tree);
  EXPECT_EQ(1, (int) nodeArray.size());
}

TEST(ZSwcNodeSelector, range)
{
  ZSwcTree tree1;
  tree1.load(GET_TEST_DATA_DIR + "/benchmark/swc/range1.swc");
  ZSwcTree tree2;
  tree2.load(GET_TEST_DATA_DIR + "/benchmark/swc/range2.swc");

  ZSwcRangeAnalyzer rangeAnalyzer;

  std::vector<ZEllipsoid> range = rangeAnalyzer.computeLayerRange(tree1);
  ZSwcNodeCompositeSelector selector;
  for (std::vector<ZEllipsoid>::const_iterator iter = range.begin();
       iter != range.end(); ++iter) {
    ZSwcNodeEllipsoidRangeSelector *childSelector =
        new ZSwcNodeEllipsoidRangeSelector;
    //iter->print();
    childSelector->setRange(*iter);
    selector.addSelector(childSelector);
  }

  std::vector<Swc_Tree_Node*> nodeArray = selector.selectFrom(tree2);
  EXPECT_EQ(3, (int) nodeArray.size());

  nodeArray = selector.antiSelectFrom(tree2);
  EXPECT_EQ(4, (int) nodeArray.size());

  tree2.load(GET_TEST_DATA_DIR + "/benchmark/swc/range3.swc");
  nodeArray = rangeAnalyzer.getOutsideNode(tree1, tree2);
  EXPECT_EQ(1, (int) nodeArray.size());
  //SwcTreeNode::setType(nodeArray.begin(), nodeArray.end(), 0);
  /*

  std::vector<ZEllipsoid> range2 = rangeAnalyzer.computeLayerRange(tree2);
  Swc_Tree_Node bufferNode;
  tree2.updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST);
  for (Swc_Tree_Node *tn = tree2.begin(); tn != NULL; tn = tree2.next()) {
    bufferNode = *tn;
    for (std::vector<ZEllipsoid>::const_iterator iter = range2.begin();
         iter != range2.end(); ++iter) {
      if (iter->containsPoint(
            SwcTreeNode::x(tn), SwcTreeNode::y(tn), SwcTreeNode::z(tn))) {
        SwcTreeNode::translate(&bufferNode, -iter->getCenter().x(),
                               -iter->getCenter().y(), 0.0);
        break;
      }
    }
    if (!selector.isSelected(&bufferNode)) {
      SwcTreeNode::setType(tn, 0);
    }
  }
*/
  //tree2.print();
  //tree2.save(GET_TEST_DATA_DIR + "/test.swc");
}

TEST(ZSwcGlobalFeatureAnalyzer, computeFeature)
{
  ZSwcTree tree;
  tree.load(GET_TEST_DATA_DIR + "/benchmark/swc/single_node.swc");
  double ratio = ZSwcGlobalFeatureAnalyzer::computeLateralVerticalRatio(tree);
  EXPECT_LT(1.0, ratio);
  EXPECT_GT(2.0, ratio);
}

TEST(ZSwcSubtreeAnalyzer, labelSubtree)
{
  ZSwcSubtreeAnalyzer analyzer;
  analyzer.setMinLength(10.0);
  analyzer.setMaxLength(15.0);

  ZSwcTree tree;
  tree.load(GET_TEST_DATA_DIR + "/benchmark/swc/length_test.swc");
  analyzer.labelSubtree(&tree);

  tree.setTypeByLabel();
  tree.save(GET_TEST_DATA_DIR + "/test.swc");
}

#endif

#endif // ZSWCANALYZERTEST_H
