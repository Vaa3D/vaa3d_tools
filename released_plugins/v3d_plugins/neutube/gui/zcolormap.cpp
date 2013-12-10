#include "zcolormap.h"
#include <limits>
#include <QWidget>
#include "QsLog.h"

#include "zcolormapwidgetwitheditorwindow.h"

ZColorMapKey::ZColorMapKey(double i, const glm::col4 &color)
  : m_intensity(i)
  , m_colorL(color)
  , m_colorR(color)
  , m_split(false)
{
}

ZColorMapKey::ZColorMapKey(double i, const glm::col4 &colorL, const glm::col4 &colorR)
  : m_intensity(i)
  , m_colorL(colorL)
  , m_colorR(colorR)
  , m_split(true)
{
}

ZColorMapKey::ZColorMapKey(double i, const glm::vec4 &color)
  : m_intensity(i)
  , m_colorL(glm::col4(color * 255.f))
  , m_colorR(glm::col4(color * 255.f))
  , m_split(false)
{
}

ZColorMapKey::ZColorMapKey(double i, const glm::vec4 &colorL, const glm::vec4 &colorR)
  : m_intensity(i)
  , m_colorL(glm::col4(colorL * 255.f))
  , m_colorR(glm::col4(colorR * 255.f))
  , m_split(true)
{
}

ZColorMapKey::ZColorMapKey(double i, const QColor &color)
  : m_intensity(i)
  , m_colorL(color.red(), color.green(), color.blue(), color.alpha())
  , m_colorR(color.red(), color.green(), color.blue(), color.alpha())
  , m_split(false)
{
}

ZColorMapKey::ZColorMapKey(double i, const QColor &colorL, const QColor &colorR)
  : m_intensity(i)
  , m_colorL(colorL.red(), colorL.green(), colorL.blue(), colorL.alpha())
  , m_colorR(colorR.red(), colorR.green(), colorR.blue(), colorR.alpha())
  , m_split(true)
{
}

bool ZColorMapKey::operator ==(const ZColorMapKey &key) const
{
  return m_intensity == key.getIntensity() && m_colorL == key.getColorL()
      && m_colorR == key.getColorR() && m_split == key.isSplit();
}

bool ZColorMapKey::operator !=(const ZColorMapKey &key) const
{
  return !(*this == key);
}

bool ZColorMapKey::operator <(const ZColorMapKey &key) const
{
  return m_intensity < key.m_intensity;
}

void ZColorMapKey::setColorL(const glm::col4 &color)
{
  m_colorL = color;
  if (!m_split) m_colorR = m_colorL;
}

void ZColorMapKey::setColorL(const glm::ivec4 &color)
{
  m_colorL = glm::col4(color);
  if (!m_split) m_colorR = m_colorL;
}

void ZColorMapKey::setColorL(const glm::vec4 &color)
{
  m_colorL = glm::col4(color * 255.f);
  if (!m_split) m_colorR = m_colorL;
}

void ZColorMapKey::setColorL(const QColor &color)
{
  m_colorL = glm::col4(color.red(), color.green(), color.blue(), color.alpha());
  if (!m_split) m_colorR = m_colorL;
}

QColor ZColorMapKey::getQColorL() const
{
  return QColor(m_colorL.r, m_colorL.g, m_colorL.b, m_colorL.a);
}

void ZColorMapKey::setColorR(const glm::col4 &color)
{
  m_colorR = color;
  if (!m_split) m_colorL = m_colorR;
}

void ZColorMapKey::setColorR(const glm::ivec4 &color)
{
  m_colorR = glm::col4(color);
  if (!m_split) m_colorL = m_colorR;
}

void ZColorMapKey::setColorR(const glm::vec4 &color)
{
  m_colorR = glm::col4(color * 255.f);
  if (!m_split) m_colorL = m_colorR;
}

void ZColorMapKey::setColorR(const QColor &color)
{
  m_colorR = glm::col4(color.red(), color.green(), color.blue(), color.alpha());
  if (!m_split) m_colorL = m_colorR;
}

