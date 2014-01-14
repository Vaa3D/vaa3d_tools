#ifndef ZDEBUG_H
#define ZDEBUG_H

#include <iostream>
#include <vector>
#include <map>
#include "zopencv_header.h"

static std::ostream &DebugStream = std::cout;

template<typename ArrayType>
void ZDebugPrintArrayG(const ArrayType &a, size_t startIndex, size_t endIndex)
{
  for (size_t index = startIndex; index <= endIndex; ++index) {
    DebugStream << index << ": ";
    DebugStream << a[index] << std::endl;
  }
}

void ZDebugPrintIntArray(const std::vector<int> &a, size_t startIndex, size_t endIndex);
void ZDebugPrintDoubleArray(const std::vector<double> &a, size_t startIndex, size_t endIndex);

template<typename T1, typename T2>
void ZDebugPrintMapG(const std::map<T1, T2> &m, size_t startIndex,
                     size_t endIndex)
{
  size_t index = 0;

  for (typename std::map<T1, T2>::const_iterator iter = m.begin(); iter != m.end();
       ++iter) {
    if (index >= startIndex && index <= (size_t) endIndex) {
      DebugStream << iter->first << " " << iter->second << std::endl;
    }
    ++index;
  }
}


template<typename T1, typename T2>
void ZDebugPrintMapG(const std::map<T1, T2> &m, size_t startIndex = 0)
{
  ZDebugPrintMapG(m, startIndex, m.size() - 1);
}


void ZDebugPrintIntMap(const std::map<int, int> &m, size_t startIndex,
                       size_t endIndex);
void ZDebugPrintIntMap(const std::map<int, int> &m, size_t startIndex = 0);

template<class RandomAccessIterator>
void ZDebugPrintIterator(RandomAccessIterator first, RandomAccessIterator last)
{
  for (RandomAccessIterator iter = first; iter!= last; ++iter) {
    std::cout << *iter << std::endl;
  }
}

#if defined(_USE_OPENCV_)
template <typename T>
void ZDebugPrintCvMatRows(const cv::Mat &mat, int rowStart, int rowEnd)
{
  rowStart = std::max(0, rowStart);
  rowEnd = std::min(rowEnd, mat.rows - 1);
  for (int i = rowStart; i <= rowEnd; ++i) {
    for (int j = 0; j < mat.cols; ++j) {
      std::cout << mat.at<T>(i, j) << " ";
    }
    std::cout << std::endl;
  }
}

void ZDebugPrintCvMatRowsF32(const cv::Mat &mat, int rowStart, int rowEnd);

#endif

#endif // ZDEBUG_H
