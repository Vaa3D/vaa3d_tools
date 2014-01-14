#ifndef ZOBJECT3DSCAN_H
#define ZOBJECT3DSCAN_H

#include <vector>
#include <string>
#include <set>
#include <map>
#include <utility>
#include "zqtheader.h"
#include "c_stack.h"
#include "zcuboid.h"
#include "zstackdrawable.h"
#include "tz_cuboid_i.h"
#include "zhistogram.h"
#include "zvoxel.h"

class ZObject3d;
class ZGraph;

class ZObject3dStripe {
public:
  ZObject3dStripe() : m_y(0), m_z(0), m_isCanonized(true) {
  }
  inline int getY() const { return m_y; }
  inline int getZ() const { return m_z; }
  int getMinX() const;
  int getMaxX() const;
  inline size_t getSize() const { return m_segmentArray.size() / 2; }
  inline int getSegmentNumber() const { return getSize(); }
  int getVoxelNumber() const;

  inline void setY(int y) { m_y = y; }
  inline void setZ(int z) { m_z = z; }

  void addSegment(int x1, int x2, bool canonizing = true);

  const int* getSegment(size_t index) const;
  int getSegmentStart(size_t index) const;
  int getSegmentEnd(size_t index) const;

  void write(FILE *fp) const;
  void read(FILE *fp);

  void drawStack(Stack *stack, int v, const int *offset = NULL) const;
  void drawStack(Stack *stack, uint8_t red, uint8_t green, uint8_t blue,
                 const int *offset = NULL) const;
  /*!
   * \brief Count the overlap area between an object and a stack
   *
   * \param stack Input stack. It's foreground is defined as any pixel with
   *        intensity > 0.
   * \param offset Offset of the object.
   * \return The number of voxels overlapped.
   */
  size_t countForegroundOverlap(Stack *stack, const int *offset = NULL) const;

  inline bool isEmpty() const { return m_segmentArray.empty(); }
  inline bool isCanonized() const { return isEmpty() || m_isCanonized; }

  void sort();
  void canonize();
  bool unify(const ZObject3dStripe &stripe, bool canonizing = true);

  void print() const;

  void downsample(int xintv);
  void downsampleMax(int xintv);

  void clearSegment();

  void translate(int dx, int dy, int dz);

  /*!
   * \brief Add z value
   *
   * Basically it is the same as translate(0, 0, \a dz);
   */
  void addZ(int dz);

  bool isCanonizedActually();

  /*!
   * \brief Test if two stripe are the same with respect to internal representation
   */
  bool equalsLiterally(const ZObject3dStripe &stripe) const;

  void dilate();

  inline void setCanonized(bool canonized) { m_isCanonized = canonized; }

private:
  std::vector<int> m_segmentArray;
  int m_y;
  int m_z;
  bool m_isCanonized;
};

//Scan-line representation of a 3D object
class ZObject3dScan : public ZStackDrawable
{
public:
  ZObject3dScan();
  virtual ~ZObject3dScan();

  enum EComponent {
    STRIPE_INDEX_MAP, INDEX_SEGMENT_MAP, ACCUMULATED_STRIPE_NUMBER,
    ALL_COMPONENT
  };

  bool isDeprecated(EComponent comp) const;
  void deprecate(EComponent comp);
  void deprecateDependent(EComponent comp);

  void clear();

  ZObject3d* toObject3d() const;

  bool isEmpty() const;
  size_t getStripeNumber() const;
  size_t getVoxelNumber() const;

  /*!
   * \brief Get voxel number at a certain slice
   * \param z The slice position.
   */
  size_t getVoxelNumber(int z) const;

  /*!
   * \brief Get the voxel number on each slice
   * \return The ith element is the #voxel at slice i.
   */
  std::vector<size_t> getSlicewiseVoxelNumber() const;

  ZObject3dStripe getStripe(size_t index) const;

