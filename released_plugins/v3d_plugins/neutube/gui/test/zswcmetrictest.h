#ifndef ZSWCMETRICTEST_H
#define ZSWCMETRICTEST_H

#include <iostream>

#include "ztestheader.h"
#include "swc/zswcmetric.h"
#include "swc/zswcterminalanglemetric.h"
#include "swc/zswcterminalsurfacemetric.h"
#include "swc/zswcdeepanglemetric.h"
#include "zswctree.h"
#include "neutubeconfig.h"

#ifdef _USE_GTEST_

TEST(ZSwcMetric, measure)
{
  ZSwcTerminalSurfaceMetric metric;

  ZSwcTree tree1;
  ZSwcTree tree2;

  tree1.load(GET_TEST_DATA_DIR + "/benchmark/bundle1/swc/1.swc");
  tree2.load(GET_TEST_DATA_DIR + "/benchmark/bundle1/swc/2.swc");

  double dist = metric.measureDistance(&tree1, &tree2);

  EXPECT_DOUBLE_EQ(5000, dist);

  tree2.load(GET_TEST_DATA_DIR + "/benchmark/bundle1/swc/3.swc");
  dist = metric.measureDistance(&tree1, &tree2);
  EXPECT_DOUBLE_EQ(19015.232570685483, dist);

  tree2.load(GET_TEST_DATA_DIR + "/benchmark/bundle1/swc/4.swc");
  dist = metric.measureDistance(&tree1, &tree2);
  EXPECT_GT(0, dist);

  ZSwcTerminalAngleMetric metric2;
  metric2.setDistanceWeight(false);
  tree2.load(GET_TEST_DATA_DIR + "/benchmark/bundle1/swc/2.swc");
  dist = metric2.measureDistance(&tree1, &tree2);

  EXPECT_DOUBLE_EQ(0.0, dist);

  tree2.load(GET_TEST_DATA_DIR + "/benchmark/bundle1/swc/3.swc");
  dist = metric2.measureDistance(&tree1, &tree2);

  EXPECT_DOUBLE_EQ(2.8551067559664194, dist);

  tree2.load(GET_TEST_DATA_DIR + "/benchmark/bundle1/swc/4.swc");
  dist = metric2.measureDistance(&tree1, &tree2);

  EXPECT_DOUBLE_EQ(1.5707963267948966, dist);

  ZSwcDeepAngleMetric metric3;
  metric3.setMinDist(10000.0);
  tree2.load(GET_TEST_DATA_DIR + "/benchmark/bundle1/swc/2.swc");
  dist = metric3.measureDistance(&tree1, &tree2);

  EXPECT_DOUBLE_EQ(0.0, dist);

  metric3.setLevel(3);

  dist = metric3.measureDistance(&tree1, &tree2);


  EXPECT_DOUBLE_EQ(0.0, dist);

  std::cout << dist << std::endl;
}

#endif


#endif // ZSWCMETRICTEST_H
