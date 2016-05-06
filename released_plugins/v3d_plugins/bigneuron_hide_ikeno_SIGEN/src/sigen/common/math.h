#pragma once
#include <vector>
namespace sigen {
inline double Mean(const std::vector<double> &xs) {
  if (xs.empty())
    return 0.0;
  double sum = 0.0;
  for (int i = 0; i < (int)xs.size(); ++i) {
    sum += xs[i];
  }
  return sum / xs.size();
}
} // namespace sigen
