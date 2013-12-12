#ifndef Z3DINTERACTIONHANDLER_H
#define Z3DINTERACTIONHANDLER_H

#include "zeventlistenerparameter.h"
#include "zglmutils.h"

#include <QObject>

class Z3DInteractionHandler : public QObject
{
  Q_OBJECT
public:
  Z3DInteractionHandler(const QString& name, QObject *parent = 0);
  virtual ~Z3DInteractionHandler();

  void setName(const QString& name) { m_name = name; }
  inline QString getName() const { return m_name; }

  // Default to 1.0. Set it to a different value to emphasize or de-emphasize the action triggered by
  // mouse wheel motion.
  void setMouseWheelMotionFactor(float f) { m_mouseWheelMotionFactor = f; }
  float getMouseWheelMotionFactor() const { return m_mouseWheelMotionFactor; }

  void setEnabled(bool enabled);
  void setVisible(bool state);
  void setSharing(bool sharing);

  const std::vector<ZEventListenerParameter*>& getEventListeners() const
  {return m_eventListeners;}

signals:
  void cameraMoved();

protected:
  enum State {
    NONE,
    ROTATE,
    ROLL,
    DOLLY,
    ZOOM,
    SHIFT
  };
  void setState(State state) { m_state = state; }

  void addEventListener(ZEventListenerParameter* eventListener)
  { m_eventListeners.push_back(eventListener); }

  QString m_name;
  std::vector<ZEventListenerParameter*> m_eventListeners;
  State m_state;

  float m_mouseWheelMotionFactor;
};

class Z3DCameraParameter;

class Z3DTrackballInteractionHandler : public Z3DInteractionHandler
{
  Q_OBJECT
public:
  Z3DTrackballInteractionHandler(const QString& name, Z3DCameraParameter* camera, QObject *parent = 0);
  virtual ~Z3DTrackballInteractionHandler();

  Z3DCameraParameter* getCamera() const { return m_camera; }
  void setCamera(Z3DCameraParameter* camera) { m_camera = camera; }

  // dolly in on wheel up and out on wheel down when true (default), otherwise when false
  void setMouseWheelUpDollyIn(bool b) { m_mouseWheelUpDollyIn = b; }
  // roll left on wheel up and right on wheel down when true (default), otherwise when false
  void setMouseWheelUpRollLeft(bool b) { m_mouseWheelUpRollLeft = b; }

  // more sensitive (bigger motion) at bigger mouse motion factor
  void setMouseMotionFactor(float f) { m_mouseMotionFactor = f; }
  float getMouseMotionFactor() const { return m_mouseMotionFactor; }

  // angle per key press in degree
  void setKeyPressAngle(float degree) { m_keyPressAngle = degree; }
  float getkeyPressAngle() const { return m_keyPressAngle; }

  // distance per key press in pixels
  void setKeyPressDistance(int d) { m_keyPressDistance = d; }
  int getKeyPressDistance() const { return m_keyPressDistance; }

  void setMoveObjects(bool v) { m_moveObjects = v; }
  bool isMovingObjects() const { return m_moveObjects; }

signals:
  void objectsMoved(double x, double y, double z);

protected slots:
  void rotateEvent(QMouseEvent* e, int w, int h);
  void dollyEvent(QWheelEvent *e, int w, int h);
  void shiftEvent(QMouseEvent *e, int w, int h);
  void rollEvent(QMouseEvent *e, int w, int h);
  void rollEvent(QWheelEvent *e, int w, int h);
  virtual void keyRotateEvent(QKeyEvent* e, int w, int h);
  virtual void keyShiftEvent(QKeyEvent* e, int w, int h);
  virtual void keyDollyEvent(QKeyEvent* e, int w, int h);
  virtual void keyRollEvent(QKeyEvent* e, int w, int h);

protected:
  virtual void mousePressEvent(QMouseEvent* e, int w, int h);
  virtual void mouseReleaseEvent(QMouseEvent* e, int w, int h);
  virtual void mouseMoveEvent(QMouseEvent* e, int w, int h);
  virtual void wheelEvent(QWheelEvent* e, int w, int h);

  // convert screen space move to world space move and move camera eye and center
  void shift(glm::ivec2 mouseStart, glm::ivec2 mouseEnd, int w, int h);
  // use mouse move and mouseMotionFactor to calculate angle then rotate camera
  void rotate(glm::ivec2 mouseStart, glm::ivec2 mouseEnd, int w, int h);
  void roll(glm::ivec2 mouseStart, glm::ivec2 mouseEnd, int w, int h);

  ZEventListenerParameter* m_rotateEvent;
  ZEventListenerParameter* m_shiftEvent;
  ZEventListenerParameter* m_wheelDollyEvent;
  ZEventListenerParameter* m_rollEvent;
  ZEventListenerParameter* m_keyRotateEvent;
  ZEventListenerParameter* m_keyShiftEvent;
  ZEventListenerParameter* m_keyDollyEvent;
  ZEventListenerParameter* m_keyRollEvent;

  Z3DCameraParameter *m_camera;
  glm::ivec2 m_lastMousePosition;

  bool m_mouseWheelUpDollyIn;
  bool m_mouseWheelUpRollLeft;

  // sensitivity of the interactor to mouse motion.
  float m_mouseMotionFactor;
  // sensitivity of the interactor to key press
  float m_keyPressAngle;
  int m_keyPressDistance;

  bool m_moveObjects;
};

#endif // Z3DINTERACTIONHANDLER_H
