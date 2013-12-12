#include "zhistogram.h"
#include <iostream>
#include <cmath>
#include "tz_math.h"
#include "tz_darray.h"


using namespace std;

ZHistogram::ZHistogram(double start, double interval) :
  m_start(start), m_interval(interval)
{
}

int ZHistogram::getBinIndex(double x) const
{
  return floor((x - m_start) / m_interval);
}

void ZHistogram::increment(double x)
{
  addCount(x, 1.0);
}

void ZHistogram::addCount(double x, double dv)
{
  if (dv > 0.0) {
    int binIndex = getBinIndex(x);

    if (binIndex < 0) {
      m_start += m_interval * binIndex;
      vector<double> newCount(-binIndex, 0.0);
      newCount.insert(newCount.end(), m_count.begin(), m_count.end());
      binIndex = 0;
      m_count = newCount;
    } else if (binIndex >= (int) m_count.size()) {
      m_count.resize(binIndex + 1, 0.0);
    }

    m_count[binIndex] += dv;
  }
}

double ZHistogram::getCount(double x) const
{
  int binIndex = getBinIndex(x);
  if (binIndex < 0 || binIndex >= (int) m_count.size()) {
    return 0.0;
  }

  return m_count[binIndex];
}

double ZHistogram::getBinStart(int index) const
{
  return m_start + m_interval * index;
}

double ZHistogram::getBinEnd(int index) const
{
  return m_start + m_interval * (index + 1);
}

double ZHistogram::getCount(double x1, double x2) const
{
  if (x1 >= x2) {
    return 0.0;
  }

  int startIndex = getBinIndex(x1);
  int endIndex = getBinIndex(x2);

  if (startIndex < 0 && endIndex < 0) {
    return 0.0;
  }

  if (startIndex >=  (int) m_count.size() &&
      endIndex >= (int) m_count.size()) {
    return 0.0;
  }

  if (startIndex < 0) {
    startIndex = 0;
    x1 = getBinStart(startIndex);
  }

  if (endIndex >= (int) m_count.size()) {
    endIndex = m_count.size() - 1;
    x2 = getBinEnd(endIndex);
  }

  double count = 0.0;

  for (int i = startIndex; i <= endIndex; ++i) {
    count += m_count[i];
  }

  //Subtract partial bins
  double dx = x1 - getBinStart(startIndex);
  if (dx > 0.0) {
    count -= getDensity(x1) * dx;
  }

  dx = getBinEnd(endIndex) - x2;
  if (dx > 0.0) {
    count -= getDensity(x2) * dx;
  }

  if (count < 0.0) {
    count = 0.0;
  }

  return count;
}

double ZHistogram::getDensity(double x) const
{
  int binIndex = getBinIndex(x);
  if (binIndex < 0 || binIndex >= (int) m_count.size()) {
    return 0.0;
  }

  return m_count[binIndex] / m_interval;
}

void ZHistogram::print() const
{
  cout << "Histogram - start point: " << m_start << "; interval: " << m_interval
       << "; " << m_count.size() << " bins" << endl;
  double x0 = m_start;
  for (size_t i = 0; i < m_count.size(); ++i) {
    if (m_count[i] > 0) {
      cout << "  [" << x0 << "," << x0 + m_interval << "): " << m_count[i] << endl;
    }
    x0 += m_interval;
  }
}

void ZHistogram::clear()
{
  m_count.clear();
}

bool ZHistogram::isEmpty() const
{
  for (size_t i = 0; i < m_count.size(); ++i) {
    if (m_count[i] > 0.0) {
      return false;
    }
  }

  return true;
}

void ZHistogram::normalize()
{
  double maxCount = getMaxCount();
  if (maxCount > 0.0) {
    for (size_t i = 0; i < m_count.size(); ++i) {
      m_count[i] /= maxCount;
    }
  }
}

double ZHistogram::getMaxCount(int *index) const
{
  int maxIndex = 0;
  double maxCount = 0.0;
  for (size_t i = 0; i < m_count.size(); ++i) {
    if (m_count[i] > maxCount) {
      maxCount = m_count[i];
      maxIndex = i;
    }
  }

  if (index != NULL) {
    *index = maxIndex;
  }

  return maxCount;
}

double ZHistogram::getUpperBound() const
{
  return m_start + m_interval * m_count.size() + m_interval / 2.0;
}

double ZHistogram::getBinCenter(int binIndex) const
{
  return m_start + m_interval * (0.5 + binIndex);
}

void ZHistogram::rebin(double interval)
{
  if (interval > 0.0 && interval != m_interval) {
    if (!m_count.empty()) {
      double upperBound = getUpperBound();
      std::vector<double> count(std::ceil((upperBound - m_start) / interval));

      for (size_t i = 0; i < count.size(); ++i) {
        count[i] = getCount(m_start + interval * i,
                            m_start + interval * (i + 1));
      }

      m_count = count;
    }

    m_interval = interval;
  }
}


ZHistogram& ZHistogram::operator +=(const ZHistogram &hist)
{
  if (m_interval == hist.m_interval && m_start == hist.m_start) {
    if (hist.m_count.size() > m_count.size()) {
      m_count.resize(hist.m_count.size(), 0.0);
    }
    for (size_t i = 0; i < hist.m_count.size(); ++i) {
      m_count[i] += hist.m_count[i];
    }
  } else {
    int startIndex = getBinIndex(hist.m_start);
    int endIndex = getBinIndex(hist.getUpperBound());

    if (endIndex >= (int) m_count.size()) {
      m_count.resize(endIndex + 1, 0.0);
    }

    if (startIndex < 0) {
      vector<double> newCount(-startIndex, 0.0);
      newCount.insert(newCount.end(), m_count.begin(), m_count.end());
      m_count = newCount;
      m_start += startIndex * m_interval;
    }

    for (size_t i = 0; i < m_count.size(); ++i) {
      m_count[i] += hist.getCount(getBinStart(i), getBinEnd(i));
    }
  }

  return (*this);
}
