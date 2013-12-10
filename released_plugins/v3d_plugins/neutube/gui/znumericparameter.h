#ifndef ZNUMERICPARAMETER_H
#define ZNUMERICPARAMETER_H

#include "zparameter.h"
#include <limits>
#include "zglmutils.h"
#include <QList>
#include <QString>
#include "QsLog.h"

template<class T>
class ZNumericParameter : public ZSingleValueParameter<T>
{
public:
  ZNumericParameter(const QString &name, T value, T min, T max, QObject *parent = NULL)
    : ZSingleValueParameter<T>(name, value, parent), m_min(min), m_max(max)
  {
    if (std::numeric_limits<T>::is_integer) {
      m_step = 1;
      m_decimal = 0;
    } else {
      m_step = static_cast<T>(.01);
      m_decimal = 3;
    }
    m_tracking = true;
    if (this->m_value < m_min)
      this->m_value = m_min;
    if (this->m_value > m_max)
      this->m_value = m_max;
  }

  inline void setSingleStep(T v) { m_step = v; }

  inline void setTracking(bool t) { m_tracking = t; }

  inline void setDecimal(int d) { m_decimal = d; }

  void setRange(T min, T max)
  {
    if (min != m_min || max != m_max) {
      m_min = min;
      m_max = max;
      changeRange();
    }
  }

protected:
  virtual void makeValid(T& value) const
  {
    if (value < m_min)
      value = m_min;
    if (value > m_max)
      value = m_max;
  }
  // inherite this to notify associated widgets about the range change (emit a signal)
  virtual void changeRange() {}

protected:
  T m_min;
  T m_max;
  T m_step;
  bool m_tracking;
  int m_decimal;
};

class ZIntParameter : public ZNumericParameter<int>
{
  Q_OBJECT
public:
  ZIntParameter(const QString &name, int value, int min, int max, QObject *parent = NULL);

signals:
  void valueChanged(int);
  void rangeChanged(int min, int max);
public slots:
  void setValue(int v);

protected:
  virtual void beforeChange(int &value);
  virtual QWidget* actualCreateWidget(QWidget *parent);
  virtual void changeRange();
};

class ZDoubleParameter : public ZNumericParameter<double>
{
  Q_OBJECT
public:
  ZDoubleParameter(const QString &name, double value, double min, double max, QObject *parent = NULL);

signals:
  void valueChanged(double);
  void rangeChanged(double min, double max);
public slots:
  void setValue(double v);

protected:
  virtual void beforeChange(double &value);
  virtual QWidget* actualCreateWidget(QWidget *parent);
  virtual void changeRange();
};

class ZFloatParameter : public ZNumericParameter<float>
{
  Q_OBJECT
public:
  ZFloatParameter(const QString &name, float value, float min, float max, QObject *parent = NULL);

signals:
  void valueChanged(double);
  void rangeChanged(double min, double max);
public slots:
  void setValue(double v);

protected:
  virtual void beforeChange(float &value);
  virtual QWidget* actualCreateWidget(QWidget *parent);
  virtual void changeRange();
};

//---------------------------------------------------------------------------------------------------

template<class T>
class ZNumericVectorParameter : public ZSingleValueParameter<T>
{
public:
  ZNumericVectorParameter(const QString &name, T value, T min, T max, QObject *parent = NULL)
    : ZSingleValueParameter<T>(name, value, parent), m_min(min), m_max(max), m_widgetOrientation(Qt::Vertical)
  {
    if (std::numeric_limits<typename T::value_type>::is_integer) {
      m_step = 1;
      m_decimal = 0;
    } else {
      m_step = static_cast<typename T::value_type>(.01);
      m_decimal = 3;
    }
    m_tracking = true;
    for (size_t i=0; i<this->m_value.length(); i++) {
      if (this->m_value[i] < m_min[i])
        this->m_value[i] = m_min[i];
      if (this->m_value[i] > m_max[i])
        this->m_value[i] = m_max[i];
      m_nameOfEachValue.push_back("");
    }
  }

  inline void setSingleStep(typename T::value_type v) { m_step = v; }

  inline void setTracking(bool t) { m_tracking = t; }

  inline void setDecimal(int d) { m_decimal = d; }

  // default is vertical
  inline void setWidgetOrientation(Qt::Orientation o) { m_widgetOrientation = o; }

  void setNameForEachValue(const QList<QString> &other)
  {
    if (static_cast<size_t>(other.size()) >= this->m_value.length())
      m_nameOfEachValue = other;
    else
      LERROR() << "input names do not have enough member";
  }

  // for some widget style all subwidgets be bound by a groupbox
  // default is empty
  inline void setGroupBoxName(const QString &name) { m_groupBoxName = name; }

protected:
  virtual void makeValid(T& value) const
  {
    for (size_t i=0; i<this->m_value.length(); i++) {
      if (value[i] < m_min[i])
        value[i] = m_min[i];
      if (value[i] > m_max[i])
        value[i] = m_max[i];
    }
  }

protected:
  T m_min;
  T m_max;
  typename T::value_type m_step;
  bool m_tracking;
  int m_decimal;
  Qt::Orientation m_widgetOrientation;
  QList<QString> m_nameOfEachValue;   // default is empty string for each value
  QString m_groupBoxName;
};

