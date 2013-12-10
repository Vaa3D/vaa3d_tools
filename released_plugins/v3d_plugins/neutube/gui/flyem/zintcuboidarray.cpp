#include "zintcuboidarray.h"

#include <iostream>
#include <math.h>
#include "zstring.h"
#include "zcuboid.h"
#include "zswctree.h"
#include "swctreenode.h"

using namespace std;

FlyEm::ZIntCuboidArray::ZIntCuboidArray()
{
}

void FlyEm::ZIntCuboidArray::append(
    int x, int y, int z, int width, int height, int depth)
{


  Cuboid_I cuboid;
  Cuboid_I_Set_S(&cuboid, x, y, z, width, height, depth);
  push_back(cuboid);
#ifdef _DEBUG_2
  std::cout << depth << std::endl;
  Print_Cuboid_I(&cuboid);
#endif
}

int FlyEm::ZIntCuboidArray::hitTest(double x, double y, double z)
{
  int ix = floor(x);
  int iy = floor(y);
  int iz = floor(z);

  for (size_t i = 0; i < size(); ++i) {
    if (Cuboid_I_Hit(&((*this)[i]), ix, iy, iz)) {
#ifdef _DEBUG_2
      std::cout << ix << ' ' << iy << ' ' << iz << std::endl;
      Print_Cuboid_I(&((*this)[i]));
#endif
      return i;
    }
  }

  return -1;
}

int FlyEm::ZIntCuboidArray::hitInternalTest(double x, double y, double z)
{
  int ix = floor(x);
  int iy = floor(y);
  int iz = floor(z);

  for (size_t i = 0; i < size(); ++i) {
    if (Cuboid_I_Hit_Internal(&((*this)[i]), ix, iy, iz)) {
#ifdef _DEBUG_2
      std::cout << ix << ' ' << iy << ' ' << iz << std::endl;
      Print_Cuboid_I(&((*this)[i]));
#endif
      return i;
    }
  }

  return -1;
}

void FlyEm::ZIntCuboidArray::loadSubstackList(const std::string filePath)
{
  clear();

  ZString str;
  FILE *fp = fopen(filePath.c_str(), "r");
  if (fp != NULL) {
    while (str.readLine(fp)) {
      std::vector<int> valueArray = str.toIntegerArray();
      if (valueArray.size() == 7) {
        append(valueArray[1], valueArray[3], valueArray[5],
            abs(valueArray[2]) - valueArray[1] + 1,
            abs(valueArray[4]) - valueArray[3] + 1,
            abs(valueArray[6]) - valueArray[5] + 1);
      }
    }
  } else {
    cerr << "Cannot open " << filePath << endl;
  }
}

void FlyEm::ZIntCuboidArray::translate(int x, int y, int z)
{
  for (ZIntCuboidArray::iterator iter = begin(); iter != end(); ++iter) {
    iter->cb[0] += x;
    iter->ce[0] += x;
    iter->cb[1] += y;
    iter->ce[1] += y;
    iter->cb[2] += z;
    iter->ce[2] += z;
  }
}

void FlyEm::ZIntCuboidArray::rescale(double factor)
{
  for (ZIntCuboidArray::iterator iter = begin(); iter != end(); ++iter) {
    iter->cb[0] *= factor;
    iter->ce[0] *= factor;
    iter->cb[1] *= factor;
    iter->ce[1] *= factor;
    iter->cb[2] *= factor;
    iter->ce[2] *= factor;
  }
}

void FlyEm::ZIntCuboidArray::exportSwc(const string &filePath) const
{
  if (!empty()) {
    ZSwcTree *tree = new ZSwcTree;
    int index = 0;
    for (ZIntCuboidArray::const_iterator iter = begin(); iter != end();
         ++iter, ++index) {
      ZCuboid cuboid;
      cuboid.set(iter->cb[0], iter->cb[1], iter->cb[2], iter->ce[0], iter->ce[1],
          iter->ce[2]);
      ZSwcTree *subtree = ZSwcTree::createCuboidSwc(cuboid);
      subtree->setType(index);
      tree->merge(subtree, true);
    }

    tree->resortId();
    tree->save(filePath);

    delete tree;
  }
}

