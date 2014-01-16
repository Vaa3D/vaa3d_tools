#ifndef ZCOLORMAP_H
#define ZCOLORMAP_H

#define GLM_SWIZZLE
#include "zglmutils.h"
#include <vector>
#include <QColor>
#include <limits>
#include "zparameter.h"
#if defined(_WIN32) || defined(_WIN64)
#include "stdint.h"
#else
#include <stdint.h>
#include <algorithm>
#endif


class QMainWindow;

class ZColorMapKey
{
public:
  ZColorMapKey(double i, const glm::col4& color);
  ZColorMapKey(double i, const glm::col4& colorL, const glm::col4& colorR);
  ZColorMapKey(double i, const glm::vec4& color);
  ZColorMapKey(double i, const glm::vec4& colorL, const glm::vec4& colorR);
  ZColorMapKey(double i, const QColor& color);
  ZColorMapKey(double i, const QColor& colorL, const QColor& colorR);
  bool operator==(const ZColorMapKey& key) const;
  bool operator!=(const ZColorMapKey& key) const;
  bool operator<(const ZColorMapKey& key) const;

  void setColorL(const glm::col4& color);
  void setColorL(const glm::ivec4& color);
  void setColorL(const glm::vec4& color);
  void setColorL(const QColor& color);
  inline glm::col4 getColorL() const {return m_colorL;}
  QColor getQColorL() const;

  void setColorR(const glm::col4& color);
  void setColorR(const glm::ivec4& color);
  void setColorR(const glm::vec4& color);
  void setColorR(const QColor& color);
  inline glm::col4 getColorR() const {return m_colorR;}
  QColor getQColorR() const;

  inline bool isSplit() const {return m_split;}
  void setSplit(bool split, bool useLeft=true);

  void setFloatAlphaR(double a);
  void setFloatAlphaL(double a);
  void setAlphaR(uint8_t a);
  void setAlphaL(uint8_t a);
  double getFloatAlphaR() const;
  double getFloatAlphaL() const;
  uint8_t getAlphaR() const;
  uint8_t getAlphaL() const;

  inline double getIntensity() const {return m_intensity;}
  inline void setIntensity(double i) {m_intensity = i;}

  ZColorMapKey* clone() const;

private:
  double m_intensity;
  glm::col4 m_colorL;
  glm::col4 m_colorR;
  bool m_split;
};

// class for colormap. A ColorMap class should always has at least two keys
class ZColorMap : public QObject
{
  Q_OBJECT
public:
  template<class ForwardIterator>
  ZColorMap(const ForwardIterator first, const ForwardIterator last, const glm::col4 &minColor = glm::col4(0,0,0,255),
            const glm::col4 &maxColor = glm::col4(255,255,255,255), QObject* parent = 0);

  ZColorMap(double min = 0.0, double max = 255.0, const glm::col4 &minColor = glm::col4(0,0,0,255),
            const glm::col4 &maxColor = glm::col4(255,255,255,255), QObject *parent = 0);

  template<class ForwardIterator>
  ZColorMap(const ForwardIterator first, const ForwardIterator last, const glm::vec4 &minColor = glm::vec4(0.f,0.f,0.f,1.f),
            const glm::vec4 &maxColor = glm::vec4(1.f,1.f,1.f,1.f), QObject *parent = 0);

  ZColorMap(double min, double max, const glm::vec4 &minColor = glm::vec4(0.f,0.f,0.f,1.f),
            const glm::vec4 &maxColor = glm::vec4(1.f,1.f,1.f,1.f), QObject *parent = 0);

  template<class ForwardIterator>
  ZColorMap(const ForwardIterator first, const ForwardIterator last, const QColor &minColor = QColor(0,0,0,255),
            const QColor &maxColor = QColor(255,255,255,255), QObject *parent = 0);

  ZColorMap(double min, double max, const QColor &minColor, const QColor &maxColor = QColor(0,0,0,255), QObject *parent = 0);

  ZColorMap(const ZColorMap& cm);

  void swap(ZColorMap& other);
  ZColorMap& operator =(ZColorMap other);


