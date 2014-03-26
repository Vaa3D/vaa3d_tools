#ifndef ZSTACKGRAPHTEST_H
#define ZSTACKGRAPHTEST_H

#include "ztestheader.h"
#include "neutubeconfig.h"
#include "zstackgraph.h"
#include "c_stack.h"
#include "zgraph.h"
#include "zdebug.h"

#ifdef _USE_GTEST_

TEST(ZStackGraph, Basic) {
  ZStackGraph stackGraph;
  stackGraph.setSignalMask(NULL);

  EXPECT_EQ(0x0, stackGraph.buildGraph(NULL));
}

TEST(ZStackGraph, buildGraph) {
  Stack *stack = C_Stack::make(GREY, 3, 3, 3);
  One_Stack(stack);
  ZStackGraph stackGraph;
  ZGraph *graph = stackGraph.buildGraph(stack);

  EXPECT_EQ(27, graph->getVertexNumber());
  EXPECT_EQ(158, graph->getEdgeNumber());

  delete graph;

  Stack *mask = C_Stack::make(GREY, 3, 3, 3);
  Zero_Stack(mask);
  stackGraph.setSignalMask(mask);
  graph = stackGraph.buildGraph(stack);

  EXPECT_EQ(27, graph->getVertexNumber());
  EXPECT_EQ(0, graph->getEdgeNumber());

  delete graph;

  mask->array[0] = 1;
  mask->array[1] = 1;
  stackGraph.setSignalMask(mask);

  graph = stackGraph.buildGraph(stack);
  EXPECT_EQ(2, graph->getVertexNumber());
  EXPECT_EQ(1, graph->getEdgeNumber());
  EXPECT_EQ(0, graph->getEdgeIndex(0, 1));

  delete graph;

  stackGraph.setSignalMaskAcrossZ(mask, C_Stack::depth(stack));
  graph = stackGraph.buildGraph(stack);
  EXPECT_EQ(6, graph->getVertexNumber());
  EXPECT_EQ(11, graph->getEdgeNumber());
  EXPECT_EQ(0, graph->getEdgeIndex(0, 1));
  delete graph;

  Stack *groupMask = C_Stack::make(GREY, 3, 3, 3);
  Zero_Stack(groupMask);
  groupMask->array[0] = 1;
  groupMask->array[2] = 1;
  stackGraph.setGroupMask(groupMask);

  graph = stackGraph.buildGraph(stack);

  EXPECT_EQ(7, graph->getVertexNumber());
  EXPECT_EQ(12, graph->getEdgeNumber());
  EXPECT_LE(0, graph->getEdgeIndex(0, 6));

  delete graph;

  stackGraph.setGroupMask(NULL);
  graph = stackGraph.buildGraph(stack);
  std::vector<int> path = graph->computeShortestPath(1, 5);

  EXPECT_EQ(1, path[0]);
  EXPECT_EQ(3, path[1]);
  EXPECT_EQ(5, path[2]);

  delete graph;

  Zero_Stack(groupMask);
  groupMask->array[1] = 1;
  groupMask->array[9] = 1;
  stackGraph.setGroupMask(groupMask);
  graph = stackGraph.buildGraph(stack);
  path = graph->computeShortestPath(1, 5);

  //graph->exportDotFile(GET_TEST_DATA_DIR + "/test.dot");

  EXPECT_EQ(1, path[0]);
  EXPECT_EQ(6, path[1]);
  EXPECT_EQ(2, path[2]);
  EXPECT_EQ(5, path[3]);

  delete graph;

  groupMask->array[0] = 2;
  groupMask->array[10] = 2;
  stackGraph.setGroupMask(groupMask);
  graph = stackGraph.buildGraph(stack);

  EXPECT_EQ(8, graph->getVertexNumber());
  EXPECT_EQ(15, graph->getEdgeNumber());
  EXPECT_LE(0, graph->getEdgeIndex(0, 6));

  path = graph->computeShortestPath(1, 5);

  EXPECT_EQ(1, path[0]);
  EXPECT_EQ(7, path[1]);
  EXPECT_EQ(2, path[2]);
  EXPECT_EQ(5, path[3]);

  //graph->exportDotFile(GET_TEST_DATA_DIR + "/test.dot");

  delete graph;

  One_Stack(mask);
  Zero_Stack(groupMask);
  groupMask->array[0] = 1;
  groupMask->array[8] = 2;
  stackGraph.setSignalMask(mask);
  stackGraph.setGroupMaskAcrossZ(groupMask, C_Stack::depth(stack));
  graph = stackGraph.buildGraph(stack);
  //graph->exportDotFile(GET_TEST_DATA_DIR + "/test.dot");

  path = graph->computeShortestPath(0, 26);
  EXPECT_EQ(0, path[0]);
  EXPECT_EQ(27, path[1]);
  EXPECT_EQ(18, path[2]);
  EXPECT_EQ(22, path[3]);
  EXPECT_EQ(26, path[4]);

  delete graph;

  path = stackGraph.computeShortestPath(stack, 0, 26);
  EXPECT_EQ(0, path[0]);
  EXPECT_EQ(18, path[1]);
  EXPECT_EQ(22, path[2]);
  EXPECT_EQ(26, path[3]);

  Zero_Stack(mask);
  mask->array[4] = 1;
  mask->array[5] = 1;
  mask->array[7] = 1;
  mask->array[8] = 1;
  stackGraph.setSignalMaskAcrossZ(mask, C_Stack::depth(stack));

  Zero_Stack(groupMask);
  stackGraph.setGroupMask(groupMask);

  graph = stackGraph.buildGraph(stack);
  //graph->exportDotFile(GET_TEST_DATA_DIR + "/test.dot");

  path = stackGraph.computeShortestPath(stack, 4, 26);
  EXPECT_EQ(4, path[0]);
  EXPECT_EQ(13, path[1]);
  EXPECT_EQ(26, path[2]);
  //ZDebugPrintIntArray(path, 0, path.size() - 1);

  C_Stack::kill(mask);
  C_Stack::kill(groupMask);
  C_Stack::kill(stack);
}

#endif

#endif // ZSTACKGRAPHTEST_H
