#pragma once
#include "sigen/common/binary_cube.h"
#include "sigen/common/cluster.h"
#include "sigen/common/voxel.h"
#include <boost/utility.hpp>
#include <vector>
namespace sigen {
class Extractor : boost::noncopyable {
  void Labeling();

public:
  BinaryCube cube_;
  std::vector<std::vector<VoxelPtr> > components_;
  explicit Extractor(const BinaryCube &cube) : cube_(cube) {}
  std::vector<ClusterPtr> Extract();
};
} // namespace sigen
