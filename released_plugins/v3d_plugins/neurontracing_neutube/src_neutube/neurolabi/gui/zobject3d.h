/**@file zobject3d.h
 * @brief 3D object class
 * @author Ting Zhao
 */
#ifndef ZOBJECT3D_H
#define ZOBJECT3D_H

#include "zqtheader.h"

#include <vector>
#include <string>

#include "tz_object_3d.h"
#include "zdocumentable.h"
#include "zstackdrawable.h"
#include "tz_fmatrix.h"
#include "zpoint.h"
#include "tz_stack_utils.h"

#ifndef INT_VOXEL_TYPE
#error "Incompatible voxel type"
#endif

class ZStack;
class ZObject3dArray;

class ZObject3d : public ZDocumentable, public ZStackDrawable {
public:
  ZObject3d(Object_3d *obj = NULL);
  ZObject3d(const std::vector<size_t> &indexArray, int width, int height,
            int dx, int dy, int dz);
  virtual ~ZObject3d();

  virtual const std::string& className() const;

  virtual void save(const char *filePath);
  virtual void load(const char *filePath);
  virtual void display(QPainter &painter, int z = 0, Display_Style option = NORMAL)
  const;

public:
  inline size_t size() const { return m_voxelArray.size() / 3; }
  inline void setSize(size_t s) { m_voxelArray.resize(s * 3); }
  inline int x(size_t index) const { return m_voxelArray[index * 3]; }
  inline int y(size_t index) const { return m_voxelArray[index * 3 + 1]; }
  inline int z(size_t index) const { return m_voxelArray[index * 3 + 2]; }
  void set(int index, int x, int y, int z);
  void set(int index, Voxel_t voxel);
  void set(int index, size_t voxelIndex, int width, int height,
           int dx, int dy, int dz);
  inline void setX(int index, int x) { m_voxelArray[index * 3] = x; }
  inline void setY(int index, int y) { m_voxelArray[index * 3 + 1] = y; }
  inline void setZ(int index, int z) { m_voxelArray[index * 3 + 2] = z; }

  bool isEmpty();

  void append(int x, int y, int z);

  void setLine(ZPoint start, ZPoint end);

  Object_3d* c_obj() const;

  void labelStack(Stack *stack, int label = 1);
  void labelStack(Stack *stack, int label, int dx, int dy, int dz);

  //For tbar detection specifically
  ZPoint computeCentroid(FMatrix *matrix);

  template <typename T>
  std::vector<T> toIndexArray(int width, int height, int depth);
  template <typename T>
  std::vector<T> toIndexArray(int width, int height, int depth,
                              int offsetX, int offsetY, int offsetZ);

  void sortByIndex();

  //static bool compareByIndex(const )

  inline void clear() { m_voxelArray.clear(); }

  void translate(int x, int y, int z);

  inline std::vector<int> voxelArray() { return m_voxelArray; }

  void exportSwcFile(std::string filePath);
  void exportCsvFile(std::string filePath);

  ZObject3d *clone() const;

  double averageIntensity(const Stack *stack) const;

  void print();

  inline int lastX() { return *(m_voxelArray.end() - 3); }
  inline int lastY() { return *(m_voxelArray.end() - 2); }
  inline int lastZ() { return *(m_voxelArray.end() - 1); }

  void getRange(int *corner) const;

  ZObject3dArray* growLabel(const ZObject3d &seed, int growLevel = -1);

  Stack* toStack(int *offset = NULL);
  bool loadStack(const Stack *stack, int threshold = 0);

  ZPoint getCenter() const;
  double getRadius() const;

  /*!
   * \brief Duplicate the object voxels across planes
   *
   * duplicateAcrossZ() first project all voxels into the first plane
   * (0-indexed) and then duplicate the voxels in every other plane within the
   * range of [1, \a depth - 1]. It does nothing if \a depth <= 0.
   *
   * \param depth Number of planes to copy.
   */
  void duplicateAcrossZ(int depth);

private:
  int m_conn;
  std::vector<int> m_voxelArray;
  mutable Object_3d m_objWrapper;
};

template <typename T>
std::vector<T> ZObject3d::toIndexArray(int width, int height, int depth)
{
  std::vector<T> indexArray(size(), -1);
  for (size_t i = 0; i < size(); i++) {
    indexArray[i] = Stack_Util_Offset(x(i), y(i), z(i), width, height, depth);
  }

  return indexArray;
}

template <typename T>
std::vector<T> ZObject3d::toIndexArray(int width, int height, int depth,
                                       int offsetX, int offsetY, int offsetZ)
{
  std::vector<T> indexArray(size(), -1);
  for (size_t i = 0; i < size(); i++) {
    indexArray[i] = Stack_Util_Offset(x(i) + offsetX, y(i) + offsetY,
                                      z(i) + offsetZ, width, height, depth);
  }

  return indexArray;
}
#endif // ZOBJECT3D_H
