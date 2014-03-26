#ifndef ZTREETEST_H
#define ZTREETEST_H

#include "ztestheader.h"
#include "neutubeconfig.h"
#include "ztree.h"
#include "ztreeiterator.h"

#ifdef _USE_GTEST_

TEST(tree, iterator)
{
  ZTree<int> tree;
  EXPECT_TRUE(tree.isEmpty());

  ZTreeNode<int> *root = new ZTreeNode<int>;
  root->setData(1);
  tree.setRoot(root);


  ZTreeIterator<int> iterator(tree);
  EXPECT_EQ(1, (int) iterator.size());

  root->addChild(2);
  root->addChild(3);
  root->addChild(4);
  iterator.update(tree);

  /*
  while (iterator.hasNext()) {
    std::cout << iterator.next() << std::endl;
  }
  */

  EXPECT_EQ(4, (int) iterator.size());

  iterator.update(tree, ZTreeIterator<int>::BREADTH_FIRST);
  EXPECT_EQ(4, (int) iterator.size());
}

#endif

#endif // ZTREETEST_H