QColor ZColorMapKey::getQColorR() const
{
  return QColor(m_colorR.r, m_colorR.g, m_colorR.b, m_colorR.a);
}

void ZColorMapKey::setSplit(bool split, bool useLeft)
{
  if (m_split == split)
    return;
  if (!split) {
    if (useLeft)
      m_colorR = m_colorL;
    else
      m_colorL = m_colorR;
  }
  m_split = split;
}

void ZColorMapKey::setFloatAlphaR(double a)
{
  m_colorR.a = static_cast<uint8_t>(a*255.);
  if (!m_split)
    m_colorL.a = m_colorR.a;
}

void ZColorMapKey::setFloatAlphaL(double a)
{
  m_colorL.a = static_cast<uint8_t>(a*255.);
  if (!m_split)
    m_colorR.a = m_colorL.a;
}

void ZColorMapKey::setAlphaR(uint8_t a)
{
  m_colorR.a = a;
  if (!m_split)
    m_colorL.a = m_colorR.a;
}

void ZColorMapKey::setAlphaL(uint8_t a)
{
  m_colorL.a = a;
  if (!m_split)
    m_colorR.a = m_colorL.a;
}

double ZColorMapKey::getFloatAlphaR() const
{
  return m_colorR.a / 255.;
}

double ZColorMapKey::getFloatAlphaL() const
{
  return m_colorL.a / 255.;
}

uint8_t ZColorMapKey::getAlphaR() const
{
  return m_colorR.a;
}

uint8_t ZColorMapKey::getAlphaL() const
{
  return m_colorL.a;
}

ZColorMapKey *ZColorMapKey::clone() const
{
  if (!m_split)
    return new ZColorMapKey(m_intensity, m_colorL);
  else
    return new ZColorMapKey(m_intensity, m_colorL, m_colorR);
}

ZColorMap::ZColorMap(double min, double max, const glm::col4 &minColor,
                     const glm::col4 &maxColor, QObject *parent)
  : QObject(parent), m_hasDataRange(false), m_dataMin(0), m_dataMax(0)
{
  addKey(ZColorMapKey(min, minColor));
  max = std::max(max, min + std::numeric_limits<double>::epsilon());
  addKey(ZColorMapKey(max, maxColor));
}

ZColorMap::ZColorMap(double min, double max, const glm::vec4 &minColor,
                     const glm::vec4 &maxColor, QObject *parent)
  : QObject(parent), m_hasDataRange(false), m_dataMin(0), m_dataMax(0)
{
  addKey(ZColorMapKey(min, minColor));
  max = std::max(max, min + std::numeric_limits<double>::epsilon());
  addKey(ZColorMapKey(max, maxColor));
}

ZColorMap::ZColorMap(double min, double max, const QColor &minColor, const QColor &maxColor, QObject *parent)
  : QObject(parent), m_hasDataRange(false), m_dataMin(0), m_dataMax(0)
{
  addKey(ZColorMapKey(min, minColor));
  max = std::max(max, min + std::numeric_limits<double>::epsilon());
  addKey(ZColorMapKey(max, maxColor));
}

ZColorMap::ZColorMap(const ZColorMap &cm)
  : QObject(cm.parent()), m_hasDataRange(cm.m_hasDataRange), m_dataMin(cm.m_dataMin), m_dataMax(cm.m_dataMax)
{
  m_keys = cm.m_keys;
}

void ZColorMap::swap(ZColorMap &other)
{
  m_keys.swap(other.m_keys);
  std::swap(m_hasDataRange, other.m_hasDataRange);
  std::swap(m_dataMin, other.m_dataMin);
  std::swap(m_dataMax, other.m_dataMax);
}

ZColorMap& ZColorMap::operator =(ZColorMap other)
{
  swap(other);
  return *this;
}

void ZColorMap::reset(double min, double max, const glm::col4 &minColor, const glm::col4 &maxColor)
{
  m_hasDataRange = false;
  blockSignals(true);
  clearKeys();
  addKey(ZColorMapKey(min, minColor));
  max = std::max(max, min + std::numeric_limits<double>::epsilon());
  addKey(ZColorMapKey(max, maxColor));
  m_dataMin = 0;
  m_dataMax = 0;
  blockSignals(false);
  emit changed();
}

