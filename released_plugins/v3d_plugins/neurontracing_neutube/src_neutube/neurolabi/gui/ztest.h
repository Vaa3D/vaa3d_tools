#ifndef ZTEST_H
#define ZTEST_H

#include <set>
#include <iostream>
#include "zswctree.h"

class MainWindow;

struct ZTestSwcTreeIteratorConfig {
  int option;
  Swc_Tree_Node *start;
  std::set<Swc_Tree_Node*> *blocker;

  ZTestSwcTreeIteratorConfig() : option(SWC_TREE_ITERATOR_NO_UPDATE),
    start(NULL), blocker(NULL) {}
};

class ZTest
{
public:
  ZTest();

public:
  template <typename T>
  static bool testEqual(const T &golden, const T &v);
  static void test(MainWindow *host);
  static bool testTreeIterator();
  static int runUnitTest(int argc, char *argv[]);

public:
  static bool testTreeIterator(ZSwcTree &tree,
                               const ZTestSwcTreeIteratorConfig &config,
                               int *truthArray,
                               int truthCount, bool testReverse = false);

  static std::ostream &m_failureStream;
};

template<typename T>
bool ZTest::testEqual(const T &golden, const T &v)
{
  if (golden != v) {
    m_failureStream << "Unexpected value: " << v << "; Expected: "
                    << golden << std::endl;

    return false;
  }

  return true;
}

#endif // ZTEST_H
