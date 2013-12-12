#ifndef ZGRAPHTEST_H
#define ZGRAPHTEST_H

#include "ztestheader.h"
#include "neutubeconfig.h"
#include "zgraph.h"

#ifdef _USE_GTEST_


TEST(ZGraph, Basic) {
  ZGraph graph;
  EXPECT_FALSE(graph.isDirected());
  EXPECT_FALSE(graph.isWeighted());

  EXPECT_EQ(0, (int) graph.size());
  EXPECT_EQ(0, graph.order());

  graph.setDirected(true);
  EXPECT_TRUE(graph.isDirected());
  EXPECT_FALSE(graph.isWeighted());
}

TEST(ZGraph, getProperty) {
  ZGraph graph;
  graph.addEdge(1, 2);
  graph.addEdge(3, 4);
  graph.addEdge(1, 2);

  EXPECT_FALSE(graph.isDirected());
  EXPECT_FALSE(graph.isWeighted());

  EXPECT_EQ(2, (int) graph.size());
  EXPECT_EQ(5, graph.order());

  EXPECT_TRUE(graph.hasVertex(0));
  EXPECT_FALSE(graph.hasVertex(5));
}

TEST(ZGraph, AddEdge) {
  ZGraph graph;
  graph.addEdge(1, 2);
  graph.addEdge(3, 4);
  graph.addEdge(1, 2);
  EXPECT_EQ(0, graph.getEdgeIndex(1, 2));
  EXPECT_EQ(1, graph.getEdgeIndex(3, 4));

  graph.clear();
  EXPECT_EQ(0, (int) graph.size());
  EXPECT_EQ(0, graph.order());

  graph.setDirected(false);
  graph.addEdge(1, 2);
  graph.addEdge(1, 2);
  graph.addEdge(3, 4);
  EXPECT_EQ(0, graph.getEdgeIndex(1, 2));
  EXPECT_EQ(0, graph.getEdgeIndex(2, 1));
  EXPECT_EQ(1, graph.getEdgeIndex(3, 4));
  EXPECT_EQ(1, graph.getEdgeIndex(4, 3));

  graph.setEdge(0, 5, 6);
  EXPECT_EQ(-1, graph.getEdgeIndex(1, 2));
  EXPECT_EQ(0, graph.getEdgeIndex(5, 6));

  graph.setVertexNumber(10);
  EXPECT_EQ(10, graph.getVertexNumber());

  graph.setDirected(true);
  EXPECT_EQ(0, graph.getEdgeIndex(5, 6));
  EXPECT_EQ(-1, graph.getEdgeIndex(6, 5));
  EXPECT_EQ(1, graph.getEdgeIndex(3, 4));
  EXPECT_EQ(-1, graph.getEdgeIndex(4, 3));

  graph.setDirected(false);
  EXPECT_EQ(0, graph.getEdgeIndex(5, 6));
  EXPECT_EQ(0, graph.getEdgeIndex(6, 5));
  EXPECT_EQ(1, graph.getEdgeIndex(3, 4));
  EXPECT_EQ(1, graph.getEdgeIndex(4, 3));
}

TEST(ZGraph, getDegree) {
  ZGraph graph;
  graph.setDirected(false);
  graph.addEdge(1, 2);
  graph.addEdge(3, 4);
  graph.addEdge(3, 5);
  graph.addEdge(6, 3);
  EXPECT_EQ(1, graph.getDegree(1));
  EXPECT_EQ(1, graph.getDegree(2));
  EXPECT_EQ(3, graph.getDegree(3));
  EXPECT_EQ(0, graph.getDegree(7));

  ZGraph *graph2 = graph.clone();
  graph2->setDirected(false);
  graph2->addEdge(1, 2);
  graph2->addEdge(3, 4);
  graph2->addEdge(3, 5);
  graph2->addEdge(6, 3);
  EXPECT_EQ(1, graph2->getDegree(1));
  EXPECT_EQ(1, graph2->getDegree(2));
  EXPECT_EQ(3, graph2->getDegree(3));
  EXPECT_EQ(0, graph2->getDegree(7));
}

