#include "sigen/builder/builder.h"
#include "sigen/common/disjoint_set.h"
#include <algorithm>
#include <boost/foreach.hpp>
#include <cassert>
#include <cmath>
#include <iostream>
#include <map>
#include <queue>
#include <set>
#include <stack>
#include <utility>
#include <vector>
namespace sigen {
void Builder::ConnectNeighbors() {
  std::multimap<IPoint, int> coord_to_index;
  for (int i = 0; i < (int)data_.size(); ++i) {
    BOOST_FOREACH (const IPoint &p, data_[i]->points_) {
      coord_to_index.insert(std::make_pair(p, i));
    }
  }
  for (int i = 0; i < (int)data_.size(); ++i) {
    BOOST_FOREACH (const IPoint &p, data_[i]->points_) {
      for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
          for (int dz = -1; dz <= 1; ++dz) {
            const IPoint q(p.x_ + dx, p.y_ + dy, p.z_ + dz);
            typedef std::multimap<IPoint, int>::iterator iter_type;
            std::pair<iter_type, iter_type> range = coord_to_index.equal_range(q);
            for (iter_type it = range.first; it != range.second; ++it) {
              if (it->second != i && data_[i]->HasConnection(data_[it->second]) == false) {
                data_[i]->AddConnection(data_[it->second]);
              }
            }
          }
        }
      }
    }
  }
}

void Builder::CutLoops() {
  assert(is_radius_computed_);
  // use kruskal like algorithm
  // see https://en.wikipedia.org/wiki/Kruskal%27s_algorithm
  DisjointSet<Cluster *> U;
  typedef std::pair<double, std::pair<Cluster *, Cluster *> > item_type;
  std::vector<item_type> E;
  BOOST_FOREACH (ClusterPtr cls, data_) {
    U.Add(cls.get());
    BOOST_FOREACH (Cluster *adj, cls->adjacent_) {
      double strength = (cls->radius_ + adj->radius_) / 2.0;
      Cluster *a = cls.get(), *b = adj;
      if (a < b) {
        E.push_back(std::make_pair(strength, std::make_pair(cls.get(), adj)));
      }
    }
  }
  U.SetUp();
  std::sort(E.begin(), E.end());
  std::reverse(E.begin(), E.end());
  BOOST_FOREACH (const item_type &it, E) {
    Cluster *a = it.second.first;
    Cluster *b = it.second.second;
    if (U.IsSame(a, b)) {
      a->RemoveConnection(b);
      b->RemoveConnection(a);
    } else {
      U.Merge(a, b);
    }
  }
}

static NeuronNode *findEdgeNode(NeuronNode *node) {
  std::queue<NeuronNode *> que;
  NeuronNode *last = node;
  for (int i = 0; i < 2; ++i) {
    std::set<NeuronNode *> used;
    que.push(last);
    used.insert(last);
    while (!que.empty()) {
      NeuronNode *cur = que.front();
      que.pop();
      last = cur;
      BOOST_FOREACH (NeuronNode *next, cur->adjacent_) {
        if (used.count(next)) {
          continue;
        }
        que.push(next);
        used.insert(next);
      }
    }
  }
  return last;
}

void Builder::ComputeGravityPoints() {
  BOOST_FOREACH (ClusterPtr cls, data_) {
    cls->UpdateGravityPoint();
  }
}

void Builder::ComputeRadius() {
  BOOST_FOREACH (ClusterPtr cls, data_) {
    cls->UpdateRadius(scale_xy_, scale_z_);
  }
  is_radius_computed_ = true;
}