  void reset(double min = 0.0, double max = 255.0, const glm::col4 &minColor = glm::col4(0,0,0,255),
              const glm::col4 &maxColor = glm::col4(255,255,255,255));
  template<class ForwardIterator>
  void reset(const ForwardIterator first, const ForwardIterator last, const glm::col4 &minColor = glm::col4(0,0,0,255),
            const glm::col4 &maxColor = glm::col4(255,255,255,255));
  template<class ForwardIterator>
  void reset(const ForwardIterator first, const ForwardIterator last, const glm::vec4 &minColor = glm::vec4(0.f,0.f,0.f,1.f),
            const glm::vec4 &maxColor = glm::vec4(1.f,1.f,1.f,1.f));
  void reset(double min, double max, const glm::vec4 &minColor = glm::vec4(0.f,0.f,0.f,1.f),
            const glm::vec4 &maxColor = glm::vec4(1.f,1.f,1.f,1.f));
  template<class ForwardIterator>
  void reset(const ForwardIterator first, const ForwardIterator last, const QColor &minColor = QColor(0,0,0,255),
            const QColor &maxColor = QColor(255,255,255,255));
  void reset(double min, double max, const QColor &minColor = QColor(0,0,0,255),
            const QColor &maxColor = QColor(255,255,255,255));

  bool operator==(const ZColorMap& cm) const;
  bool operator !=(const ZColorMap& cm) const;

  double getDomainMin() const;
  double getDomainMax() const;
  glm::dvec2 getDomain() const { return glm::dvec2(getDomainMin(), getDomainMax()); }
  virtual bool isValidDomainMin(double min) const;
  virtual bool isValidDomainMax(double max) const;
  // set new domain range might fail, check the validality use isValidDomain*** function
  bool setDomainMin(double min, bool rescaleKeys = false);
  bool setDomainMax(double max, bool rescaleKeys = false);
  void setDomain(double min, double max, bool rescaleKeys = false);
  void setDomain(glm::dvec2 domain, bool rescaleKeys = false);
  glm::col4 getMappedColor(double i) const;
  glm::col4 getMappedColorBGRA(double i) const;
  glm::vec4 getMappedFColor(double i) const;
  QColor getMappedQColor(double i) const;
  glm::col4 getKeyColorL(size_t index) const;
  glm::vec4 getKeyFColorL(size_t index) const;
  QColor getKeyQColorL(size_t index) const;
  glm::col4 getKeyColorR(size_t index) const;
  glm::vec4 getKeyFColorR(size_t index) const;
  QColor getKeyQColorR(size_t index) const;
  void setKeyColorL(size_t index, const glm::col4 &color);
  void setKeyColorR(size_t index, const glm::col4 &color);
  void setKeyColorL(size_t index, const glm::vec4 &color);
  void setKeyColorR(size_t index, const glm::vec4 &color);
  void setKeyColorL(size_t index, const QColor &color);
  void setKeyColorR(size_t index, const QColor &color);
  double getKeyFloatAlphaL(size_t index) const;
  uint8_t getKeyAlphaL(size_t index) const;
  double getKeyFloatAlphaR(size_t index) const;
  uint8_t getKeyAlphaR(size_t index) const;
  void setKeyAlphaL(size_t index, uint8_t a);
  void setKeyAlphaR(size_t index, uint8_t a);
  void setKeyFloatAlphaL(size_t index, double a);
  void setKeyFloatAlphaR(size_t index, double a);
  double getKeyIntensity(size_t index) const;
  // note: don't call this function in a loop because this function might
  // change the order of keys and invalidate iterator
  void setKeyIntensity(size_t index, double intensity);
  bool isKeySelected(size_t index) const;
  void setKeySelected(size_t index, bool v = true);
  void deselectAllKeys();
  std::vector<size_t> getSelectedKeyIndexes() const;
  bool isKeySplit(size_t index) const;
  void setKeySplit(size_t index, bool v = true, bool useLeft=true);
  // fraction range [0.0 1.0]
  glm::col4 getFractionMappedColor(double fraction) const;
  glm::vec4 getFractionMappedFColor(double fraction) const;
  QColor getFractionMappedQColor(double fraction) const;
  inline size_t getNumKeys() const {return m_keys.size();}
  inline ZColorMapKey& getKey(size_t index) {return m_keys.at(index).first;}
  inline bool hasDataRange() const {return m_hasDataRange;}
  inline double getDataMin() const {return m_dataMin;}
  inline double getDataMax() const {return m_dataMax;}
  inline void setDataRange(double min, double max) {m_hasDataRange = true; m_dataMin = min; m_dataMax = max;}
  inline void removeDataRange() {m_hasDataRange = false;}
  bool setKey(size_t index, const ZColorMapKey &key, bool select = false);   //might change domain
  inline bool setKeys(const std::vector<ZColorMapKey> &keys);  //might change domain
  ZColorMapKey &addKey(const ZColorMapKey &key, bool select = false);
  void addKeyAtIntensity(double intensity, bool select = false);
  void addKeyAtIntensity(double intensity, const glm::col4 &color, bool select = false);
  void addKeyAtIntensity(double intensity, uint8_t alpha, bool select = false);
  void addKeyAtIntensity(double intensity, double alpha, bool select = false);
  void addKeyAtFraction(double fraction, const glm::col4 &color, bool select = false);
  void addKeyAtFraction(double fraction, bool select = false);
  void addKeyAtFraction(double fraction, uint8_t alpha, bool select = false);
  void addKeyAtFraction(double fraction, double alpha, bool select = false);
  void updateKeys();
  bool removeDuplicatedKeys();
  bool removeSelectedKeys();
  void removeKey(const ZColorMapKey &key);
  void removeKey(size_t index);
  inline bool isEmpty() const {return m_keys.empty();}

signals:
  void changed();

protected:
  inline void clearKeys() {m_keys.clear();}
  virtual bool equalTo(const ZColorMap& cm) const;