void ZColorMap::reset(double min, double max, const glm::vec4 &minColor, const glm::vec4 &maxColor)
{
  m_hasDataRange = false;
  blockSignals(true);
  clearKeys();
  addKey(ZColorMapKey(min, minColor));
  max = std::max(max, min + std::numeric_limits<double>::epsilon());
  addKey(ZColorMapKey(max, maxColor));
  m_dataMin = 0;
  m_dataMax = 0;
  blockSignals(false);
  emit changed();
}

void ZColorMap::reset(double min, double max, const QColor &minColor, const QColor &maxColor)
{
  m_hasDataRange = false;
  blockSignals(true);
  clearKeys();
  addKey(ZColorMapKey(min, minColor));
  max = std::max(max, min + std::numeric_limits<double>::epsilon());
  addKey(ZColorMapKey(max, maxColor));
  m_dataMin = 0;
  m_dataMax = 0;
  blockSignals(false);
  emit changed();
}

bool ZColorMap::operator ==(const ZColorMap &cm) const
{
  return equalTo(cm);
}

bool ZColorMap::operator !=(const ZColorMap &cm) const
{
  return !(*this == cm);
}

double ZColorMap::getDomainMin() const
{
  return m_keys[0].first.getIntensity();
}

double ZColorMap::getDomainMax() const
{
  return m_keys[m_keys.size()-1].first.getIntensity();
}

bool ZColorMap::isValidDomainMin(double min) const
{
  if ((!m_hasDataRange && min < getDomainMax()) || (m_hasDataRange && min <= m_dataMin))
    return true;
  else
    return false;
}

bool ZColorMap::isValidDomainMax(double max) const
{
  if ((!m_hasDataRange && max > getDomainMin()) || (m_hasDataRange && max >= m_dataMax))
    return true;
  else
    return false;
}

bool ZColorMap::setDomainMin(double min, bool rescaleKeys)
{
  if (min == getDomainMin())
    return true;
  if (isValidDomainMin(min)) {
    blockSignals(true);
    if (rescaleKeys) {
      double prevDistToMax = getDomainMax() - getDomainMin();
      double distToMax = getDomainMax() - min;
      double scale = distToMax/prevDistToMax;
      double dmax = getDomainMax();
      std::vector<std::pair<ZColorMapKey, bool> > newKeys;
      for (size_t i=0; i<m_keys.size(); i++) {
        double inten = getKeyIntensity(i);
        double newInten = dmax - (dmax-inten)*scale;
        if (isKeySplit(i))
          newKeys.push_back(std::make_pair(ZColorMapKey(newInten, getKeyColorL(i), getKeyColorR(i)), m_keys[i].second));
        else
          newKeys.push_back(std::make_pair(ZColorMapKey(newInten, getKeyColorL(i)), m_keys[i].second));
      }
      m_keys = newKeys;
    } else {
      glm::col4 col = getMappedColor(min);
      size_t startIdx = m_keys.size();
      size_t endIdx = 0;
      for (size_t i=0; i<m_keys.size(); i++) {
        if (getKeyIntensity(i) <= min) {
          startIdx = std::min(startIdx, i);
          endIdx = std::max(endIdx, i);
        } else
          break;
      }
      if (startIdx < m_keys.size())
        m_keys.erase(m_keys.begin()+startIdx, m_keys.begin()+endIdx+1);

      addKey(ZColorMapKey(min, col));
    }
    blockSignals(false);
    emit changed();
    return true;
  } else
    return false;
}

