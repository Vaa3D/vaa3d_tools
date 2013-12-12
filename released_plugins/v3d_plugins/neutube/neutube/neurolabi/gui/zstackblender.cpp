#include "zstackblender.h"

#include <iostream>
#include <cstring>

using namespace std;

ZStackBlender::ZStackBlender()
{
}

ZStack* ZStackBlender::blend(const ZStack &stack1,
                             const ZStack &stack2, double alpha)
{
  if (stack1.kind() != GREY || stack2.kind() != GREY) {
    cout << "Blending failed: invalid type" << endl;
    return NULL;
  }

  if (stack1.width() != stack2.width()) {
    cout << "Blending failed: different width" << endl;
    return NULL;
  }

  if (stack1.height() != stack2.height()) {
    cout << "Blending failed: different height" << endl;
    return NULL;
  }

  if (stack1.depth() != stack2.depth()) {
    cout << "Blending failed: different depth" << endl;
    return NULL;
  }

  size_t volume = stack1.getVoxelNumber();

  int channelNumber = max(stack1.channelNumber(), stack2.channelNumber());
  ZStack *out = new ZStack(GREY, stack1.width(), stack1.height(), stack1.depth(),
                           channelNumber);

  for (int c = 0; c < stack1.channelNumber(); c++) {
    const uint8_t *stack1Array = stack1.array8(c);
    const uint8_t *stack2Array = stack2.array8(c);
    uint8_t *outArray = out->array8(c);

    if (stack1Array != NULL && stack2Array != NULL) {
      for (size_t voxelIndex = 0; voxelIndex < volume; voxelIndex++) {
        double newAlpha = alpha * stack2Array[voxelIndex] / 255.0;
        double v = (1 - newAlpha) * stack1Array[voxelIndex] +
            newAlpha * stack2Array[voxelIndex];
        if (v < 0.0) {
          v = 0.0;
        } else if (v > 255.0) {
          v = 255.0;
        }
        outArray[voxelIndex] = v;
      }
    } else if (stack1Array != NULL) {
      memcpy(outArray, stack1Array, volume);
    } else {
      memcpy(outArray, stack2Array, volume);
    }

  }

  return out;
}