  std::vector<std::pair<ZColorMapKey, bool> > m_keys;
  typedef std::vector<std::pair<ZColorMapKey, bool> >::iterator KeyIterType;
  bool m_hasDataRange;
  double m_dataMin;
  double m_dataMax;
};

template<class ForwardIterator>
ZColorMap::ZColorMap(const ForwardIterator first, const ForwardIterator last, const glm::col4 &minColor, const glm::col4 &maxColor, QObject *parent)
  : QObject(parent), m_hasDataRange(true)
{
  m_dataMin = *(std::min_element(first, last));
  m_dataMax = *(std::max_element(first, last));
  m_dataMax = std::max(m_dataMax, m_dataMin + std::numeric_limits<double>::epsilon());
  addKey(ZColorMapKey(m_dataMin, minColor));
  addKey(ZColorMapKey(m_dataMax, maxColor));
}

template<class ForwardIterator>
ZColorMap::ZColorMap(const ForwardIterator first, const ForwardIterator last, const glm::vec4 &minColor, const glm::vec4 &maxColor, QObject *parent)
  : QObject(parent), m_hasDataRange(true)
{
  m_dataMin = *(std::min_element(first, last));
  m_dataMax = *(std::max_element(first, last));
  m_dataMax = std::max(m_dataMax, m_dataMin + std::numeric_limits<double>::epsilon());
  addKey(ZColorMapKey(m_dataMin, minColor));
  addKey(ZColorMapKey(m_dataMax, maxColor));
}

template<class ForwardIterator>
ZColorMap::ZColorMap(const ForwardIterator first, const ForwardIterator last, const QColor &minColor, const QColor &maxColor, QObject *parent)
  : QObject(parent), m_hasDataRange(true)
{
  m_dataMin = *(std::min_element(first, last));
  m_dataMax = *(std::max_element(first, last));
  m_dataMax = std::max(m_dataMax, m_dataMin + std::numeric_limits<double>::epsilon());
  addKey(ZColorMapKey(m_dataMin, minColor));
  addKey(ZColorMapKey(m_dataMax, maxColor));
}

template<class ForwardIterator>
void ZColorMap::reset(const ForwardIterator first, const ForwardIterator last, const glm::col4 &minColor, const glm::col4 &maxColor)
{
  m_hasDataRange = true;
  blockSignals(true);
  clearKeys();
  m_dataMin = *(std::min_element(first, last));
  m_dataMax = *(std::max_element(first, last));
  m_dataMax = std::max(m_dataMax, m_dataMin + std::numeric_limits<double>::epsilon());
  addKey(ZColorMapKey(m_dataMin, minColor));
  addKey(ZColorMapKey(m_dataMax, maxColor));
  blockSignals(false);
  emit changed();
}