bool ZColorMap::setDomainMax(double max, bool rescaleKeys)
{
  if (max == getDomainMax())
    return true;
  if (isValidDomainMax(max)) {
    blockSignals(true);
    if (rescaleKeys) {
      double prevDistToMin = getDomainMax() - getDomainMin();
      double distToMin = max - getDomainMin();
      double scale = distToMin/prevDistToMin;
      double dmin = getDomainMin();
      std::vector<std::pair<ZColorMapKey, bool> > newKeys;
      for (size_t i=0; i<m_keys.size(); i++) {
        double inten = getKeyIntensity(i);
        double newInten = dmin + (inten-dmin)*scale;
        if (isKeySplit(i))
          newKeys.push_back(std::make_pair(ZColorMapKey(newInten, getKeyColorL(i), getKeyColorR(i)), m_keys[i].second));
        else
          newKeys.push_back(std::make_pair(ZColorMapKey(newInten, getKeyColorL(i)), m_keys[i].second));
      }
      m_keys = newKeys;
    } else {
      glm::col4 col = getMappedColor(max);
      size_t startIdx = m_keys.size();
      size_t endIdx = 0;
      for (int i=(int)(m_keys.size())-1; i>=0; i--) {
        if (getKeyIntensity(i) >= max) {
          startIdx = std::min(startIdx, (size_t)i);
          endIdx = std::max(endIdx, (size_t)i);
        } else
          break;
      }
      if (startIdx < m_keys.size())
        m_keys.erase(m_keys.begin()+startIdx, m_keys.begin()+endIdx+1);

      addKey(ZColorMapKey(max, col));
    }
    blockSignals(false);
    emit changed();
    return true;
  } else
    return false;
}

void ZColorMap::setDomain(double min, double max, bool rescaleKeys)
{
  if (min >= max) {
    LERROR() << "wrong input";
    return;
  }
  if (min == getDomainMin() && max == getDomainMax())
    return;
  if (min < getDomainMax()) {
    bool ok = false;
    blockSignals(true);
    ok &= setDomainMin(min, rescaleKeys);
    ok &= setDomainMax(max, rescaleKeys);
    blockSignals(false);
    if (ok)
      emit changed();
  } else {
    bool ok = false;
    blockSignals(true);
    ok &= setDomainMax(max, rescaleKeys);
    ok &= setDomainMin(min, rescaleKeys);
    blockSignals(false);
    if (ok)
      emit changed();
  }
}

void ZColorMap::setDomain(glm::dvec2 domain, bool rescaleKeys)
{
  setDomain(domain.x, domain.y, rescaleKeys);
}

glm::col4 ZColorMap::getMappedColor(double i) const
{
  if (m_keys.empty())
    return glm::col4(0, 0, 0, 0);

  // iterate through all keys until we get to the correct position
  std::vector<std::pair<ZColorMapKey,bool> >::const_iterator keyIt = m_keys.begin();

  while ((keyIt != m_keys.end()) && (i > (*keyIt).first.getIntensity()))
    keyIt++;

  if (keyIt == m_keys.begin())
    return m_keys[0].first.getColorL();
  else if (keyIt == m_keys.end())
    return (*(keyIt-1)).first.getColorR();
  else{
    // calculate the value weighted by the destination to the next left and right key
    ZColorMapKey leftKey = (*(keyIt-1)).first;
    ZColorMapKey rightKey = keyIt->first;
    double fraction = (i - leftKey.getIntensity()) / (rightKey.getIntensity() - leftKey.getIntensity());
    glm::col4 leftDest = leftKey.getColorR();
    glm::col4 rightDest = rightKey.getColorL();
    glm::col4 result = leftDest;
    result.r += static_cast<uint8_t>((rightDest.r - leftDest.r) * fraction);
    result.g += static_cast<uint8_t>((rightDest.g - leftDest.g) * fraction);
    result.b += static_cast<uint8_t>((rightDest.b - leftDest.b) * fraction);
    result.a += static_cast<uint8_t>((rightDest.a - leftDest.a) * fraction);
    return result;
  }
}

glm::col4 ZColorMap::getMappedColorBGRA(double i) const
{
  glm::col4 result = getMappedColor(i);
  std::swap(result.r, result.b);
  return result;
}

glm::vec4 ZColorMap::getMappedFColor(double i) const
{
  glm::col4 col = getMappedColor(i);
  return glm::vec4(col)/255.f;
}

