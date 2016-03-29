#include "sigen/extractor/extractor.h"
#include <boost/foreach.hpp>
#include <gtest/gtest.h>
#include <iostream>
using namespace std;
using namespace sigen;

static BinaryCube vectorStringToBinaryCube(const std::vector<std::string> &vs) {
  const int zdepth = 3;
  const int size_x = vs[0].size();
  const int size_y = vs.size();
  for (int i = 0; i < (int)size_y; ++i) {
    assert(size_x == (int)vs[i].size());
  }
  BinaryCube cube(size_x + 2, size_y + 2, zdepth);
  for (int y = 0; y < size_y; ++y) {
    for (int x = 0; x < size_x; ++x) {
      cube[x + 1][y + 1][1] = (vs[y][x] == '#');
    }
  }
  return cube;
}

TEST(Extractor, labeling) {
  std::vector<std::string> vs;
  vs.push_back("##.");
  vs.push_back("...");
  vs.push_back("###");
  BinaryCube cube = vectorStringToBinaryCube(vs);

  Extractor ext(cube);
  std::vector<ClusterPtr> ret = ext.Extract();
  EXPECT_EQ(2, (int)ext.components_.size());
  EXPECT_EQ(3, (int)ext.components_[0].size());
  EXPECT_EQ(2, (int)ext.components_[1].size());
  EXPECT_EQ(5, (int)ret.size());
  BOOST_FOREACH (ClusterPtr c, ret) {
    EXPECT_EQ(1, (int)c->points_.size());
  }
}
TEST(Extractor, labeling2) {
  std::vector<std::string> vs;
  vs.push_back("##.");
  vs.push_back("..#");
  vs.push_back("##.");
  BinaryCube cube = vectorStringToBinaryCube(vs);
  Extractor ext(cube);
  std::vector<ClusterPtr> ret = ext.Extract();
  EXPECT_EQ(1, (int)ext.components_.size());
  EXPECT_EQ(5, (int)ext.components_[0].size());
  EXPECT_EQ(5, (int)ret.size());
}
TEST(Extractor, labeling_with_loops) {
  std::vector<std::string> vs;
  vs.push_back(".#.");
  vs.push_back("#.#");
  vs.push_back(".#.");
  BinaryCube cube = vectorStringToBinaryCube(vs);
  Extractor ext(cube);
  std::vector<ClusterPtr> ret = ext.Extract();
  EXPECT_EQ(1, (int)ext.components_.size());
  EXPECT_EQ(4, (int)ext.components_[0].size());
}
TEST(Extractor, same_distance) {
  std::vector<std::string> vs;
  vs.push_back("##");
  vs.push_back("##");
  BinaryCube cube = vectorStringToBinaryCube(vs);
  Extractor ext(cube);
  std::vector<ClusterPtr> ret = ext.Extract();
  EXPECT_EQ(1, (int)ext.components_.size());
  EXPECT_EQ(4, (int)ext.components_[0].size());
  EXPECT_EQ(2, (int)ret.size());
  EXPECT_EQ(3, (int)ret[0]->points_.size());
  EXPECT_EQ(1, (int)ret[1]->points_.size());
}