TEST(ZGraph, computeShortestPath) {
  ZGraph graph(ZGraph::UNDIRECTED_WITH_WEIGHT);
  graph.addEdge(0, 2, 2.0);
  graph.addEdge(1, 2, 1.0);
  graph.addEdge(3, 2, 5.0);
  graph.addEdge(3, 4, 1.0);
  graph.addEdge(4, 5, 2.0);
  graph.addEdge(2, 4, 1.0);
  graph.addEdge(0, 4, 1.0);
  graph.addEdge(0, 6, 1.0);
  graph.addEdge(1, 7, 1.0);
  graph.addEdge(0, 8, 3.0);
  graph.addEdge(4, 9, 1.0);


  std::vector<int> path = graph.computeShortestPath(0, 9);
  EXPECT_EQ(3, (int) path.size());
  EXPECT_EQ(0, path[0]);
  EXPECT_EQ(4, path[1]);
  EXPECT_EQ(9, path[2]);

  ZGraph graph2(ZGraph::DIRECTED_WITH_WEIGHT);

  graph2.addEdge(0, 2, 2.0);
  graph2.addEdge(1, 2, 1.0);
  graph2.addEdge(3, 2, 5.0);
  graph2.addEdge(3, 4, 1.0);
  graph2.addEdge(4, 5, 2.0);
  graph2.addEdge(2, 4, 1.0);
  graph2.addEdge(0, 4, 1.0);
  graph2.addEdge(0, 6, 1.0);
  graph2.addEdge(1, 7, 1.0);
  graph2.addEdge(0, 8, 3.0);
  graph2.addEdge(4, 9, 1.0);

  //graph2.exportDotFile(GET_TEST_DATA_DIR + "/test.dot");
  path = graph2.computeShortestPath(0, 100);
  EXPECT_TRUE(path.empty());

  ZGraph graph3;
  graph3.addEdge(0, 2, 2.0);
  graph3.addEdge(1, 2, 1.0);
  graph3.addEdge(3, 2, 5.0);
  graph3.addEdge(4, 5, 2.0);
  graph3.addEdge(0, 6, 1.0);
  graph3.addEdge(1, 7, 1.0);
  graph3.addEdge(0, 8, 3.0);
  graph3.addEdge(4, 9, 1.0);
  path = graph3.computeShortestPath(0, 9);
  EXPECT_EQ(0, (int) path.size());
}

TEST(ZGraph, match)
{
  ZGraph graph(ZGraph::UNDIRECTED_WITH_WEIGHT);
  std::map<int, int> match = graph.runMinWeightSumMatch();

  EXPECT_EQ(0, (int) match.size());

  graph.addEdge(0, 1, 1);
  graph.addEdge(1, 2, 4);
  graph.addEdge(0, 2, 2);

  match = graph.runMinWeightSumMatch();
  EXPECT_EQ(1, (int) match.size());
  EXPECT_EQ(1, match[0]);

  graph.addEdge(0, 3, 9);
  graph.addEdge(1, 3, 16);
  graph.addEdge(2, 3, 25);

  match = graph.runMinWeightSumMatch();
  EXPECT_EQ(2, (int) match.size());
  EXPECT_EQ(2, (int) match[1]);
  EXPECT_EQ(3, (int) match[0]);

  graph.addEdge(3, 5, 1);
  graph.addEdge(2, 5, 2);
  graph.addEdge(3, 7, 5);
  graph.addEdge(5, 7, 3);
  graph.addEdge(7, 8, 8);
  graph.addEdge(5, 8, 21);
  graph.addEdge(7, 9, 10);
  graph.addEdge(5, 6, 9);
  graph.addEdge(6, 8, 4);
  graph.addEdge(8, 9, 22);
  graph.addEdge(6, 9, 20);

  //graph.exportDotFile(GET_TEST_DATA_DIR + "/test.dot");


  match = graph.runMinWeightSumMatch();
  EXPECT_EQ(4, (int) match.size());
  EXPECT_EQ(1, match[0]);
  EXPECT_EQ(5, match[3]);
  EXPECT_EQ(8, match[6]);
  EXPECT_EQ(9, match[7]);
  for (std::map<int, int>::const_iterator iter = match.begin();
       iter != match.end(); ++iter) {
    std::cout << iter->first << " " << iter->second << std::endl;
  }
}

#endif


#endif // ZGRAPHTEST_H