QColor ZColorMap::getMappedQColor(double i) const
{
  glm::col4 col = getMappedColor(i);
  return QColor(col.r, col.g, col.b, col.a);
}

glm::col4 ZColorMap::getKeyColorL(size_t index) const
{
  return m_keys[index].first.getColorL();
}

glm::vec4 ZColorMap::getKeyFColorL(size_t index) const
{
  return glm::vec4(m_keys[index].first.getColorL())/255.f;
}

QColor ZColorMap::getKeyQColorL(size_t index) const
{
  return m_keys[index].first.getQColorL();
}

glm::col4 ZColorMap::getKeyColorR(size_t index) const
{
  return m_keys[index].first.getColorR();
}

glm::vec4 ZColorMap::getKeyFColorR(size_t index) const
{
  return glm::vec4(m_keys[index].first.getColorR())/255.f;
}

QColor ZColorMap::getKeyQColorR(size_t index) const
{
  return m_keys[index].first.getQColorR();
}

void ZColorMap::setKeyColorL(size_t index, const glm::col4 &color)
{
  m_keys[index].first.setColorL(color);
  emit changed();
}

void ZColorMap::setKeyColorR(size_t index, const glm::col4 &color)
{
  m_keys[index].first.setColorR(color);
  emit changed();
}

void ZColorMap::setKeyColorL(size_t index, const glm::vec4 &color)
{
  m_keys[index].first.setColorL(color);
  emit changed();
}

void ZColorMap::setKeyColorR(size_t index, const glm::vec4 &color)
{
  m_keys[index].first.setColorR(color);
  emit changed();
}

void ZColorMap::setKeyColorL(size_t index, const QColor &color)
{
  m_keys[index].first.setColorL(color);
  emit changed();
}

void ZColorMap::setKeyColorR(size_t index, const QColor &color)
{
  m_keys[index].first.setColorR(color);
  emit changed();
}

double ZColorMap::getKeyFloatAlphaL(size_t index) const
{
  return m_keys[index].first.getFloatAlphaL();
}

uint8_t ZColorMap::getKeyAlphaL(size_t index) const
{
  return m_keys[index].first.getAlphaL();
}

double ZColorMap::getKeyFloatAlphaR(size_t index) const
{
  return m_keys[index].first.getFloatAlphaR();
}

uint8_t ZColorMap::getKeyAlphaR(size_t index) const
{
  return m_keys[index].first.getAlphaR();
}

void ZColorMap::setKeyAlphaL(size_t index, uint8_t a)
{
  m_keys[index].first.setAlphaL(a);
  emit changed();
}

void ZColorMap::setKeyAlphaR(size_t index, uint8_t a)
{
  m_keys[index].first.setAlphaR(a);
  emit changed();
}

void ZColorMap::setKeyFloatAlphaL(size_t index, double a)
{
  m_keys[index].first.setFloatAlphaL(a);
  emit changed();
}

void ZColorMap::setKeyFloatAlphaR(size_t index, double a)
{
  m_keys[index].first.setFloatAlphaR(a);
  emit changed();
}

double ZColorMap::getKeyIntensity(size_t index) const
{
  return m_keys[index].first.getIntensity();
}

void ZColorMap::setKeyIntensity(size_t index, double intensity)
{
  std::pair<ZColorMapKey, bool> newPair = m_keys[index];
  newPair.first.setIntensity(intensity);
  blockSignals(true);
  removeKey(index);
  addKey(newPair.first, newPair.second);
  blockSignals(false);
  emit changed();
}

bool ZColorMap::isKeySelected(size_t index) const
{
  return m_keys[index].second;
}

void ZColorMap::setKeySelected(size_t index, bool v)
{
  if (isKeySelected(index) != v) {
    m_keys[index].second = v;
    emit changed();
  }
}

void ZColorMap::deselectAllKeys()
{
  bool change = false;
  for (size_t i=0; i<m_keys.size(); i++) {
    if (m_keys[i].second) {
      change = true;
      m_keys[i].second = false;
    }
  }
  if (change)
    emit changed();
}

