#include "zpointarray.h"
#include <fstream>
#include <iostream>

ZPointArray::ZPointArray()
{
}

void ZPointArray::translate(const ZPoint &pt)
{
  for (iterator iter = begin(); iter != end(); ++iter) {
    *iter += pt;
  }
}

void ZPointArray::exportSwcFile(const std::string &filePath, double radius) const
{
  std::ofstream stream(filePath.c_str());

  for (size_t i = 0; i < size(); ++i) {
    int parentId = -1;
    const ZPoint &pt = (*this)[i];
    stream << i + 1 << " 2 " << pt.x() << " " << pt.y() << " " << pt.z()
           << " " << radius << " " << parentId << std::endl;
  }

  stream.close();
}

void ZPointArray::exportTxtFile(const std::string &filePath) const
{
    std::ofstream stream(filePath.c_str());

    for (size_t i = 0; i < size(); ++i) {
        const ZPoint &pt = (*this)[i];
        stream << pt.x() << " " << pt.y() << " " << pt.z() << std::endl;
    }

    stream.close();
}

void ZPointArray::print() const
{
  for (size_t i = 0; i < size(); ++i) {
    const ZPoint &pt = (*this)[i];
    std::cout << pt.toString() << std::endl;
  }
}

void ZPointArray::scale(double sx, double sy, double sz)
{
  if (sx != 1.0 || sy != 1.0 || sz != 1.0) {
    for (iterator iter = begin(); iter != end(); ++iter) {
      ZPoint &pt = *iter;
      pt.setX(pt.x() * sx);
      pt.setY(pt.y() * sy);
      pt.setZ(pt.z() * sz);
    }
  }
}
