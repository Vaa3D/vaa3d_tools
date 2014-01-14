#include "miscutility.h"
#include <iostream>
#include <cmath>
#include "zerror.h"
#include "c_stack.h"
#include "tz_math.h"

using namespace std;

void misc::paintRadialHistogram(
    const ZHistogram hist, double cx, double cy, int z, Stack *stack)
{
  PROCESS_WARNING(stack == NULL, "null stack", return);
  PROCESS_WARNING(C_Stack::kind(stack) != GREY, "GREY kind only", return);

  if (z < 0 || z >= C_Stack::depth(stack)) {
    return;
  }

  ZHistogram histForPaint = hist;
  histForPaint.normalize();

  int width = C_Stack::width(stack);
  int height = C_Stack::height(stack);

  uint8_t *array = C_Stack::array8(stack);

  size_t offset = width * height;
  offset *= z;
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      double dx = x - cx;
      double dy = y - cy;
      double dist = sqrt(dx * dx + dy * dy);
#ifdef _DEBUG_2
      std::cout << dist << ": " << histForPaint.getDensity(dist) << std::endl;
#endif
      int v = iround(histForPaint.getDensity(dist) * 255.0);
      array[offset++] = CLIP_VALUE(v, 0, 255);
    }
  }
}

void misc::paintRadialHistogram2D(
    const vector<ZHistogram> hist, double cx, int startZ, Stack *stack)
{
  PROCESS_WARNING(stack == NULL, "null stack", return);
  PROCESS_WARNING(C_Stack::kind(stack) != GREY, "GREY kind only", return);

  int width = C_Stack::width(stack);
  int height = C_Stack::height(stack);

  uint8_t *array = C_Stack::array8(stack);

  int y = startZ;

  for (vector<ZHistogram>::const_iterator histIter = hist.begin();
       histIter != hist.end(); ++histIter, ++y) {

    if (y >= height) {
      break;
    }

    ZHistogram histForPaint = *histIter;
    histForPaint.normalize();

    size_t offset = y *  width;
    for (int x = 0; x < width; ++x) {
      double dist = fabs(x - cx);
#ifdef _DEBUG_2
      std::cout << dist << ": " << histForPaint.getDensity(dist) << std::endl;
#endif
      int v = iround(histForPaint.getDensity(dist) * 255.0);
      array[offset++] = CLIP_VALUE(v, 0, 255);
    }
  }
}