std::vector<size_t> ZColorMap::getSelectedKeyIndexes() const
{
  std::vector<size_t> all;
  for (size_t i=0; i<m_keys.size(); i++)
  {
    if (m_keys[i].second)
      all.push_back(i);
  }
  return all;
}

bool ZColorMap::isKeySplit(size_t index) const
{
  return m_keys[index].first.isSplit();
}

void ZColorMap::setKeySplit(size_t index, bool v, bool useLeft)
{
  if (isKeySplit(index) != v) {
    m_keys[index].first.setSplit(v, useLeft);
    emit changed();
  }
}

glm::col4 ZColorMap::getFractionMappedColor(double fraction) const
{
  double i = getDomainMin() + fraction * (getDomainMax() - getDomainMin());
  return getMappedColor(i);
}

glm::vec4 ZColorMap::getFractionMappedFColor(double fraction) const
{
  double i = getDomainMin() + fraction * (getDomainMax() - getDomainMin());
  return getMappedFColor(i);
}

QColor ZColorMap::getFractionMappedQColor(double fraction) const
{
  double i = getDomainMin() + fraction * (getDomainMax() - getDomainMin());
  return getMappedQColor(i);
}

bool ZColorMap::setKey(size_t index, const ZColorMapKey &key, bool select)
{
  if (index < m_keys.size())
  {
    blockSignals(true);
    removeKey(index);
    addKey(key, select);
    blockSignals(false);
    emit changed();
    return true;
  }
  return false;
}

bool ZColorMap::setKeys(const std::vector<ZColorMapKey> &keys)
{
  if (keys.size() < 2)
    return false;
  blockSignals(true);
  clearKeys();
  for (size_t i=0; i<keys.size(); i++)
    addKey(keys[i]);
  blockSignals(false);
  emit changed();
  return true;
}

ZColorMapKey& ZColorMap::addKey(const ZColorMapKey &key, bool select)
{
  if (m_keys.empty()) {
    m_keys.push_back(std::make_pair(key, select));
    emit changed();
    return m_keys.back().first;
  }
  KeyIterType keyIt = m_keys.begin();
  // Forward to the correct position
  while ((keyIt != m_keys.end()) && (key.getIntensity() > (*keyIt).first.getIntensity()))
    keyIt++;
  if (keyIt == m_keys.end()) {
    m_keys.push_back(std::make_pair(key, select));
    emit changed();
    return m_keys.back().first;
  } else {
    KeyIterType iter = m_keys.insert(keyIt, std::make_pair(key, select));
    emit changed();
    return (*iter).first;
  }
}

void ZColorMap::addKeyAtIntensity(double intensity, bool select)
{
  addKey(ZColorMapKey(intensity, getMappedColor(intensity)), select);
}

void ZColorMap::addKeyAtIntensity(double intensity, const glm::col4 &color, bool select)
{
  addKey(ZColorMapKey(intensity, color), select);
}

void ZColorMap::addKeyAtIntensity(double intensity, uint8_t alpha, bool select)
{
  glm::col4 col = getMappedColor(intensity);
  col.a = alpha;
  addKey(ZColorMapKey(intensity, col), select);
}

void ZColorMap::addKeyAtIntensity(double intensity, double alpha, bool select)
{
  glm::vec4 col = getMappedFColor(intensity);
  col.a = alpha;
  addKey(ZColorMapKey(intensity, col), select);
}

void ZColorMap::addKeyAtFraction(double fraction, const glm::col4 &color, bool select)
{
  double intensity = getDomainMin() + fraction * (getDomainMax() - getDomainMin());
  addKey(ZColorMapKey(intensity, color), select);
}

void ZColorMap::addKeyAtFraction(double fraction, bool select)
{
  glm::col4 col = getFractionMappedColor(fraction);
  double intensity = getDomainMin() + fraction * (getDomainMax() - getDomainMin());
  addKey(ZColorMapKey(intensity, col), select);
}