class ZVec2Parameter : public ZNumericVectorParameter<glm::vec2>
{
  Q_OBJECT
public:
  ZVec2Parameter(const QString &name, glm::vec2 value, glm::vec2 min = glm::vec2(0.f),
                 glm::vec2 max = glm::vec2(1.f), QObject *parent = NULL);

signals:
  void value1Changed(double);
  void value2Changed(double);
public slots:
  void setValue1(double v);
  void setValue2(double v);

protected:
  virtual void beforeChange(glm::vec2 &value);
  virtual QWidget* actualCreateWidget(QWidget *parent);
};

class ZVec3Parameter : public ZNumericVectorParameter<glm::vec3>
{
  Q_OBJECT
public:
  ZVec3Parameter(const QString &name, glm::vec3 value, glm::vec3 min = glm::vec3(0.f),
                 glm::vec3 max = glm::vec3(1.f), QObject *parent = NULL);

signals:
  void value1Changed(double);
  void value2Changed(double);
  void value3Changed(double);
public slots:
  void setValue1(double v);
  void setValue2(double v);
  void setValue3(double v);

protected:
  virtual void beforeChange(glm::vec3 &value);
  virtual QWidget* actualCreateWidget(QWidget *parent);
};

class ZVec4Parameter : public ZNumericVectorParameter<glm::vec4>
{
  Q_OBJECT
public:
  ZVec4Parameter(const QString &name, glm::vec4 value, glm::vec4 min = glm::vec4(0.f),
                 glm::vec4 max = glm::vec4(1.f), QObject *parent = NULL);

signals:
  void value1Changed(double);
  void value2Changed(double);
  void value3Changed(double);
  void value4Changed(double);
public slots:
  void setValue1(double v);
  void setValue2(double v);
  void setValue3(double v);
  void setValue4(double v);

protected:
  virtual void beforeChange(glm::vec4 &value);
  virtual QWidget* actualCreateWidget(QWidget *parent);
};

class ZDVec2Parameter : public ZNumericVectorParameter<glm::dvec2>
{
  Q_OBJECT
public:
  ZDVec2Parameter(const QString &name, glm::dvec2 value, glm::dvec2 min = glm::dvec2(0.f),
                 glm::dvec2 max = glm::dvec2(1.f), QObject *parent = NULL);

signals:
  void value1Changed(double);
  void value2Changed(double);
public slots:
  void setValue1(double v);
  void setValue2(double v);

protected:
  virtual void beforeChange(glm::dvec2 &value);
  virtual QWidget* actualCreateWidget(QWidget *parent);
};

class ZDVec3Parameter : public ZNumericVectorParameter<glm::dvec3>
{
  Q_OBJECT
public:
  ZDVec3Parameter(const QString &name, glm::dvec3 value, glm::dvec3 min = glm::dvec3(0.f),
                 glm::dvec3 max = glm::dvec3(1.f), QObject *parent = NULL);

signals:
  void value1Changed(double);
  void value2Changed(double);
  void value3Changed(double);
public slots:
  void setValue1(double v);
  void setValue2(double v);
  void setValue3(double v);

protected:
  virtual void beforeChange(glm::dvec3 &value);
  virtual QWidget* actualCreateWidget(QWidget *parent);
};

class ZDVec4Parameter : public ZNumericVectorParameter<glm::dvec4>
{
  Q_OBJECT
public:
  ZDVec4Parameter(const QString &name, glm::dvec4 value, glm::dvec4 min = glm::dvec4(0.f),
                 glm::dvec4 max = glm::dvec4(1.f), QObject *parent = NULL);

signals:
  void value1Changed(double);
  void value2Changed(double);
  void value3Changed(double);
  void value4Changed(double);
public slots:
  void setValue1(double v);
  void setValue2(double v);
  void setValue3(double v);
  void setValue4(double v);

protected:
  virtual void beforeChange(glm::dvec4 &value);
  virtual QWidget* actualCreateWidget(QWidget *parent);
};

class ZIVec2Parameter : public ZNumericVectorParameter<glm::ivec2>
{
  Q_OBJECT
public:
  ZIVec2Parameter(const QString &name, glm::ivec2 value, glm::ivec2 min,
                  glm::ivec2 max, QObject *parent = NULL);

signals:
  void value1Changed(int);
  void value2Changed(int);
public slots:
  void setValue1(int v);
  void setValue2(int v);

protected:
  virtual void beforeChange(glm::ivec2 &value);
  virtual QWidget* actualCreateWidget(QWidget *parent);
};

