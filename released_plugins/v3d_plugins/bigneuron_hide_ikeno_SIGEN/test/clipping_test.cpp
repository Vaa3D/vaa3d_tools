#include "sigen/common/neuron.h"
#include "sigen/toolbox/toolbox.h"
#include <boost/make_shared.hpp>
#include <gtest/gtest.h>
#include <set>
#include <vector>
using namespace sigen;
void add(Neuron &n, int id) {
  NeuronNodePtr node = boost::make_shared<NeuronNode>();
  node->id_ = id;
  n.storage_.push_back(node);
}

void connect(Neuron &n, int l, int r) {
  n.storage_[l]->AddConnection(n.storage_[r].get());
  n.storage_[r]->AddConnection(n.storage_[l].get());
}

TEST(Clipping, Clipping) {
  Neuron n;
  for (int i = 0; i < 10; ++i) {
    add(n, i);
  }
  connect(n, 0, 1);
  connect(n, 1, 2);
  connect(n, 2, 3);
  connect(n, 1, 4);
  connect(n, 1, 5);
  connect(n, 1, 6);
  connect(n, 6, 7);
  connect(n, 7, 8);
  connect(n, 8, 9);
  std::vector<Neuron> ns;
  ns.push_back(n);
  std::vector<Neuron> ret = Clipping(ns, 2);
  EXPECT_EQ(1, (int)ret.size());
  EXPECT_EQ(1, (int)ret[0].storage_[0]->adjacent_.size());
  EXPECT_EQ(2, (int)ret[0].storage_[1]->adjacent_.size());
}

TEST(Clipping, Clipping2) {
  Neuron n;
  for (int i = 0; i < 10; ++i) {
    add(n, i);
  }
  connect(n, 0, 1);
  connect(n, 1, 2);
  connect(n, 2, 3);
  connect(n, 1, 4);
  connect(n, 1, 5);
  connect(n, 1, 6);
  connect(n, 6, 7);
  connect(n, 7, 8);
  connect(n, 8, 9);
  std::vector<Neuron> ns;
  ns.push_back(n);
  std::vector<Neuron> ret = Clipping(ns, 10);
  EXPECT_EQ(1, (int)ret.size());
  EXPECT_EQ(1, (int)ret[0].storage_[0]->adjacent_.size());
  EXPECT_EQ(2, (int)ret[0].storage_[1]->adjacent_.size());
}
