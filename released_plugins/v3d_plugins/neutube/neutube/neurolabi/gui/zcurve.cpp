/*
 * zcurve.cpp
 *
 *  Created on: May 28, 2009
 *      Author: zhaot
 */

#include <limits>
#include "zcurve.h"

ZCurve::ZCurve()
{
  m_yRange[0] = -std::numeric_limits<double>::infinity();
  m_yRange[1] = std::numeric_limits<double>::infinity();
}

double ZCurve::minX() const
{
  double result = 0.0;

  if (m_data.size() > 0) {
    result = m_data.at(0).x();
  }

  for (int i = 0; i < m_data.size(); i++) {
    if (m_data.at(i).x() < result) {
      result = m_data.at(i).x();
    }
  }

  return result;
}

double ZCurve::maxX() const
{
  double result = 0.0;

  if (m_data.size() > 0) {
    result = m_data.at(0).x();
  }

  for (int i = 0; i < m_data.size(); i++) {
    if (m_data.at(i).x() > result) {
      result = m_data.at(i).x();
    }
  }

  return result;
}

double ZCurve::minY() const
{
  double result = 0.0;

  if (m_data.size() > 0) {
    result = m_data.at(0).y();
  }

  for (int i = 0; i < m_data.size(); i++) {
    if (m_data.at(i).y() < result) {
      result = m_data.at(i).y();
    }
  }

  return result;
}

double ZCurve::maxY() const
{
  double result = 0.0;

  if (m_data.size() > 0) {
    result = m_data.at(0).y();
  }

  for (int i = 0; i < m_data.size(); i++) {
    if (m_data.at(i).y() > result) {
      result = m_data.at(i).y();
    }
  }

  return result;
}

void ZCurve::setYRange(double lower, double upper)
{
  m_yRange[0] = lower;
  m_yRange[1] = upper;
}