std::vector<NeuronNodePtr> Builder::ConvertToNeuronNodes() {
  std::vector<NeuronNodePtr> neuron_nodes;
  BOOST_FOREACH (ClusterPtr p, data_) {
    NeuronNodePtr n = boost::make_shared<NeuronNode>();
    n->setCoord(p->gx_ * scale_xy_,
                p->gy_ * scale_xy_,
                p->gz_ * scale_z_);
    n->radius_ = p->radius_;
    neuron_nodes.push_back(n);
  }
  std::map<Cluster *, int> ptr2index;
  for (int i = 0; i < (int)data_.size(); ++i) {
    ptr2index[data_[i].get()] = i;
  }
  for (int i = 0; i < (int)data_.size(); ++i) {
    BOOST_FOREACH (Cluster *p, data_[i]->adjacent_) {
      int j = ptr2index[p];
      if (i < j) {
        neuron_nodes[i]->AddConnection(neuron_nodes[j]);
        neuron_nodes[j]->AddConnection(neuron_nodes[i]);
      }
    }
  }
  return neuron_nodes;
}

std::vector<Neuron> Builder::ConvertToNeuron() {
  std::vector<NeuronNodePtr> neuron_nodes = ConvertToNeuronNodes();
  // split into some neurons
  std::set<NeuronNode *> used;
  std::vector<Neuron> neurons;
  std::map<NeuronNode *, NeuronNodePtr> ptr2smartptr;

  BOOST_FOREACH (NeuronNodePtr p, neuron_nodes) {
    ptr2smartptr[p.get()] = p;
  }

  BOOST_FOREACH (NeuronNodePtr node, neuron_nodes) {
    if (used.count(node.get())) {
      continue;
    }
    // FIXME
    neurons.push_back(Neuron());
    Neuron &n = neurons.back();
    n.set_root(findEdgeNode(node.get()));
    n.AddNode(node);
    std::stack<NeuronNode *> stk;
    stk.push(node.get());
    used.insert(node.get());
    while (!stk.empty()) {
      NeuronNode *cur = stk.top();
      stk.pop();
      BOOST_FOREACH (NeuronNode *next, cur->adjacent_) {
        // If next != parent
        if (!used.count(next)) {
          n.AddNode(ptr2smartptr[next]);
          stk.push(next);
          used.insert(next);
        }
      }
    }
  }
  return neurons;
}

static void computeIdInner(NeuronNode *cur, NeuronNode *prev, int &id) {
  cur->id_ = id++;
  BOOST_FOREACH (NeuronNode *next, cur->adjacent_) {
    if (next != prev) {
      computeIdInner(next, cur, id);
    }
  }
}

void Builder::ComputeIds(std::vector<Neuron> &neurons) {
  int id = 1;
  for (int i = 0; i < (int)neurons.size(); ++i) {
    computeIdInner(neurons[i].get_root(), NULL, id);
  }
}

static void computeNodeTypesInner(NeuronNode *cur, NeuronNode *prev) {
  cur->UpdateNodeType();
  BOOST_FOREACH (NeuronNode *next, cur->adjacent_) {
    if (next != prev) {
      computeNodeTypesInner(next, cur);
    }
  }
}

void Builder::ComputeNodeTypes(std::vector<Neuron> &neurons) {
  for (int i = 0; i < (int)neurons.size(); ++i) {
    computeNodeTypesInner(neurons[i].get_root(), NULL);
  }
}

std::vector<Neuron> Builder::Build() {
  bool print_progress = true;
  ComputeGravityPoints();
  if (print_progress)
    std::cerr << "compute_gravity_point" << std::endl;
  ComputeRadius();
  if (print_progress)
    std::cerr << "compute_radius" << std::endl;
  ConnectNeighbors();
  if (print_progress)
    std::cerr << "connect_neighbor" << std::endl;
  CutLoops();
  if (print_progress)
    std::cerr << "cut_loops" << std::endl;
  std::vector<Neuron> neurons = ConvertToNeuron();
  if (print_progress)
    std::cerr << "convert_to_neuron" << std::endl;
  ComputeIds(neurons);
  if (print_progress)
    std::cerr << "compute_id" << std::endl;
  ComputeNodeTypes(neurons);
  if (print_progress)
    std::cerr << "compute_node_type" << std::endl;
  return neurons;
}
} // namespace sigen
