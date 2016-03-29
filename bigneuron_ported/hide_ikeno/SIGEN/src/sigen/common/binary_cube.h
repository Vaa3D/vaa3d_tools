#pragma once
#include <vector>
namespace sigen {
// this class represents bool[][][]
class BinaryCube {
public:
  int x_, y_, z_;
  std::vector<std::vector<std::vector<bool> > > data_;
  BinaryCube(int x, int y, int z)
      : x_(x), y_(y), z_(z),
        data_(x, std::vector<std::vector<bool> >(y, std::vector<bool>(z))) {}
  // setter
  std::vector<std::vector<bool> > &operator[](int index) {
    return data_[index];
  }
  // getter
  const std::vector<std::vector<bool> > &operator[](int index) const {
    return data_[index];
  }
  void Clear() {
    data_.clear();
  }
};
} // namespace sigen
