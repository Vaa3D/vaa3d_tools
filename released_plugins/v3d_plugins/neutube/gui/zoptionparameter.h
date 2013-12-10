#ifndef ZOPTIONPARAMETER_H
#define ZOPTIONPARAMETER_H

#include "zparameter.h"
#include "zcombobox.h"
#include <QList>
#include <QString>
#include "QsLog.h"


// One of many options parameter. T is the description type, which should be
// convertible to QString if create widget. T2 is data type associated with T.
// Associated data is optional. It can be retrieved by getAssociatedData.

template<class T, class T2 = int>
class ZOptionParameter : public ZSingleValueParameter<T>
{
public:
  ZOptionParameter(const QString &name, QObject *parent = NULL, const QString &prefix = "",
                   const QString &suffix = "");
  virtual ~ZOptionParameter() {}

  void select(const T& value);
  bool isSelected(const T& value) const;
  bool isEmpty() const { return m_options.empty(); }

  inline T2 getAssociatedData() const {return m_associatedData;}

  void addOption(const T& value)
  {
    if (m_options.indexOf(value) != -1) {
      return;
    }
    m_options.push_back(value);
    m_associatedDatas.push_back(T2());
    emit this->reservedStringSignal1(getComboBoxItemString(value));
    if (!m_dataIsValid) {
      select(value);
      m_dataIsValid = true;
    }
  }

  void clearOptions()
  {
    m_options.clear();
    m_associatedDatas.clear();
    m_dataIsValid = false;
    emit this->reservedSignal1();
  }

  void removeOption(const T& value)
  {
    int idx = m_options.indexOf(value);
    if (idx == -1)
      return;
    m_options.removeAt(idx);
    m_associatedDatas.removeAt(idx);
    emit this->reservedStringSignal2(getComboBoxItemString(value));
    int index = m_options.indexOf(this->m_value);
    if (index != -1)
      emit this->reservedIntSignal1(index);
    else {
      emit this->reservedIntSignal1(0);
      this->set(m_options[0]);
    }
  }

  inline void addOptions(const T& op1, const T& op2)
  {
    addOption(op1);
    addOption(op2);
  }

  inline void addOptions(const T& op1, const T& op2, const T& op3)
  {
    addOption(op1);
    addOptions(op2, op3);
  }

  inline void addOptions(const T& op1, const T& op2, const T& op3, const T& op4)
  {
    addOption(op1);
    addOptions(op2, op3, op4);
  }

  inline void addOptions(const T& op1, const T& op2, const T& op3, const T& op4, const T& op5)
  {
    addOption(op1);
    addOptions(op2, op3, op4, op5);
  }

  inline void addOptions(const T& op1, const T& op2, const T& op3, const T& op4, const T& op5,
                         const T& op6)
  {
    addOption(op1);
    addOptions(op2, op3, op4, op5, op6);
  }

  inline void addOptions(const T& op1, const T& op2, const T& op3, const T& op4, const T& op5,
                         const T& op6, const T& op7)
  {
    addOption(op1);
    addOptions(op2, op3, op4, op5, op6, op7);
  }

  inline void addOptions(const T& op1, const T& op2, const T& op3, const T& op4, const T& op5,
                         const T& op6, const T& op7, const T& op8)
  {
    addOption(op1);
    addOptions(op2, op3, op4, op5, op6, op7, op8);
  }

  inline void addOptionWithData(const QPair<T, T2>& value)
  {
    if (m_options.indexOf(value.first) != -1)
      return;
    m_options.push_back(value.first);
    m_associatedDatas.push_back(value.second);
    emit this->reservedStringSignal1(getComboBoxItemString(value.first));
    if (!m_dataIsValid) {
      select(value.first);
      m_dataIsValid = true;
    }
  }

  inline void addOptionsWithData(const QPair<T, T2>& op1, const QPair<T, T2>& op2)
  {
    addOptionWithData(op1);
    addOptionWithData(op2);
  }

  inline void addOptionsWithData(const QPair<T, T2>& op1, const QPair<T, T2>& op2, const QPair<T, T2>& op3)
  {
    addOptionWithData(op1);
    addOptionsWithData(op2, op3);
  }

  inline void addOptionsWithData(const QPair<T, T2>& op1, const QPair<T, T2>& op2, const QPair<T, T2>& op3, const QPair<T, T2>& op4)
  {
    addOptionWithData(op1);
    addOptionsWithData(op2, op3, op4);
  }

  inline void addOptionsWithData(const QPair<T, T2>& op1, const QPair<T, T2>& op2, const QPair<T, T2>& op3, const QPair<T, T2>& op4,
                                 const QPair<T, T2>& op5)
  {
    addOptionWithData(op1);
    addOptionsWithData(op2, op3, op4, op5);
  }