  /*
  const int* getFirstStripe() const;
  static int getY(const int *stripe);
  static int getZ(const int *stripe);
  //return number of scan lines
  static int getStripeSize(const int *stripe);
  static int* getSegment(const int *stripe);
  static void setStripeSize(int *stripe, int size);
  const int* getLastStripe() const;
  int* getLastStripe();
*/
  void addStripe(int z, int y, bool canonizing = true);
  void addSegment(int x1, int x2, bool canonizing = true);
  void addSegment(int z, int y, int x1, int x2, bool canonizing = true);
  void addStripe(const ZObject3dStripe &stripe, bool canonizing = true);

  //Turn a binary stack into scanlines
  void loadStack(const Stack *stack);

  void print() const;

  void save(const std::string &filePath) const;
  bool load(const std::string &filePath);

  /*!
   * \brief Import a dvid object
   *
   * byte     Payload descriptor:
                   Bit 0 (LSB) - 8-bit grayscale
                   Bit 1 - 16-bit grayscale
                   Bit 2 - 16-bit normal
                   ...
        uint8    Number of dimensions
        uint8    Dimension of run (typically 0 = X)
        byte     Reserved (to be used later)
        uint32    # Voxels [TODO.  0 for now]
        uint32    # Spans
        Repeating unit of:
            int32   Coordinate of run start (dimension 0)
            int32   Coordinate of run start (dimension 1)
            int32   Coordinate of run start (dimension 2)
              ...
            int32   Length of run
            bytes   Optional payload dependent on first byte descriptor
   */
  bool importDvidObject(const std::string &filePath);

  /*!
   * \brief Import object from a byte array
   */
  bool importDvidObject(const char *byteArray, size_t byteNumber);

  template<class T>
  int scanArray(const T *array, int x, int y, int z, int width);

  void drawStack(Stack *stack, int v, const int *offset = NULL) const;
  void drawStack(Stack *stack, uint8_t red, uint8_t green, uint8_t blue,
                 const int *offset = NULL) const;
  void labelStack(Stack *stack, int startLabel, const int *offset = NULL);

  /*!
   * \brief Count overlap between the object and a stack
   *
   * Count the overlap between the object and \a stack. Any voxel in the object
   * has postive value in \a stack is counted.
   *
   * \param offset The offset of the object to \a stack if it is not NULL.
   * \return Number of voxels in the overlapping region. It returns 0 if \a stack
   * is NULL.
   */
  size_t countForegroundOverlap(Stack *stack, const int *offset = NULL);

  //Sort the stripes in the ascending order
  // s1 < s2 if
  //   z(s1) < z(s2) or
  //   z(s1) == z(s2) and y(s1) < y(s2)
  void sort();
  void canonize();
  void unify(const ZObject3dScan &obj);
  void concat(const ZObject3dScan &obj);

  void downsample(int xintv, int yintv, int zintv);
  void downsampleMax(int xintv, int yintv, int zintv);

  Stack* toStack(int *offset = NULL) const;
  ZCuboid getBoundBox() const;
  void getBoundBox(Cuboid_I *box) const;

  template<class T>
  static std::map<int, ZObject3dScan*>* extractAllObject(
      const T *array, int width, int height, int depth, int startPlane,
      std::map<int, ZObject3dScan*> *bodySet);

  ZGraph* buildConnectionGraph();

  const std::vector<size_t> &getStripeNumberAccumulation() const;

  const std::map<std::pair<int, int>, size_t>& getStripeMap() const;

  std::vector<size_t> getConnectedObjectSize();
  std::vector<ZObject3dScan> getConnectedComponent();

  inline bool isCanonized() const { return isEmpty() || m_isCanonized; }
  inline void setCanonized(bool canonized) { m_isCanonized = canonized; }

  const std::map<size_t, std::pair<size_t, size_t> >&
  getIndexSegmentMap() const;
  bool getSegment(size_t index, int *z, int *y, int *x1, int *x2);
  size_t getSegmentNumber();

  void translate(int dx, int dy, int dz);

  /*!
   * \brief Add z value
   *
   * Basically it is the same as translate(0, 0, \a dz);
   */
  void addZ(int dz);