class ZIVec3Parameter : public ZNumericVectorParameter<glm::ivec3>
{
  Q_OBJECT
public:
  ZIVec3Parameter(const QString &name, glm::ivec3 value, glm::ivec3 min,
                  glm::ivec3 max, QObject *parent = NULL);

signals:
  void value1Changed(int);
  void value2Changed(int);
  void value3Changed(int);
public slots:
  void setValue1(int v);
  void setValue2(int v);
  void setValue3(int v);

protected:
  virtual void beforeChange(glm::ivec3 &value);
  virtual QWidget* actualCreateWidget(QWidget *parent);
};

//---------------------------------------------------------------------------------------------------

template<class T>
class ZNumericSpanParameter : public ZSingleValueParameter<T>
{
public:
  ZNumericSpanParameter(const QString &name, T value, typename T::value_type min, typename T::value_type max, QObject *parent = NULL)
    : ZSingleValueParameter<T>(name, value, parent), m_min(min), m_max(max), m_widgetOrientation(Qt::Horizontal)
  {
    if (std::numeric_limits<typename T::value_type>::is_integer) {
      m_step = 1;
      m_decimal = 0;
    } else {
      m_step = static_cast<typename T::value_type>(.01);
      m_decimal = 3;
    }
    m_tracking = true;
    for (int i=0; i<2; i++) {
      if (this->m_value[i] < m_min)
        this->m_value[i] = m_min;
      if (this->m_value[i] > m_max)
        this->m_value[i] = m_max;
      m_nameOfEachValue.push_back("");
    }
  }

  inline void setSingleStep(typename T::value_type v) { m_step = v; }

  inline void setTracking(bool t) { m_tracking = t; }

  inline void setDecimal(int d) { m_decimal = d; }

  // default is horizonal
  inline void setWidgetOrientation(Qt::Orientation o) { m_widgetOrientation = o; }

  void setNameForEachValue(const QList<QString> &other)
  {
    if (other.size() >= 2)
      m_nameOfEachValue = other;
    else
      LERROR() << "input names do not have enough member";
  }

  void setRange(typename T::value_type min, typename T::value_type max)
  {
    if (min <= max && (min != m_min || max != m_max)) {
      m_min = min;
      m_max = max;
      changeRange();
    }
  }

  typename T::value_type lowerValue() const
  {
    return this->m_value[0];
  }

  typename T::value_type upperValue() const
  {
    return this->m_value[1];
  }

  typename T::value_type minimum() const
  {
    return m_min;
  }

  typename T::value_type maximum() const
  {
    return m_max;
  }

protected:
  virtual void makeValid(T& value) const
  {
    for (int i=0; i<2; i++) {
      if (value[i] < m_min)
        value[i] = m_min;
      if (value[i] > m_max)
        value[i] = m_max;
    }
    if (value[0] > value[1])
      std::swap(value[0], value[1]);
  }

  // inherite this to notify associated widgets about the range change (emit a signal)
  virtual void changeRange() {}


protected:
  typename T::value_type m_min;
  typename T::value_type m_max;
  typename T::value_type m_step;
  bool m_tracking;
  int m_decimal;
  Qt::Orientation m_widgetOrientation;
  QList<QString> m_nameOfEachValue;   // default is empty string for each value
};

class ZIntSpanParameter : public ZNumericSpanParameter<glm::ivec2>
{
  Q_OBJECT
public:
  ZIntSpanParameter(const QString &name, glm::ivec2 value, int min,
                    int max, QObject *parent = NULL);

signals:
  void lowerValueChanged(int);
  void upperValueChanged(int);
  void rangeChanged(int min, int max);
public slots:
  void setLowerValue(int v);
  void setUpperValue(int v);

protected:
  virtual void beforeChange(glm::ivec2 &value);
  virtual QWidget* actualCreateWidget(QWidget *parent);
  virtual void changeRange();
};

class ZFloatSpanParameter : public ZNumericSpanParameter<glm::vec2>
{
  Q_OBJECT
public:
  ZFloatSpanParameter(const QString &name, glm::vec2 value, float min,
                      float max, QObject *parent = NULL);

signals:
  void lowerValueChanged(double);
  void upperValueChanged(double);
  void rangeChanged(double min, double max);
public slots:
  void setLowerValue(double v);
  void setUpperValue(double v);

protected:
  virtual void beforeChange(glm::vec2 &value);
  virtual QWidget* actualCreateWidget(QWidget *parent);
  virtual void changeRange();
};

class ZDoubleSpanParameter : public ZNumericSpanParameter<glm::dvec2>
{
  Q_OBJECT
public:
  ZDoubleSpanParameter(const QString &name, glm::dvec2 value, double min,
                    double max, QObject *parent = NULL);

signals:
  void lowerValueChanged(double);
  void upperValueChanged(double);
  void rangeChanged(double min, double max);
public slots:
  void setLowerValue(double v);
  void setUpperValue(double v);

protected:
  virtual void beforeChange(glm::dvec2 &value);
  virtual QWidget* actualCreateWidget(QWidget *parent);
  virtual void changeRange();
};


#endif // ZNUMERICPARAMETER_H
