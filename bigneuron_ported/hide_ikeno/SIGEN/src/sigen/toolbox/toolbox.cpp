#include "sigen/toolbox/toolbox.h"
#include "sigen/common/disjoint_set.h"
#include "sigen/common/math.h"
#include <algorithm>
#include <boost/foreach.hpp>
#include <boost/scoped_array.hpp>
#include <cmath>
#include <functional>
#include <iostream>
#include <kdtree/kdtree.h>
#include <limits>
#include <map>
#include <queue>
#include <set>
#include <utility>
#include <vector>
namespace sigen {
static double norm2(
    const NeuronNodePtr &a,
    const NeuronNodePtr &b) {
  const double dx = std::abs(a->gx_ - b->gx_);
  const double dy = std::abs(a->gy_ - b->gy_);
  const double dz = std::abs(a->gz_ - b->gz_);
  return std::sqrt(dx * dx + dy * dy + dz * dz);
}

// N = left.NumNodes()
// M = right.NumNodes()
// O(N*M)
static std::pair<double, std::pair<int, int> > normNeuronFastPath(const Neuron &left, const Neuron &right) {
  int l, r;
  double minimum = std::numeric_limits<double>::max();
  for (int i = 0; i < (int)left.NumNodes(); ++i) {
    for (int j = 0; j < (int)right.NumNodes(); ++j) {
      double d = norm2(left.storage_[i], right.storage_[j]);
      if (minimum > d) {
        minimum = d;
        l = i;
        r = j;
      }
    }
  }
  return std::make_pair(minimum, std::make_pair(l, r));
}

// N = left.NumNodes()
// M = right.NumNodes()
// O((N + M) log N)
// Use https://github.com/jtsiomb/kdtree
static std::pair<double, std::pair<int, int> > normNeuronSlowPath(const Neuron &left, const Neuron &right) {
  kdtree *tree = kd_create(3);

  boost::scoped_array<int> indexes(new int[left.NumNodes()]);
  for (int i = 0; i < (int)left.NumNodes(); ++i) {
    indexes[i] = i;
    kd_insert3(
        tree,
        left.storage_[i]->gx_,
        left.storage_[i]->gy_,
        left.storage_[i]->gz_,
        &indexes[i]);
  }

  int l, r;
  double minimum = std::numeric_limits<double>::max();
  for (int j = 0; j < (int)right.NumNodes(); ++j) {
    kdres *set = kd_nearest3(
        tree,
        right.storage_[j]->gx_,
        right.storage_[j]->gy_,
        right.storage_[j]->gz_);
    int i = *(int *)kd_res_item_data(set);
    kd_res_free(set);
    double d = norm2(left.storage_[i], right.storage_[j]);
    if (minimum > d) {
      minimum = d;
      l = i;
      r = j;
    }
  }

  kd_free(tree);
  return std::make_pair(minimum, std::make_pair(l, r));
}

static std::pair<double, std::pair<int, int> > normNeuron(const Neuron &left, const Neuron &right) {
  assert(!left.IsEmpty());
  assert(!right.IsEmpty());
  if (std::min(left.NumNodes(), right.NumNodes()) >= 300) {
    return normNeuronSlowPath(left, right);
  } else {
    return normNeuronFastPath(left, right);
  }
}

std::vector<Neuron> Interpolate(const std::vector<Neuron> &input, const double dt, const int vt) {
  const int N = input.size();
  std::vector<Neuron> forest;
  for (int i = 0; i < N; ++i) {
    forest.push_back(input[i].Clone());
  }
  DisjointSet<int> set;
  std::vector<bool> is_not_small(forest.size(), false);
  for (int i = 0; i < N; ++i) {
    if (forest[i].NumNodes() >= vt) {
      set.Add(i);
      is_not_small[i] = true;
    }
  }
  set.SetUp();

  typedef std::pair<double, std::pair<int, int> > priorityQueueNode;
  std::priority_queue<
      priorityQueueNode,
      std::vector<priorityQueueNode>,
      std::greater<priorityQueueNode> >
      pq;

  std::vector<std::map<int, double> > distance(N);

  for (int i = 0; i < N; ++i) {
    if (is_not_small[i]) {
      for (int j = i + 1; j < N; ++j) {
        if (is_not_small[j]) {
          assert(i != j);
          double d = normNeuron(forest[i], forest[j]).first;
          if (d <= dt) {
            pq.push(std::make_pair(d, std::make_pair(i, j)));
            distance[i][j] = d;
            distance[j][i] = d;
          }
        }
      }
    }
  }
  while (!pq.empty()) {
    priorityQueueNode node = pq.top();
    pq.pop();
    int l = node.second.first;
    int r = node.second.second;
    if (forest[l].IsEmpty())
      continue;
    if (forest[r].IsEmpty())
      continue;
    if (set.IsSame(l, r))
      continue;
    std::pair<double, std::pair<int, int> > dist = normNeuron(forest[l], forest[r]);
    set.Merge(l, r);
    forest[l].ConnectToOtherNeuron(dist.second.first, forest[r], dist.second.second);
    forest[r].ConnectToOtherNeuron(dist.second.second, forest[l], dist.second.first);
    forest[l].Extend(forest[r]);
    forest[r].Clear();

    for (std::map<int, double>::iterator it = distance[r].begin(); it != distance[r].end(); ++it) {
      int i = it->first;
      double d = it->second;
      assert(is_not_small[i]);
      if (set.IsSame(l, i) == false && forest[i].IsEmpty() == false) {
        if (distance[l].count(i)) {
          distance[l][i] = std::min(distance[l][i], d);
        } else {
          distance[l][i] = d;
        }

        if (distance[i].count(l)) {
          distance[i][l] = std::min(distance[i][l], d);
        } else {
          distance[i][l] = d;
        }

        pq.push(std::make_pair(d, std::make_pair(l, i)));
      }
    }
  }
  for (int i = 0; i < (int)forest.size(); ++i) {
    if (forest[i].IsEmpty()) {
      forest.erase(forest.begin() + i);
      i--;
    }
  }
  return forest;
}

struct PointAndRadius {
  double gx_, gy_, gz_, radius_;
  void setCoord(const double gx, const double gy, const double gz) {
    gx_ = gx;
    gy_ = gy;
    gz_ = gz;
  }
};

std::vector<Neuron> Smoothing(const std::vector<Neuron> &input, const int n_iter) {
  std::vector<Neuron> forest;
  for (int i = 0; i < (int)input.size(); ++i) {
    forest.push_back(input[i].Clone());
  }
  for (int iter = 0; iter < n_iter; ++iter) {
    std::map<int, PointAndRadius> next_value;
    for (int i = 0; i < (int)forest.size(); ++i) {
      BOOST_FOREACH (NeuronNodePtr node, forest[i].storage_) {
        std::vector<double> gx, gy, gz, radius;
        gx.push_back(node->gx_);
        gy.push_back(node->gy_);
        gz.push_back(node->gz_);
        radius.push_back(node->radius_);
        BOOST_FOREACH (NeuronNode *adj, node->adjacent_) {
          gx.push_back(adj->gx_);
          gy.push_back(adj->gy_);
          gz.push_back(adj->gz_);
          radius.push_back(adj->radius_);
        }
        PointAndRadius next_node;
        next_node.setCoord(Mean(gx), Mean(gy), Mean(gz));
        next_node.radius_ = Mean(radius);
        next_value[node->id_] = next_node;
      }
    }
    for (int i = 0; i < (int)forest.size(); ++i) {
      BOOST_FOREACH (NeuronNodePtr node, forest[i].storage_) {
        PointAndRadius next_node = next_value[node->id_];
        node->setCoord(next_node.gx_, next_node.gy_, next_node.gz_);
        node->radius_ = next_node.radius_;
      }
    }
  }
  return forest;
}

// return max_height
static int clippingDfs(
    NeuronNode *node,
    NeuronNode *parent,
    const int level,
    std::set<int> &will_remove,
    std::map<NeuronNode *, int> &memo) {
  if (memo.count(node))
    return memo[node];
  if (node->CountNumChild(parent) < 2) {
    // If count_num_child == 1
    BOOST_FOREACH (NeuronNode *next, node->adjacent_) {
      if (next != parent) {
        return memo[node] = clippingDfs(next, node, level, will_remove, memo) + 1;
      }
    }
    // If count_num_child == 0
    return 1;
  }
  int has_longpath = 0;
  BOOST_FOREACH (NeuronNode *next, node->adjacent_) {
    if (next != parent) {
      int depth = clippingDfs(next, node, level, will_remove, memo);
      if (depth > level)
        has_longpath = true;
    }
  }
  if (has_longpath) {
    int maxdepth = 0;
    BOOST_FOREACH (NeuronNode *next, node->adjacent_) {
      if (next != parent) {
        int depth = clippingDfs(next, node, level, will_remove, memo);
        if (depth <= level) {
          will_remove.insert(next->id_);
        }
        maxdepth = std::max(maxdepth, depth);
      }
    }
    return memo[node] = maxdepth + 1;
  } else {
    int maxdepth = 0;
    NeuronNode *longest_child = NULL;
    BOOST_FOREACH (NeuronNode *next, node->adjacent_) {
      if (next != parent) {
        int depth = clippingDfs(next, node, level, will_remove, memo);
        if (maxdepth < depth) {
          maxdepth = depth;
          longest_child = next;
        }
      }
    }
    if (maxdepth > 0) {
      BOOST_FOREACH (NeuronNode *next, node->adjacent_) {
        if (next != parent && next != longest_child) {
          will_remove.insert(next->id_);
        }
      }
    }
    return memo[node] = maxdepth + 1;
  }
}

std::vector<Neuron> Clipping(const std::vector<Neuron> &input, const int level) {
  std::set<int> will_remove;
  std::vector<Neuron> forest;
  std::map<NeuronNode *, int> memo;
  for (int i = 0; i < (int)input.size(); ++i) {
    forest.push_back(input[i].Clone());
    clippingDfs(forest[i].get_root(), NULL, level, will_remove, memo);
  }
  for (int i = 0; i < (int)forest.size(); ++i) {
    forest[i].RemoveConnections(will_remove);
  }
  return forest;
}
} // namespace sigen
