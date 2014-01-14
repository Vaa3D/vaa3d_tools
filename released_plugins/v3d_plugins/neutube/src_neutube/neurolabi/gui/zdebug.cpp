#include "zdebug.h"

//std::ostream& ZDebug::DebugStream = std::cout;

void ZDebugPrintIntArray(const std::vector<int> &a, size_t startIndex, size_t endIndex)
{
  ZDebugPrintArrayG(a, startIndex, endIndex);
}

void ZDebugPrintDoubleArray(const std::vector<double> &a, size_t startIndex, size_t endIndex)
{
  ZDebugPrintArrayG(a, startIndex, endIndex);
}

void ZDebugPrintIntMap(const std::map<int, int> &m, size_t startIndex, size_t endIndex)
{
  ZDebugPrintMapG(m, startIndex, endIndex);
}

void ZDebugPrintIntMap(const std::map<int, int> &m, size_t startIndex)
{
  ZDebugPrintMapG(m, startIndex);
}

#if defined(_USE_OPENCV_)
void ZDebugPrintCvMatRowsF32(const cv::Mat &mat, int rowStart, int rowEnd)
{
  ZDebugPrintCvMatRows<float>(mat, rowStart, rowEnd);
}

#endif