void ZColorMap::addKeyAtFraction(double fraction, uint8_t alpha, bool select)
{
  glm::col4 col = getFractionMappedColor(fraction);
  col.a = alpha;
  double intensity = getDomainMin() + fraction * (getDomainMax() - getDomainMin());
  addKey(ZColorMapKey(intensity, col), select);
}

void ZColorMap::addKeyAtFraction(double fraction, double alpha, bool select)
{
  glm::vec4 col = getFractionMappedFColor(fraction);
  col.a = alpha;
  double intensity = getDomainMin() + fraction * (getDomainMax() - getDomainMin());
  addKey(ZColorMapKey(intensity, col), select);
}

namespace {

bool KeyIntensityEqual(const std::pair<ZColorMapKey, bool> &key1,
                       const std::pair<ZColorMapKey, bool> &key2)
{
  return key1.first.getIntensity() == key2.first.getIntensity();
}

bool KeyIsSelected(const std::pair<ZColorMapKey, bool> &key)
{
  return key.second;
}

}

bool ZColorMap::removeDuplicatedKeys()
{
  size_t sizeBefore = m_keys.size();
  m_keys.erase(std::unique(m_keys.begin(), m_keys.end(), KeyIntensityEqual), m_keys.end());
  if (m_keys.size()!=sizeBefore)
    emit changed();
  return m_keys.size()!=sizeBefore;
}

bool ZColorMap::removeSelectedKeys()
{
  size_t sizeBefore = m_keys.size();
  m_keys.erase(std::remove_if(m_keys.begin(), m_keys.end(), KeyIsSelected), m_keys.end());
  if (m_keys.size()!=sizeBefore)
    emit changed();
  return m_keys.size()!=sizeBefore;
}

void ZColorMap::updateKeys()
{
  std::sort(m_keys.begin(), m_keys.end());
}

void ZColorMap::removeKey(const ZColorMapKey &key)
{
  m_keys.erase(std::remove(m_keys.begin(), m_keys.end(), std::make_pair(key, false)), m_keys.end());
  m_keys.erase(std::remove(m_keys.begin(), m_keys.end(), std::make_pair(key, true)), m_keys.end());
  emit changed();
}

void ZColorMap::removeKey(size_t index)
{
  m_keys.erase(m_keys.begin()+index);
  emit changed();
}

ZColorMapParameter::ZColorMapParameter(const QString &name, const ZColorMap &cm)
  : ZSingleValueParameter<ZColorMap>(name, cm)
{
  connect(&m_value, SIGNAL(changed()), this, SIGNAL(valueChanged()));
}

ZColorMapParameter::ZColorMapParameter(const QString &name, double min, double max, const glm::col4 &minColor, const glm::col4 &maxColor)
  : ZSingleValueParameter<ZColorMap>(name)
{
  m_value.reset(min, max, minColor, maxColor);
  connect(&m_value, SIGNAL(changed()), this, SIGNAL(valueChanged()));
}

ZColorMapParameter::ZColorMapParameter(const QString &name, double min, double max, const glm::vec4 &minColor, const glm::vec4 &maxColor)
  : ZSingleValueParameter<ZColorMap>(name)
{
  m_value.reset(min, max, minColor, maxColor);
  connect(&m_value, SIGNAL(changed()), this, SIGNAL(valueChanged()));
}

ZColorMapParameter::ZColorMapParameter(const QString &name, double min, double max, const QColor &minColor, const QColor &maxColor)
  : ZSingleValueParameter<ZColorMap>(name)
{
  m_value.reset(min, max, minColor, maxColor);
  connect(&m_value, SIGNAL(changed()), this, SIGNAL(valueChanged()));
}

QWidget *ZColorMapParameter::actualCreateWidget(QWidget *parent)
{
  return new ZColorMapWidgetWithEditorWindow(this, m_mainWin, parent);
}

bool ZColorMap::equalTo(const ZColorMap &cm) const
{
  return m_keys == cm.m_keys && m_hasDataRange == cm.m_hasDataRange && m_dataMin == cm.m_dataMin && m_dataMax == cm.m_dataMax;
}

