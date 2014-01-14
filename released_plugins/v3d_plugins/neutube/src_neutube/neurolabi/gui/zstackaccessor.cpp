#include "zstackaccessor.h"
#include <string.h>
#include "tz_image_io.h"
#include "tz_file_list.h"

using namespace std;

//Suppor 100M buffer
int ZStackAccessor::m_capacity = 104857600;

ZStackAccessor::ZStackAccessor()
{
  m_buffer = NULL;
  m_bufferZStart = 0;
  m_width = 0;
  m_height = 0;
  m_byteNumberPerVoxel = 2;
  m_bufferDepth = 0;
  m_planeSize = 0;
  m_area = 0;
  m_rowSize = 0;
}

int ZStackAccessor::bufferDepthCapacity()
{
  int planeSize = m_width * m_height * m_byteNumberPerVoxel;

  if (planeSize == 0) {
    return 0;
  }

  return m_capacity / planeSize;
}

int ZStackAccessor::bufferDepth()
{
  return m_bufferDepth;
}

int ZStackAccessor::stackDepth()
{
  return static_cast<int>(m_fileList.size());
}

void ZStackAccessor::updateBuffer(int z)
{
  int bdepth = bufferDepth();
  int newBufferZStart = m_bufferZStart;
  int zStart = newBufferZStart; //The first image to load
  int depth = 0;       //Number of images to be loaded in the buffer
  int bufferShift = 0; //Shift of the buffer to load new image

  if ((z >= m_bufferZStart + bdepth) || (z < m_bufferZStart)) {
    newBufferZStart = max(0, z - bdepth / 2);
    newBufferZStart = min(newBufferZStart, stackDepth() - bdepth);
    zStart = newBufferZStart;

    int planeSize = m_width * m_height * m_byteNumberPerVoxel;

    if (m_buffer != NULL) {
      if (newBufferZStart != m_bufferZStart) {
        depth = bdepth;
        int availableStart = m_bufferZStart;
        int availableDepth = 0;
        int zShift = -1;
        if (newBufferZStart > m_bufferZStart) {
          int bufferEnd = m_bufferZStart + bdepth;
          if (newBufferZStart < bufferEnd) {
            zShift = 0;
            availableStart = newBufferZStart - m_bufferZStart;
            availableDepth = bdepth - availableStart;
            bufferShift = availableDepth;
            zStart += availableDepth;
          }
        } else {
          if (newBufferZStart + bdepth > m_bufferZStart) {
            availableStart = 0;
            availableDepth = newBufferZStart + bdepth - m_bufferZStart;
            zShift = m_bufferZStart - newBufferZStart;
          }
        }

        if (availableDepth > 0) {
          depth -= availableDepth;
          memmove((char*)m_buffer + zShift * planeSize,
                  (char*)m_buffer + availableStart * planeSize,
                  planeSize * availableDepth);
        }
      }
    } else {
      m_bufferDepth = min(static_cast<int>(m_fileList.size()),
                          bufferDepthCapacity());
      m_buffer = (uint16_t*)
          malloc(m_width * m_height * bufferDepth() * m_byteNumberPerVoxel);
      depth = bufferDepth();
      newBufferZStart = max(0, z - depth / 2);
      newBufferZStart = min(newBufferZStart, stackDepth() - depth);
      zStart = newBufferZStart;
    }
  }

  loadImage(bufferShift, zStart, depth);

  m_bufferZStart = newBufferZStart;
}

void ZStackAccessor::loadImage(int bufferShift, int zStart, int depth)
{
  int planeSize = m_width * m_height * m_byteNumberPerVoxel;

  char *buffer = (char*) m_buffer + planeSize * bufferShift;
  for (int i = 0; i < depth; i++) {
    Stack *stack = Read_Stack_U(m_fileList[i + zStart].c_str());
    memcpy((char*)buffer + planeSize * i, stack->array, planeSize);
    Kill_Stack(stack);
  }
}

void ZStackAccessor::attachFileList(std::string dirPath, std::string ext)
{
  File_List *fileList = File_List_Load_Dir(dirPath.c_str(), ext.c_str(), NULL);
  File_List_Sort_By_Number(fileList);
  m_fileList.resize(fileList->file_number);
  for (int i = 0; i < fileList->file_number; i++) {
    m_fileList[i] = fileList->file_path[i];
  }

  if (m_fileList.size() > 0) {
    Stack *stack = Read_Stack_U(fileList->file_path[0]);
    m_width = stack->width;
    m_height = stack->height;
    m_byteNumberPerVoxel = stack->kind;
    m_area = m_width * m_height;
    m_planeSize = m_area * m_byteNumberPerVoxel;
    m_rowSize = m_width * m_byteNumberPerVoxel;
    Kill_Stack(stack);
  }
}

void ZStackAccessor::exportBuffer(std::string filePath)
{
  int depth = min(stackDepth(), bufferDepth());

  if ((m_buffer != NULL) && (depth > 0)) {
    Stack stack;
    stack.array = (uint8*) m_buffer;
    stack.width = m_width;
    stack.height = m_height;
    stack.depth = depth;
    stack.kind = m_byteNumberPerVoxel;
    stack.text = const_cast<char*>("");

    Write_Stack_U(filePath.c_str(), &stack, NULL);
  }
}

double ZStackAccessor::voxel(int x, int y, int z)
{
  updateBuffer(z);
  z -= m_bufferZStart;

  return m_buffer[z * m_width * m_height + y * m_width + x];
}

void* ZStackAccessor::rowPointer(int y, int z)
{
  updateBuffer(z);

  z-= m_bufferZStart;

  char *buffer = (char*) m_buffer +
      (z * m_area + y * m_width) * m_byteNumberPerVoxel;

  return static_cast<void*>(buffer);
}

void* ZStackAccessor::planePointer(int z)
{
  updateBuffer(z);

  z-= m_bufferZStart;

  char *buffer = (char*) m_buffer + z * m_planeSize;

  return static_cast<void*>(buffer);
}

void* ZStackAccessor::wholePointer()
{
  if (bufferDepth() >= stackDepth()) {
    return static_cast<void*>(m_buffer);
  }

  return NULL;
}
