#pragma once
#include "sigen/common/binary_cube.h"
#include "sigen/common/image_sequence.h"
namespace sigen {
class Binarizer {
public:
  BinaryCube Binarize(const ImageSequence &is);
};
}
