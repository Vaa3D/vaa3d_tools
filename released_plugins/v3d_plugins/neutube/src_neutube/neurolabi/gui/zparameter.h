#ifndef ZPARAMETER_H
#define ZPARAMETER_H

// Base class for all parameters that algorithms or renderers need.
// A parameter can emit changed() signal while changed. And it has
// createWidget functions which can be used for UI
// generation. The changed() signal can be used to change algorithm
// or renderer behavior dynamicly.

#include <QObject>
#include <QStringList>
#include <set>

class QWidget;
class QLayout;
class QLabel;
class QMainWindow;

class ZParameter : public QObject
{
  Q_OBJECT
public:
  explicit ZParameter(const QString& name, QObject *parent = 0);
  virtual ~ZParameter();

  inline QString getName() const {return m_name;}
  void setName(const QString& name);

  inline QString getStyle() const {return m_style;}
  // if style does not exist, fall back to "DEFAULT"
  void setStyle(const QString& style);

  // some widget might need mainwindow
  inline void setMainWindow(QMainWindow* mainWin) {m_mainWin = mainWin;}

  // create widget based on current style
  QLabel* createNameLabel(QWidget* parent = NULL);
  QWidget* createWidget(QWidget* parent = NULL);

  void setVisible(bool s);
  void setEnabled(bool s);
  
signals:
  void nameChanged(const QString &);
  void valueChanged();

  void setWidgetsEnabled(bool s);
  void setWidgetsVisible(bool s);

  // some templated subclass might need this
  void reservedIntSignal1(int);
  void reservedIntSignal2(int);
  void reservedStringSignal1(QString);
  void reservedStringSignal2(QString);
  void reservedSignal1();
  void reservedSignal2();

protected slots:
  // some templated subclass might need this
  virtual void reservedIntSlot1(int) {}
  virtual void reservedIntSlot2(int) {}
  virtual void reservedStringSlot1(QString) {}
  virtual void reservedStringSlot2(QString) {}
  virtual void reservedSlot1() {}
  virtual void reservedSlot2() {}

  virtual void updateFromDependee() {}

protected:
  inline void addStyle(const QString& style) {m_allStyles.push_back(style);}
  // all subclass should implement this function
  virtual QWidget* actualCreateWidget(QWidget* parent) = 0;

  QString m_name;
  QString m_style;
  QStringList m_allStyles;

  //std::set<QWidget*> m_widgets;
  bool m_isWidgetsEnabled;
  bool m_isWidgetsVisible;

  QMainWindow *m_mainWin;
};

// parameter contains a single value
template<class T>
class ZSingleValueParameter : public ZParameter {
public:
  ZSingleValueParameter(const QString &name, T value, QObject *parent = NULL);
  ZSingleValueParameter(const QString &name, QObject *parent = NULL);

  virtual ~ZSingleValueParameter();

  void set(const T &valueIn);

  inline const T& get() const { return m_value; }
  inline T& get() {return m_value; }

  void dependsOn(ZSingleValueParameter<T>* other);

protected:
  // subclass can use this function to change input value to a valid value
  // default implement do nothing
  virtual void makeValid(T& value) const;
  // subclass can use this function to emit customized signal before m_value
  // is changed or do other update, input is new value
  virtual void beforeChange(T& value);

  virtual void updateFromDependee();

  T m_value;
  ZSingleValueParameter<T>* m_dependee;
  bool m_locked;
};

//---------------------------------------------------------------------------

template<class T>
ZSingleValueParameter<T>::ZSingleValueParameter(const QString& name, T value, QObject *parent)
  : ZParameter(name, parent)
  , m_value(value), m_dependee(NULL), m_locked(false)
{}

template<class T>
ZSingleValueParameter<T>::ZSingleValueParameter(const QString& name, QObject *parent)
  : ZParameter(name, parent), m_dependee(NULL), m_locked(false)
{}

template<class T>
ZSingleValueParameter<T>::~ZSingleValueParameter()
{}

template<class T>
void ZSingleValueParameter<T>::set(const T &valueIn)
{
  if (m_locked)
    return;    // prevent widget change echo back
  if (m_value != valueIn) {
    T value = valueIn;
    makeValid(value);
    if (m_value != value) {
      m_locked = true;
      beforeChange(value);
      m_value = value;
      emit valueChanged();
      m_locked = false;
    }
  }
}

template<class T>
void ZSingleValueParameter<T>::dependsOn(ZSingleValueParameter<T> *other)
{
  if (other != m_dependee) {
    if (m_dependee)
      m_dependee->disconnect(this);
    m_dependee = other;
    updateFromDependee();
    connect(m_dependee, SIGNAL(valueChanged()), this, SLOT(updateFromDependee()));
  }
}

template<class T>
void ZSingleValueParameter<T>::makeValid(T &/*value*/) const
{
}

template<class T>
void ZSingleValueParameter<T>::beforeChange(T &/*value*/)
{
}

template<class T>
void ZSingleValueParameter<T>::updateFromDependee()
{
  set(m_dependee->get());
}

//-----------------------------------------------------------------------------------------------

class ZBoolParameter : public ZSingleValueParameter<bool>
{
  Q_OBJECT
public:
  ZBoolParameter(const QString& name, bool value, QObject *parent = NULL);
signals:
  void valueChanged(bool);
public slots:
  void setValue(bool v);

protected:
  virtual void beforeChange(bool &value);
  virtual QWidget* actualCreateWidget(QWidget *parent);
};

#endif // ZPARAMETER_H
