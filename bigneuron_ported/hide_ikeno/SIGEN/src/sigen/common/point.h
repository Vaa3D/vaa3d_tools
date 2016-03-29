#pragma once
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>
namespace sigen {
class IPoint {
public:
  int x_, y_, z_;
  IPoint(const int x, const int y, const int z) : x_(x), y_(y), z_(z) {}
  bool operator<(const IPoint &other) const {
    return boost::tie(x_, y_, z_) < boost::tie(other.x_, other.y_, other.z_);
  }
};
} // namespace sigen
