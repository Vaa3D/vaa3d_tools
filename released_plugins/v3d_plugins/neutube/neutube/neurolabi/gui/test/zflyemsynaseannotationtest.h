#ifndef ZFLYEMSYNASEANNOTATIONTEST_H
#define ZFLYEMSYNASEANNOTATIONTEST_H

#include "ztestheader.h"
#include "neutubeconfig.h"
#include "flyem/zsynapseannotationarray.h"
#include "flyem/zintcuboidarray.h"
#include "flyem/zflyemdatabundle.h"
#include "zgraph.h"

#ifdef _USE_GTEST_

TEST(ZFlyEmSyanpzeAnnotation, synapseCount) {
  FlyEm::ZSynapseAnnotationArray synapseArray;

  synapseArray.loadJson(
        GET_TEST_DATA_DIR + "/benchmark/flyem/annotations-synapse.json");

  //synapseArray.print();

  std::vector<int> tbarCount = synapseArray.countTBar();
  EXPECT_EQ(100000001, (int) tbarCount.size());
  EXPECT_EQ(0, tbarCount[100]);
  EXPECT_EQ(1, tbarCount[65535]);
  EXPECT_EQ(1, tbarCount[16711935]);
  EXPECT_EQ(1, tbarCount[100000000]);

  std::vector<int> psdCount = synapseArray.countPsd();
  EXPECT_EQ(100000001, (int) psdCount.size());
  EXPECT_EQ(0, psdCount[100]);
  EXPECT_EQ(1, psdCount[65535]);
  EXPECT_EQ(2, psdCount[16711935]);
  EXPECT_EQ(3, psdCount[100000000]);

  std::vector<int> count = synapseArray.countSynapse();
  EXPECT_EQ(100000001, (int) psdCount.size());
  EXPECT_EQ(0, count[100]);
  EXPECT_EQ(2, count[65535]);
  EXPECT_EQ(3, count[16711935]);
  EXPECT_EQ(4, count[100000000]);
}

TEST(ZFlyEmSyanpzeAnnotation, buildConnection) {
  FlyEm::ZSynapseAnnotationArray synapseArray;

  synapseArray.loadJson(
        GET_TEST_DATA_DIR + "/benchmark/flyem/annotations-synapse.json");

  ZGraph *graph = synapseArray.getConnectionGraph();
  EXPECT_EQ(100000001, graph->getVertexNumber());
  EXPECT_EQ(3, graph->getEdgeNumber());

  ZFlyEmDataBundle bundle;
  bundle.loadJsonFile(GET_TEST_DATA_DIR + "/benchmark/flyem/data_bundle.json");

  FlyEm::ZSynapseAnnotationArray *synapseAnnotation =
      bundle.getSynapseAnnotation();
  graph = synapseAnnotation->getConnectionGraph();
  graph->print();
  EXPECT_EQ(100000001, graph->getVertexNumber());
  EXPECT_EQ(3, graph->getEdgeNumber());
}

#endif

#endif // ZFLYEMSYNASEANNOTATIONTEST_H
