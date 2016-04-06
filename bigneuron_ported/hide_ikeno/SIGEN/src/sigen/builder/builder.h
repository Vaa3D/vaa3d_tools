#pragma once
#include "sigen/common/cluster.h"
#include "sigen/common/neuron.h"
#include <boost/utility.hpp>
#include <vector>
namespace sigen {
class Builder : boost::noncopyable {
  bool is_radius_computed_;
  const double scale_xy_, scale_z_;

public:
  std::vector<ClusterPtr> data_;
  explicit Builder(const std::vector<ClusterPtr> &data,
                   const double scale_xy,
                   const double scale_z)
      : is_radius_computed_(false), scale_xy_(scale_xy), scale_z_(scale_z), data_(data) {}
  std::vector<Neuron> Build();
  std::vector<Neuron> ConvertToNeuron();
  std::vector<NeuronNodePtr> ConvertToNeuronNodes();
  static void ComputeNodeTypes(std::vector<Neuron> &neurons);
  static void ComputeIds(std::vector<Neuron> &neurons);
  void ConnectNeighbors();
  void CutLoops();
  void ComputeGravityPoints();
  void ComputeRadius();
};
} // namespace sigen
