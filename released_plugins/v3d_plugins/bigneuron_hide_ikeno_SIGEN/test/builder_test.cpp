#include "sigen/builder/builder.h"
#include "sigen/extractor/extractor.h"
#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>
#include <cassert>
#include <gtest/gtest.h>
#include <string>
#include <vector>
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

// Fixture
class BuilderTestAlpha : public ::testing::Test {
protected:
  boost::shared_ptr<Builder> bld;
  virtual void SetUp() {
    std::vector<std::string> vs;
    vs.push_back("##.");
    vs.push_back("...");
    vs.push_back("###");
    BinaryCube cube = vectorStringToBinaryCube(vs);
    Extractor ext(cube);
    std::vector<ClusterPtr> data = ext.Extract();
    bld = boost::make_shared<Builder>(data, 1.0, 1.0);
  }
};

TEST_F(BuilderTestAlpha, ConnectNeighbors) {
  bld->ConnectNeighbors();
  ASSERT_EQ(5, (int)bld->data_.size());
  EXPECT_EQ(1, (int)bld->data_[0]->adjacent_.size());
  EXPECT_EQ(2, (int)bld->data_[1]->adjacent_.size());
  EXPECT_EQ(1, (int)bld->data_[2]->adjacent_.size());
  EXPECT_EQ(1, (int)bld->data_[3]->adjacent_.size());
  EXPECT_EQ(1, (int)bld->data_[4]->adjacent_.size());
}
TEST_F(BuilderTestAlpha, ComputeGravityPoints) {
  bld->ConnectNeighbors();
  bld->ComputeGravityPoints();
  EXPECT_DOUBLE_EQ(1.0, bld->data_[0]->gx_);
  EXPECT_DOUBLE_EQ(3.0, bld->data_[0]->gy_);
  EXPECT_DOUBLE_EQ(1.0, bld->data_[0]->gz_);
  EXPECT_DOUBLE_EQ(2.0, bld->data_[4]->gx_);
  EXPECT_DOUBLE_EQ(1.0, bld->data_[4]->gy_);
  EXPECT_DOUBLE_EQ(1.0, bld->data_[4]->gz_);
}
TEST_F(BuilderTestAlpha, ConvertToNeuronNodes) {
  bld->ConnectNeighbors();
  bld->ComputeGravityPoints();
  bld->ComputeRadius();
  bld->CutLoops();
  std::vector<NeuronNodePtr> nn = bld->ConvertToNeuronNodes();
  ASSERT_EQ(5, (int)nn.size());
  EXPECT_DOUBLE_EQ(1.0, nn[0]->gx_);
  EXPECT_DOUBLE_EQ(3.0, nn[0]->gy_);
  EXPECT_DOUBLE_EQ(1.0, nn[0]->gz_);
  EXPECT_DOUBLE_EQ(2.0, nn[4]->gx_);
  EXPECT_DOUBLE_EQ(1.0, nn[4]->gy_);
  EXPECT_DOUBLE_EQ(1.0, nn[4]->gz_);
  ASSERT_EQ(1, (int)nn[0]->adjacent_.size());
  ASSERT_EQ(2, (int)nn[1]->adjacent_.size());
  ASSERT_EQ(1, (int)nn[2]->adjacent_.size());
  ASSERT_EQ(1, (int)nn[3]->adjacent_.size());
  ASSERT_EQ(1, (int)nn[4]->adjacent_.size());
  EXPECT_TRUE(nn[0]->HasConnection(nn[1]));
  EXPECT_TRUE(nn[1]->HasConnection(nn[0]));
  EXPECT_TRUE(nn[1]->HasConnection(nn[2]));
  EXPECT_TRUE(nn[2]->HasConnection(nn[1]));
  EXPECT_TRUE(nn[3]->HasConnection(nn[4]));
  EXPECT_TRUE(nn[4]->HasConnection(nn[3]));
}
TEST_F(BuilderTestAlpha, ConvertToNeuron) {
  bld->ConnectNeighbors();
  bld->ComputeGravityPoints();
  bld->ComputeRadius();
  bld->CutLoops();
  std::vector<Neuron> ns = bld->ConvertToNeuron();
  ASSERT_EQ(2, (int)ns.size());
  EXPECT_EQ(3, (int)ns[0].storage_.size());
  EXPECT_EQ(2, (int)ns[1].storage_.size());
}
TEST_F(BuilderTestAlpha, ComputeIds) {
  bld->ConnectNeighbors();
  bld->ComputeGravityPoints();
  bld->ComputeRadius();
  bld->CutLoops();
  std::vector<Neuron> ns = bld->ConvertToNeuron();
  bld->ComputeIds(ns);
  EXPECT_EQ(1, ns[0].storage_[0]->id_);
  EXPECT_EQ(2, ns[0].storage_[1]->id_);
  EXPECT_EQ(3, ns[0].storage_[2]->id_);
  EXPECT_EQ(4, ns[1].storage_[0]->id_);
  EXPECT_EQ(5, ns[1].storage_[1]->id_);
}