  inline void addOptionsWithData(const QPair<T, T2>& op1, const QPair<T, T2>& op2, const QPair<T, T2>& op3, const QPair<T, T2>& op4,
                                 const QPair<T, T2>& op5, const QPair<T, T2>& op6)
  {
    addOptionWithData(op1);
    addOptionsWithData(op2, op3, op4, op5, op6);
  }

  inline void addOptionsWithData(const QPair<T, T2>& op1, const QPair<T, T2>& op2, const QPair<T, T2>& op3, const QPair<T, T2>& op4,
                                 const QPair<T, T2>& op5, const QPair<T, T2>& op6, const QPair<T, T2>& op7)
  {
    addOptionWithData(op1);
    addOptionsWithData(op2, op3, op4, op5, op6, op7);
  }

  inline void addOptionsWithData(const QPair<T, T2>& op1, const QPair<T, T2>& op2, const QPair<T, T2>& op3, const QPair<T, T2>& op4,
                                 const QPair<T, T2>& op5, const QPair<T, T2>& op6, const QPair<T, T2>& op7, const QPair<T, T2>& op8)
  {
    addOptionWithData(op1);
    addOptionsWithData(op2, op3, op4, op5, op6, op7, op8);
  }

protected:
  virtual void reservedIntSlot1(int v);
  virtual QWidget* actualCreateWidget(QWidget* parent);
  virtual void beforeChange(T &value);
  virtual void makeValid(T& value) const;
  QString getComboBoxItemString(const T &value) const;

  QList<T> m_options;
  T2 m_associatedData;
  QList<T2> m_associatedDatas;

  bool m_dataIsValid;
  QString m_prefix;
  QString m_suffix;
};

template<class T, class T2>
ZOptionParameter<T, T2>::ZOptionParameter(const QString &name, QObject *parent, const QString &prefix,
                                          const QString &suffix)
  : ZSingleValueParameter<T>(name, parent), m_dataIsValid(false)
  , m_prefix(prefix), m_suffix(suffix)
{
}

template<class T, class T2>
void ZOptionParameter<T, T2>::select(const T &value)
{
  this->set(value);
}

template<class T, class T2>
bool ZOptionParameter<T, T2>::isSelected(const T &value) const
{
#if defined(_DEBUG_)
  if (!m_options.contains(value)) {
    LERROR() << QString("Option <%1> does not exist.").arg(value);
  }
#endif
  return value == this->m_value;
}

template<class T, class T2>
void ZOptionParameter<T, T2>::reservedIntSlot1(int index)
{
  // notify all widgets
  if (index >= 0 && index < m_options.size())
    this->set(m_options[index]);
}

template<class T, class T2>
QWidget* ZOptionParameter<T, T2>::actualCreateWidget(QWidget* parent)
{
  ZComboBox *cb = new ZComboBox(parent);

  for (int i=0; i<m_options.size(); i++) {
    cb->addItem(getComboBoxItemString(m_options[i]));
  }
  if (!m_options.empty()) {
    int index = m_options.indexOf(this->m_value);
    if (index != -1)
      cb->setCurrentIndex(index);
    else {
      cb->setCurrentIndex(0);
      this->set(m_options[0]);
    }
  }
  this->connect(this, SIGNAL(reservedIntSignal1(int)), cb, SLOT(setCurrentIndex(int)));
  this->connect(this, SIGNAL(reservedStringSignal1(QString)), cb, SLOT(addItemSlot(QString)));
  this->connect(this, SIGNAL(reservedStringSignal2(QString)), cb, SLOT(removeItemSlot(QString)));
  this->connect(this, SIGNAL(reservedSignal1()), cb, SLOT(clear()));
  this->connect(cb, SIGNAL(currentIndexChanged(int)), this, SLOT(reservedIntSlot1(int)));
  return cb;
}

template<class T, class T2>
void ZOptionParameter<T, T2>::makeValid(T &value) const
{
  if (!m_options.contains(value)) {
    LERROR() << QString("Optiong value <%1> does not exist.").arg(value);
    if (m_options.empty())
      LERROR() << QString("Error: Try to select <%1> from empty options list. Call addOptions() first!").arg(value);
    else if (m_dataIsValid) {
      LERROR() << QString("Warning: Select failed, value is still <%1>").arg(this->m_value);
      value = this->m_value;
    } else {
      LERROR() << QString("Default to first option <%1>").arg(m_options[0]);
      value = m_options[0];
    }
  }
}

template<class T, class T2>
QString ZOptionParameter<T,T2>::getComboBoxItemString(const T &value) const
{
  return QString("%1%2%3").arg(m_prefix).arg(value).arg(m_suffix);
}

template<class T, class T2>
void ZOptionParameter<T, T2>::beforeChange(T &value)
{
  int index = m_options.indexOf(value);
  m_associatedData = m_associatedDatas[index];
  emit this->reservedIntSignal1(index);
}


#endif // ZOPTIONPARAMETER_H
