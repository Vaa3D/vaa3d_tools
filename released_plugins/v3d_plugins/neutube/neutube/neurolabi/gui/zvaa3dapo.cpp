#include "zvaa3dapo.h"

#include <sstream>

using namespace std;

ZVaa3dApo::ZVaa3dApo() : m_id(0), m_orderInfo(1), m_maxIntensity(255),
  m_intensity(100), m_sdev(0), m_mass(10)
{
}

string ZVaa3dApo::toString()
{
  ostringstream stream;

  stream << m_id << "," << m_orderInfo << ","
         << m_name << "," << m_comment << ","
         << z() << "," << x() << "," << y()
         << "," << m_maxIntensity << "," << m_intensity
         << "," << m_sdev << "," << volSize() << "," << m_mass << ","
         << ","
         << static_cast<int>(m_color[0]) << ","
         << static_cast<int>(m_color[1]) << ","
         << static_cast<int>(m_color[2]) << ",";

  return stream.str();
}