// Fixture
class BuilderTestBeta : public ::testing::Test {
protected:
  boost::shared_ptr<Builder> bld;
  virtual void SetUp() {
    std::vector<std::string> vs;
    vs.push_back(".#.");
    vs.push_back("#.#");
    vs.push_back(".#.");
    BinaryCube cube = vectorStringToBinaryCube(vs);
    Extractor ext(cube);
    std::vector<ClusterPtr> data = ext.Extract();
    bld = boost::make_shared<Builder>(data, 1.0, 1.0);
  }
};

TEST_F(BuilderTestBeta, DISABLED_convertToNeuronNodesWithLoops) {
  bld->ConnectNeighbors();
  bld->ComputeGravityPoints();
  bld->ComputeRadius();
  bld->CutLoops();
  std::vector<NeuronNodePtr> nn = bld->ConvertToNeuronNodes();
  ASSERT_EQ(4, (int)nn.size());
  EXPECT_DOUBLE_EQ(1.0, nn[0]->gx_);
  EXPECT_DOUBLE_EQ(2.0, nn[0]->gy_);

  EXPECT_DOUBLE_EQ(2.0, nn[1]->gx_);
  EXPECT_DOUBLE_EQ(1.0, nn[1]->gy_);

  EXPECT_DOUBLE_EQ(2.0, nn[2]->gx_);
  EXPECT_DOUBLE_EQ(3.0, nn[2]->gy_);

  EXPECT_DOUBLE_EQ(3.0, nn[3]->gx_);
  EXPECT_DOUBLE_EQ(2.0, nn[3]->gy_);
  ASSERT_EQ(1, (int)nn[0]->adjacent_.size());
  ASSERT_EQ(1, (int)nn[1]->adjacent_.size());
  ASSERT_EQ(2, (int)nn[2]->adjacent_.size());
  ASSERT_EQ(2, (int)nn[3]->adjacent_.size());
  EXPECT_TRUE(nn[0]->HasConnection(nn[2]));
  EXPECT_TRUE(nn[1]->HasConnection(nn[3]));
  EXPECT_TRUE(nn[2]->HasConnection(nn[0]));
  EXPECT_TRUE(nn[2]->HasConnection(nn[3]));
  EXPECT_TRUE(nn[3]->HasConnection(nn[2]));
  EXPECT_TRUE(nn[3]->HasConnection(nn[1]));
}
TEST_F(BuilderTestBeta, ConvertToNeuronWithLoops) {
  bld->ConnectNeighbors();
  bld->ComputeGravityPoints();
  bld->ComputeRadius();
  bld->CutLoops();
  std::vector<Neuron> ns = bld->ConvertToNeuron();
  EXPECT_EQ(1, (int)ns.size());
  EXPECT_EQ(4, (int)ns[0].storage_.size());
}
TEST_F(BuilderTestBeta, ComputeIdsWithLoops) {
  bld->ConnectNeighbors();
  bld->ComputeGravityPoints();
  bld->ComputeRadius();
  bld->CutLoops();
  std::vector<Neuron> ns = bld->ConvertToNeuron();
  bld->ComputeIds(ns);
}
TEST(Builder, DISABLED_ComputeNodeTypes) {
  ASSERT_TRUE(false);
}
