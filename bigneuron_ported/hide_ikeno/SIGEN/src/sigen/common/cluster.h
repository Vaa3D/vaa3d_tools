#pragma once
#include "sigen/common/math.h"
#include "sigen/common/point.h"
#include "sigen/common/voxel.h"
#include <algorithm>
#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>
#include <cassert>
#include <cmath>
#include <set>
#include <vector>
namespace sigen {
class Cluster;
typedef boost::shared_ptr<Cluster> ClusterPtr;
class Cluster : boost::noncopyable {
  bool is_gravity_point_computed_;

public:
  double gx_, gy_, gz_;
  double radius_;
  std::vector<IPoint> points_;
  std::set<Cluster *> adjacent_;
  explicit Cluster(const std::vector<IPoint> &points)
      : is_gravity_point_computed_(false),
        gx_(0.0), gy_(0.0), gz_(0.0),
        radius_(0.0), points_(points) {}

  // http://stackoverflow.com/questions/5727264
  bool HasConnection(const Cluster *p) const {
    return adjacent_.count(const_cast<Cluster *>(p)) > 0; // FIXME please remove const_cast
  }
  bool HasConnection(ClusterPtr p) const {
    return this->HasConnection(p.get());
  }
  void RemoveConnection(const Cluster *p) {
    assert(this->HasConnection(p));
    adjacent_.erase(const_cast<Cluster *>(p)); // FIXME please remove const_cast
  }
  void AddConnection(const Cluster *p) {
    assert(!this->HasConnection(p));
    adjacent_.insert(const_cast<Cluster *>(p)); // FIXME please remove const_cast
  }
  void AddConnection(ClusterPtr p) {
    this->AddConnection(p.get());
  }
  void UpdateGravityPoint() {
    assert(!points_.empty());
    std::vector<double> sx, sy, sz;
    BOOST_FOREACH (const IPoint &p, points_) {
      sx.push_back(p.x_);
      sy.push_back(p.y_);
      sz.push_back(p.z_);
    }
    gx_ = Mean(sx);
    gy_ = Mean(sy);
    gz_ = Mean(sz);
    is_gravity_point_computed_ = true;
  }
  void UpdateRadius(const double scale_xy, const double scale_z) {
    assert(is_gravity_point_computed_);
    double mx = 0.0;
    BOOST_FOREACH (const IPoint &p, points_) {
      double dx = scale_xy * (p.x_ - gx_);
      double dy = scale_xy * (p.y_ - gy_);
      double dz = scale_z * (p.z_ - gz_);
      mx = std::max(mx, std::sqrt(dx * dx + dy * dy + dz * dz));
    }
    radius_ = mx;
  }
};
} // namespace sigen
