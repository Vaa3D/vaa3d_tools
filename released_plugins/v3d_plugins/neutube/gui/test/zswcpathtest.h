#ifndef ZSWCPATHTEST_H
#define ZSWCPATHTEST_H

#include "ztestheader.h"
#include "zswcpath.h"
#include "zswctree.h"
#include "neutubeconfig.h"
#include "swctreenode.h"

#ifdef _USE_GTEST_

static void createSwcPath(ZSwcPath *path)
{
  ZSwcTree *tree = new ZSwcTree;
  tree->load((GET_TEST_DATA_DIR +
             "/benchmark/swc/fork.swc").c_str());
  path->append(tree->firstRegularRoot());
  path->append(SwcTreeNode::firstChild(tree->firstRegularRoot()));
  path->append(SwcTreeNode::firstChild(
                 SwcTreeNode::firstChild(tree->firstRegularRoot())));
}

TEST(TestSwcPath, TestGetProperty) {
  ZSwcPath path;
  createSwcPath(&path);
  ASSERT_EQ(path.isContinuous(), true);
  ASSERT_EQ((int) path.size(), 3);
  for (ZSwcPath::const_iterator iter = path.begin();
       iter != path.end(); ++iter) {
    ASSERT_EQ(SwcTreeNode::label(*iter), 0);
  }

  path.label(1);
  for (ZSwcPath::const_iterator iter = path.begin();
       iter != path.end(); ++iter) {
    ASSERT_EQ(SwcTreeNode::label(*iter), 1);
  }

  path.addLabel(1);
  for (ZSwcPath::const_iterator iter = path.begin();
       iter != path.end(); ++iter) {
    ASSERT_EQ(SwcTreeNode::label(*iter), 2);
  }

  path.setType(5);
  for (ZSwcPath::const_iterator iter = path.begin();
       iter != path.end(); ++iter) {
    ASSERT_EQ(SwcTreeNode::type(*iter), 5);
  }
}

#endif

#endif // ZSWCPATHTEST_H