bool FlyEm::ZIntCuboidArray::isInvalid(const Cuboid_I &cuboid)
{
  return !Cuboid_I_Is_Valid(&cuboid);
}

void FlyEm::ZIntCuboidArray::removeInvalidCuboid()
{
  iterator newEnd = std::remove_if(begin(), end(), isInvalid);
  if (newEnd != end()) {
    resize(newEnd - begin());
  }
}

void FlyEm::ZIntCuboidArray::intersect(const Cuboid_I &cuboid)
{
  for (ZIntCuboidArray::iterator iter = begin(); iter != end(); ++iter) {
    Cuboid_I_Intersect(&(*iter), &cuboid, &(*iter));
  }

  removeInvalidCuboid();
}


Cuboid_I FlyEm::ZIntCuboidArray::getBoundBox() const
{
  Cuboid_I box;
  Cuboid_I_Set_S(&box, 0, 0, 0, 0, 0, 0);

#ifdef _DEBUG_2
  Print_Cuboid_I(&box);
#endif

  if (!empty()) {
    ZIntCuboidArray::const_iterator iter = begin();
    box = *iter;
    for (++iter; iter != end(); ++iter) {
      Cuboid_I_Union(&(*iter), &box, &box);
#ifdef _DEBUG_2
  Print_Cuboid_I(&box);
#endif
    }
  }

  return box;
}

FlyEm::ZIntCuboidArray FlyEm::ZIntCuboidArray::getFace() const
{
  FlyEm::ZIntCuboidArray face;

  for (ZIntCuboidArray::const_iterator iter = begin(); iter != end(); ++iter) {
    if (Cuboid_I_Is_Valid(&(*iter))) {
      for (int i = 0; i < 3; ++i) {
        Cuboid_I tmpFace = *iter;
        tmpFace.ce[i] = tmpFace.cb[i];
        face.push_back(tmpFace);
        tmpFace = *iter;
        tmpFace.cb[i] = tmpFace.ce[i];
        face.push_back(tmpFace);
      }
    }
  }

  return face;
}

FlyEm::ZIntCuboidArray FlyEm::ZIntCuboidArray::getInnerFace() const
{
  FlyEm::ZIntCuboidArray face;

  int index1 = 0;

  for (ZIntCuboidArray::const_iterator iter = begin(); iter != end();
       ++iter, ++index1) {
    Cuboid_I box1 = *iter;
    Cuboid_I_Expand_X(&box1, 1);
    Cuboid_I_Expand_Y(&box1, 1);
    Cuboid_I_Expand_Z(&box1, 1);

    int index2 = 0;
    for (ZIntCuboidArray::const_iterator iter2 = begin(); iter2 != end();
         ++iter2, ++index2) {
      if (index1 != index2) {
        Cuboid_I box2 = *iter2;
#ifdef _DEBUG_2
        std::cout << "Intersecting " << std::endl;
          Print_Cuboid_I(&box1);
          Print_Cuboid_I(&box2);
#endif
        Cuboid_I_Intersect(&box1, &box2, &box2);

        int width, height, depth;
        Cuboid_I_Size(&box2, &width, &height, &depth);
        if (width > 0 && height > 0 && depth > 0 &&
            (width > 1) + (height > 1) + (depth > 1) > 1) {
          face.push_back(box2);
#ifdef _DEBUG_2
          Print_Cuboid_I(&box2);
#endif
        }
      }
    }
  }

  return face;
}

void FlyEm::ZIntCuboidArray::print() const
{
  for (ZIntCuboidArray::const_iterator iter = begin(); iter != end();
       ++iter) {
    cout << "(" << iter->cb[0] << ", " << iter->cb[1] << ", " << iter->cb[2]
         << ") -> (" << iter->ce[0] << ", " << iter->ce[1] << ", "
         << iter->ce[2] << ")" << endl;
  }
}

size_t FlyEm::ZIntCuboidArray::getVolume() const
{
  size_t volume = 0;
  for (ZIntCuboidArray::const_iterator iter = begin(); iter != end();
       ++iter) {
    volume += Cuboid_I_Volume(&(*iter));
  }

  return volume;
}
