#ifndef ZSTACKACCESSOR_H
#define ZSTACKACCESSOR_H

#include <vector>
#include <string>

#include "tz_stdint.h"
#include "tz_stack_lib.h"

class ZStack;

class ZStackAccessor
{
public:
  ZStackAccessor();

public:
  double voxel(int x, int y, int z);
  void* rowPointer(int y, int z);
  void* planePointer(int z);
  void* wholePointer();

  void updateBuffer(int z);

  void attachFileList(std::string dirPath, std::string ext);

  int bufferDepth();
  int stackDepth();
  int bufferDepthCapacity();

  void loadImage(int bufferShift, int zStart, int depth);

  void exportBuffer(std::string filePath);

  static inline void setCapacity(int capacity) { m_capacity = capacity; }

  Stack completeStack();

private:
  uint16_t *m_buffer;
  int m_bufferZStart;
  std::vector<std::string> m_fileList;
  int m_width;
  int m_height;
  int m_byteNumberPerVoxel;
  int m_bufferDepth;
  int m_planeSize;
  int m_area;
  int m_rowSize;
  static int m_capacity;
};

#endif // ZSTACKACCESSOR_H
