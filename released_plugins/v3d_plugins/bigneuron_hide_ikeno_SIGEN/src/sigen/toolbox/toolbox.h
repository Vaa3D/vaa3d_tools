#pragma once
#include "sigen/common/neuron.h"
#include <vector>
namespace sigen {
std::vector<Neuron> Interpolate(const std::vector<Neuron> &input, const double dt, const int vt);
std::vector<Neuron> Smoothing(const std::vector<Neuron> &input, const int n_iter);
std::vector<Neuron> Clipping(const std::vector<Neuron> &input, const int level);
}