template<class ForwardIterator>
void ZColorMap::reset(const ForwardIterator first, const ForwardIterator last, const glm::vec4 &minColor, const glm::vec4 &maxColor)
{
  m_hasDataRange = true;
  blockSignals(true);
  clearKeys();
  m_dataMin = *(std::min_element(first, last));
  m_dataMax = *(std::max_element(first, last));
  m_dataMax = std::max(m_dataMax, m_dataMin + std::numeric_limits<double>::epsilon());
  addKey(ZColorMapKey(m_dataMin, minColor));
  addKey(ZColorMapKey(m_dataMax, maxColor));
  blockSignals(false);
  emit changed();
}

template<class ForwardIterator>
void ZColorMap::reset(const ForwardIterator first, const ForwardIterator last, const QColor &minColor, const QColor &maxColor)
{
  m_hasDataRange = true;
  blockSignals(true);
  clearKeys();
  m_dataMin = *(std::min_element(first, last));
  m_dataMax = *(std::max_element(first, last));
  m_dataMax = std::max(m_dataMax, m_dataMin + std::numeric_limits<double>::epsilon());
  addKey(ZColorMapKey(m_dataMin, minColor));
  addKey(ZColorMapKey(m_dataMax, maxColor));
  blockSignals(false);
  emit changed();
}


// ZColormapParameter class
class ZColorMapParameter : public ZSingleValueParameter<ZColorMap>
{
public:
  ZColorMapParameter(const QString &name, const ZColorMap& cm);

  template<class ForwardIterator>
  ZColorMapParameter(const QString& name, const ForwardIterator first, const ForwardIterator last, const glm::col4 &minColor = glm::col4(0,0,0,255),
                     const glm::col4 &maxColor = glm::col4(255,255,255,255));

  ZColorMapParameter(const QString& name, double min = 0.0, double max = 255.0, const glm::col4 &minColor = glm::col4(0,0,0,255),
                     const glm::col4 &maxColor = glm::col4(255,255,255,255));

  template<class ForwardIterator>
  ZColorMapParameter(const QString& name, const ForwardIterator first, const ForwardIterator last, const glm::vec4 &minColor = glm::vec4(0.f,0.f,0.f,1.f),
                     const glm::vec4 &maxColor = glm::vec4(1.f,1.f,1.f,1.f));

  ZColorMapParameter(const QString& name, double min, double max, const glm::vec4 &minColor = glm::vec4(0.f,0.f,0.f,1.f),
                     const glm::vec4 &maxColor = glm::vec4(1.f,1.f,1.f,1.f));

  template<class ForwardIterator>
  ZColorMapParameter(const QString& name, const ForwardIterator first, const ForwardIterator last, const QColor &minColor = QColor(0,0,0,255),
                     const QColor &maxColor = QColor(255,255,255,255));

  ZColorMapParameter(const QString& name, double min, double max, const QColor &minColor = QColor(0,0,0,255),
                     const QColor &maxColor = QColor(255,255,255,255));

protected:
  virtual QWidget* actualCreateWidget(QWidget *parent);
};


template<class ForwardIterator>
ZColorMapParameter::ZColorMapParameter(const QString& name, const ForwardIterator first, const ForwardIterator last, const glm::col4 &minColor, const glm::col4 &maxColor)
  : ZSingleValueParameter<ZColorMap>(name)
{
  m_value.reset(first, last, minColor, maxColor);
  connect(&m_value, SIGNAL(changed()), this, SIGNAL(valueChanged()));
}

template<class ForwardIterator>
ZColorMapParameter::ZColorMapParameter(const QString& name, const ForwardIterator first, const ForwardIterator last, const glm::vec4 &minColor, const glm::vec4 &maxColor)
  : ZSingleValueParameter<ZColorMap>(name)
{
  m_value.reset(first, last, minColor, maxColor);
  connect(&m_value, SIGNAL(changed()), this, SIGNAL(valueChanged()));
}

template<class ForwardIterator>
ZColorMapParameter::ZColorMapParameter(const QString& name, const ForwardIterator first, const ForwardIterator last, const QColor &minColor, const QColor &maxColor)
  : ZSingleValueParameter<ZColorMap>(name)
{
  m_value.reset(first, last, minColor, maxColor);
  connect(&m_value, SIGNAL(changed()), this, SIGNAL(valueChanged()));
}



#endif // ZCOLORMAP_H