  bool isCanonizedActually();

  void duplicateAcrossZ(int depth);

  ZObject3dScan getSlice(int z) const;
  ZObject3dScan getSlice(int minZ, int maxZ) const;

  virtual void display(QPainter &painter, int z = 0, Display_Style option = NORMAL)
  const;
  virtual const std::string& className() const;

  void dilate();

  ZPoint getCentroid() const;
  /*!
   * \brief Get the single voxel representing the object
   *
   * \return A voxel on the object. It returns (-1, -1, -1) if the object is
   *         empty.
   */
  ZVoxel getMarker() const;

  ZHistogram getRadialHistogram(int z) const;

  ZObject3dScan makeZProjection() const;
  ZObject3dScan makeZProjection(int minZ, int maxZ);

  /*!
   * \brief Get minimal Z
   *
   * \return The minimal Z value of the object. If the object is empty,
   *         it returns 0.
   */
  int getMinZ() const;

  /*!
   * \brief Get maximal Z
   *
   * \return The maximal Z value of the object. If the object is empty,
   *         it returns 0.
   */
  int getMaxZ() const;

  /*!
   * \brief Test if two objects are the same with respect to internal representation
   */
  bool equalsLiterally(const ZObject3dScan &obj) const;

  /*!
   * \brief Get the complement of the object
   *
   * The complement object is defined as the background of the bound box painted
   * with the original object.
   */
  ZObject3dScan getComplementObject();

  /*!
   * \brief Find all holes as a single object.
   * \return An object composed of all holes of the original object.
   */
  ZObject3dScan findHoleObject();

  /*!
   * \brief Find all holes as a single object
   * \return An array of objects, each representing a hole.
   */
  std::vector<ZObject3dScan> findHoleObjectArray();

  /*!
   * \brief Fill the holes of the object
   */
  void fillHole();

private:
  std::vector<ZObject3dStripe> m_stripeArray;
  /*
  int m_stripeNumber;
  std::vector<int> m_sripeSize;
  */

  mutable std::vector<size_t> m_accNumberArray;
  mutable std::map<std::pair<int, int>, size_t> m_stripeMap;
  mutable std::map<size_t, std::pair<size_t, size_t> > m_indexSegmentMap;
  bool m_isCanonized;
  //mutable int *m_lastStripe;
};


template<class T>
int ZObject3dScan::scanArray(const T *array, int x, int y, int z, int width)
{
  if (array == NULL) {
    return 0;
  }

  if (x < 0 || x >= width) {
    return 0;
  }

  int length = 0;
  T v = array[x];

  if (isEmpty()) {
    addStripe(z, y);
  } else {
    if (m_stripeArray.back().getY() != y || m_stripeArray.back().getZ() != z) {
      addStripe(z, y);
    }
  }

  while (array[x + length] == v) {
    ++length;
    if (x + length >= width) {
      break;
    }
  }

  addSegment(x, x + length - 1, false);

  return length;
}

template<class T>
std::map<int, ZObject3dScan*>* ZObject3dScan::extractAllObject(
    const T *array, int width, int height, int depth, int startPlane,
    std::map<int, ZObject3dScan*> *bodySet)
{
  if (bodySet == NULL) {
    bodySet = new std::map<int, ZObject3dScan*>;
  }

  ZObject3dScan *obj = NULL;
  for (int z = 0; z < depth; ++z) {
    for (int y = 0; y < height; ++y) {
      int x = 0;
      while (x < width) {
        int v = array[x];
        std::map<int, ZObject3dScan*>::iterator iter = bodySet->find(v);
        if (iter == bodySet->end()) {
          obj = new ZObject3dScan;
          //(*bodySet)[v] = obj;
          bodySet->insert(std::map<int, ZObject3dScan*>::value_type(v, obj));
        } else {
          obj = iter->second;
        }
        int length = obj->scanArray(array, x, y, z + startPlane, width);

        x += length;
      }
      array += width;
    }
  }

  return bodySet;
}

#endif // ZOBJECT3DSCAN_H
