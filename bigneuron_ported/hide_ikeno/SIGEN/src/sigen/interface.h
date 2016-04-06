#pragma once
#include "sigen/common/binary_cube.h"
#include <vector>
namespace sigen {
namespace interface {
struct Options {
public:
  double scale_xy;
  double scale_z;
  bool enable_interpolation;
  int volume_threshold;
  double distance_threshold;
  bool enable_smoothing;
  int smoothing_level;
  bool enable_clipping;
  int clipping_level;
};
void Extract(const BinaryCube &cube,
             std::vector<int> &out_n, std::vector<int> &out_type,
             std::vector<double> &out_x, std::vector<double> &out_y, std::vector<double> &out_z,
             std::vector<double> &out_r, std::vector<int> &out_pn, const Options &options);
} // namespace interface
} // namespace sigen
