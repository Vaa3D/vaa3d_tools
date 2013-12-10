#ifndef ZEVENTLISTENERPARAMETER_H
#define ZEVENTLISTENERPARAMETER_H

#include "zparameter.h"
#include <QList>
#include <QMouseEvent>
#include <QKeyEvent>

class ZEventListenerParameter : public ZParameter
{
  Q_OBJECT
public:
  ZEventListenerParameter(const QString& name, bool acceptEvent = true, bool sharing = false, QObject* parent = NULL);

  inline void setSharing(bool s) { if (m_sharing != s) { m_sharing = s; emit valueChanged(); } }
  inline bool isSharing() const { return m_sharing; }
  inline void setAcceptingEvent(bool s) { if (m_acceptEvent != s) { m_acceptEvent = s; emit valueChanged(); } }
  inline bool isAcceptingEvent() const { return m_acceptEvent; }

  // buttons and modifiers should be exact match with the input to trigger the event signal
  // An OR-combinations of buttons means all buttons should be pressed at the time
  // An OR-combinations of modifiers means all modifiers should be pressed at the time
  void listenTo(const QString& actionName, Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, QEvent::Type type);
  //
  void listenTo(const QString& actionName, Qt::Key key, Qt::KeyboardModifiers modifiers, QEvent::Type type = QEvent::KeyPress);

  void clearAll();

  void sendEvent(QEvent* e, int w, int h);

signals:
  void eventTriggered(QEvent* e, int w, int h);
  void mouseEventTriggered(QMouseEvent* e, int w, int h);
  void keyEventTriggered(QKeyEvent* e, int w, int h);
  void wheelEventTriggered(QWheelEvent* e, int w, int h);

protected:
  struct MouseEvent
  {
    MouseEvent(const QString& actionName, Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, QEvent::Type type)
      : actionName(actionName), buttons(buttons), modifiers(modifiers), type(type)
    {}

    QString actionName;
    Qt::MouseButtons buttons;
    Qt::KeyboardModifiers modifiers;
    QEvent::Type type;
  };

  struct KeyEvent
  {
    KeyEvent(const QString& actionName, Qt::Key key, Qt::KeyboardModifiers modifiers, QEvent::Type type = QEvent::KeyPress)
      : actionName(actionName), key(key), modifiers(modifiers), type(type)
    {}

    QString actionName;
    Qt::Key key;
    Qt::KeyboardModifiers modifiers;
    QEvent::Type type;
  };

  virtual QWidget* actualCreateWidget(QWidget *parent);

  bool m_acceptEvent;
  bool m_sharing;
  QList<MouseEvent> m_mouseEvents;
  QList<KeyEvent> m_keyEvents;
};

#endif // ZEVENTLISTENERPARAMETER_H
