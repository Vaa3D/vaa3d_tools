#include "zflyemneuronrange.h"
#include <cmath>
#include <fstream>
#include "zerror.h"
#include "zswctree.h"
#include "zflyemneuronaxis.h"
#include "zstring.h"

ZFlyEmNeuronRange::ZFlyEmNeuronRange()
{
}

bool ZFlyEmNeuronRange::isEmpty() const
{
  return m_planeRange.empty();
}

double ZFlyEmNeuronRange::getMinZ() const
{
  if (m_planeRange.empty()) {
    return 0.0;
  }

  return m_planeRange.begin()->first;
}

double ZFlyEmNeuronRange::getMaxZ() const
{
  if (m_planeRange.empty()) {
    return 0.0;
  }

  return m_planeRange.rbegin()->first;
}


void ZFlyEmNeuronRange::setPlaneRange(double z, double r)
{
  m_planeRange[z] = r;
}

double ZFlyEmNeuronRange::getRadius(double z) const
{
  double r = -1.0;

  if (z >= getMinZ() && z <= getMaxZ()) {
    double z1 = z;
    double z2 = z;

    for (RangeMap::const_iterator iter = m_planeRange.begin();
         iter != m_planeRange.end(); ++iter) {
      z2 = iter->first;
      if (z2 >= z) {
        break;
      }
      z1 = iter->first;
    }

    if (z == z1) {
      if (m_planeRange.count(z1) > 0) {
        r = m_planeRange.at(z1);
      }
    } else {
      PROCESS_WARNING(z1 == z2, "Unexpected identical plane", return 0.0);
      double ratio = (z - z1) / (z2 - z1);

      r = m_planeRange.at(z1) * (1 - ratio) + m_planeRange.at(z2) * ratio;
    }
  }

  return r;
}

bool ZFlyEmNeuronRange::contains(double x, double y, double z) const
{
  return sqrt(x * x + y * y) <= getRadius(z);
}

bool ZFlyEmNeuronRange::contains(const ZPoint &pt) const
{
  return contains(pt.x(), pt.y(), pt.z());
}

void ZFlyEmNeuronRange::transform(double offset, double scale)
{
  for (RangeMap::iterator iter = m_planeRange.begin();
       iter != m_planeRange.end(); ++iter) {
    iter->second = iter->second * scale + offset;
  }
}

void ZFlyEmNeuronRange::unify(const ZFlyEmNeuronRange &range)
{
  for (RangeMap::const_iterator iter = range.m_planeRange.begin();
       iter != range.m_planeRange.end(); ++iter) {
    double buddyZ = getZ(*iter);
    if (getRadius(buddyZ) < getRadius(*iter)) {
      setPlaneRange(buddyZ, getRadius(*iter));
    }
  }

  for (RangeMap::iterator iter = m_planeRange.begin();
       iter != m_planeRange.end(); ++iter) {
    double z = getZ(*iter);
    double buddyZ = range.getRadius(z);
    if (getRadius(*iter) < buddyZ) {
      setRadius(*iter, buddyZ);
    }
  }
}

bool ZFlyEmNeuronRange::exportCsvFile(const std::string &filePath) const
{
  std::ofstream stream;
  stream.open(filePath.c_str());
  if (!stream.is_open()) {
    return false;
  }

  for (RangeMap::const_iterator iter = m_planeRange.begin();
       iter != m_planeRange.end(); ++iter) {
    stream << iter->first << "," << iter->second << std::endl;
  }

  stream.close();

  return true;
}

bool ZFlyEmNeuronRange::importCsvFile(const std::string &filePath)
{
  m_planeRange.clear();

  FILE *fp = fopen(filePath.c_str(), "r");
  if (fp != NULL) {
    ZString line;
    while (line.readLine(fp)) {
      std::vector<double> value = line.toDoubleArray();
      if (value.size() == 2) {
        m_planeRange[value[0]] = value[1];
      }
    }

    return true;
  }

  return false;
}
