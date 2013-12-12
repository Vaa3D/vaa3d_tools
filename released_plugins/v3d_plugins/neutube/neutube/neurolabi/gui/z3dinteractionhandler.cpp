#include "z3dinteractionhandler.h"

#include "z3dcameraparameter.h"
#include "QsLog.h"

#include <cmath>
#include <cassert>

Z3DInteractionHandler::Z3DInteractionHandler(const QString &name, QObject *parent)
  : QObject(parent)
  , m_name(name)
  , m_state(NONE)
  , m_mouseWheelMotionFactor(1.f)
{
}

Z3DInteractionHandler::~Z3DInteractionHandler()
{
  for (size_t i=0; i<m_eventListeners.size(); ++i) {
    delete m_eventListeners[i];
  }
  m_eventListeners.clear();
}

void Z3DInteractionHandler::setEnabled(bool enabled)
{
  for (size_t i=0; i<m_eventListeners.size(); i++)
    m_eventListeners[i]->setEnabled(enabled);
}

void Z3DInteractionHandler::setVisible(bool state)
{
  for (size_t i=0; i<m_eventListeners.size(); i++)
    m_eventListeners[i]->setVisible(state);
}

void Z3DInteractionHandler::setSharing(bool sharing)
{
  for (size_t i=0; i<m_eventListeners.size(); i++)
    m_eventListeners[i]->setSharing(sharing);
}

//////////////////////////////////////////////////////////////////////////////////


Z3DTrackballInteractionHandler::Z3DTrackballInteractionHandler(const QString &name, Z3DCameraParameter *camera, QObject *parent)
  : Z3DInteractionHandler(name, parent)
  , m_camera(camera)
  , m_mouseWheelUpDollyIn(true)
  , m_mouseWheelUpRollLeft(true)
  , m_mouseMotionFactor(10.f)
  , m_keyPressAngle(10.f)
  , m_keyPressDistance(10.f)
  , m_moveObjects(false)
{
  m_rotateEvent = new ZEventListenerParameter(name + " Rotate");
  m_rotateEvent->listenTo("rotate", Qt::LeftButton, Qt::NoModifier, QEvent::MouseButtonPress);
  m_rotateEvent->listenTo("rotate", Qt::LeftButton, Qt::NoModifier, QEvent::MouseButtonRelease);
  m_rotateEvent->listenTo("rotate", Qt::LeftButton, Qt::NoModifier, QEvent::MouseMove);
  connect(m_rotateEvent, SIGNAL(mouseEventTriggered(QMouseEvent*,int,int)), this, SLOT(rotateEvent(QMouseEvent*,int,int)));
  addEventListener(m_rotateEvent);

  m_shiftEvent = new ZEventListenerParameter(name + " Shift");
  m_shiftEvent->listenTo("shift", Qt::LeftButton, Qt::ShiftModifier, QEvent::MouseButtonPress);
  m_shiftEvent->listenTo("shift", Qt::LeftButton, Qt::ShiftModifier, QEvent::MouseButtonRelease);
  m_shiftEvent->listenTo("shift", Qt::LeftButton, Qt::ShiftModifier, QEvent::MouseMove);
  connect(m_shiftEvent, SIGNAL(mouseEventTriggered(QMouseEvent*,int,int)), this, SLOT(shiftEvent(QMouseEvent*,int,int)));
  addEventListener(m_shiftEvent);

  m_wheelDollyEvent = new ZEventListenerParameter(name + " Wheel Dolly");
  m_wheelDollyEvent->listenTo("dolly", Qt::NoButton, Qt::NoModifier, QEvent::Wheel);
  connect(m_wheelDollyEvent, SIGNAL(wheelEventTriggered(QWheelEvent*,int,int)), this, SLOT(dollyEvent(QWheelEvent*,int,int)));
  addEventListener(m_wheelDollyEvent);

  m_rollEvent = new ZEventListenerParameter(name + " Roll");
  m_rollEvent->listenTo("roll", Qt::LeftButton, Qt::AltModifier, QEvent::MouseButtonPress);
  m_rollEvent->listenTo("roll", Qt::LeftButton, Qt::AltModifier, QEvent::MouseButtonRelease);
  m_rollEvent->listenTo("roll", Qt::LeftButton, Qt::AltModifier, QEvent::MouseMove);
  m_rollEvent->listenTo("roll", Qt::NoButton, Qt::AltModifier, QEvent::Wheel);
  connect(m_rollEvent, SIGNAL(mouseEventTriggered(QMouseEvent*,int,int)), this, SLOT(rollEvent(QMouseEvent*,int,int)));
  connect(m_rollEvent, SIGNAL(wheelEventTriggered(QWheelEvent*,int,int)), this, SLOT(rollEvent(QWheelEvent*,int,int)));
  addEventListener(m_rollEvent);

  m_keyRotateEvent = new ZEventListenerParameter(name + " Key Rotate");
  m_keyRotateEvent->listenTo("rotate left", Qt::Key_Left, Qt::KeypadModifier, QEvent::KeyPress);
  m_keyRotateEvent->listenTo("rotate right", Qt::Key_Right, Qt::KeypadModifier, QEvent::KeyPress);
  m_keyRotateEvent->listenTo("rotate up", Qt::Key_Up, Qt::KeypadModifier, QEvent::KeyPress);
  m_keyRotateEvent->listenTo("rotate down", Qt::Key_Down, Qt::KeypadModifier, QEvent::KeyPress);
  connect(m_keyRotateEvent, SIGNAL(keyEventTriggered(QKeyEvent*,int,int)), this, SLOT(keyRotateEvent(QKeyEvent*,int,int)));
  addEventListener(m_keyRotateEvent);

  m_keyShiftEvent = new ZEventListenerParameter(name + " Key Shift");
  m_keyShiftEvent->listenTo("shift left", Qt::Key_Left, Qt::ShiftModifier | Qt::KeypadModifier, QEvent::KeyPress);
  m_keyShiftEvent->listenTo("shift right", Qt::Key_Right, Qt::ShiftModifier | Qt::KeypadModifier, QEvent::KeyPress);
  m_keyShiftEvent->listenTo("shift up", Qt::Key_Up, Qt::ShiftModifier | Qt::KeypadModifier, QEvent::KeyPress);
  m_keyShiftEvent->listenTo("shift down", Qt::Key_Down, Qt::ShiftModifier | Qt::KeypadModifier, QEvent::KeyPress);
  connect(m_keyShiftEvent, SIGNAL(keyEventTriggered(QKeyEvent*,int,int)), this, SLOT(keyShiftEvent(QKeyEvent*,int,int)));
  addEventListener(m_keyShiftEvent);

  m_keyDollyEvent = new ZEventListenerParameter(name + " Key Dolly");
  m_keyDollyEvent->listenTo("dolly in", Qt::Key_Equal, Qt::NoModifier, QEvent::KeyPress);
  m_keyDollyEvent->listenTo("dolly out", Qt::Key_Minus, Qt::NoModifier, QEvent::KeyPress);
  m_keyDollyEvent->listenTo("dolly in", Qt::Key_Up, Qt::ControlModifier | Qt::KeypadModifier, QEvent::KeyPress);
  m_keyDollyEvent->listenTo("dolly out", Qt::Key_Down, Qt::ControlModifier | Qt::KeypadModifier, QEvent::KeyPress);
  connect(m_keyDollyEvent, SIGNAL(keyEventTriggered(QKeyEvent*,int,int)), this, SLOT(keyDollyEvent(QKeyEvent*,int,int)));
  addEventListener(m_keyDollyEvent);

  m_keyRollEvent = new ZEventListenerParameter(name + " Key Roll");
  m_keyRollEvent->listenTo("left", Qt::Key_Left, Qt::AltModifier | Qt::KeypadModifier, QEvent::KeyPress);
  m_keyRollEvent->listenTo("right", Qt::Key_Right, Qt::AltModifier | Qt::KeypadModifier, QEvent::KeyPress);
  connect(m_keyRollEvent, SIGNAL(keyEventTriggered(QKeyEvent*,int,int)), this, SLOT(keyRollEvent(QKeyEvent*,int,int)));
  addEventListener(m_keyRollEvent);
}

Z3DTrackballInteractionHandler::~Z3DTrackballInteractionHandler()
{
}

void Z3DTrackballInteractionHandler::rotateEvent(QMouseEvent *e, int w, int h)
{
  if (e->type() == QEvent::MouseButtonPress) {
    setState(ROTATE);
    mousePressEvent(e, w, h);
  } else if (e->type() == QEvent::MouseButtonRelease) {
    mouseReleaseEvent(e, w, h);
  } else if (e->type() == QEvent::MouseMove) {
    mouseMoveEvent(e, w, h);
    emit cameraMoved();
  }
}

void Z3DTrackballInteractionHandler::dollyEvent(QWheelEvent* e, int w, int h)
{
  setState(DOLLY);
  wheelEvent(e, w, h);
  emit cameraMoved();
}

void Z3DTrackballInteractionHandler::shiftEvent(QMouseEvent *e, int w, int h)
{
  if (e->type() == QEvent::MouseButtonPress) {
    setState(SHIFT);
    mousePressEvent(e, w, h);
  } else if (e->type() == QEvent::MouseButtonRelease) {
    mouseReleaseEvent(e, w, h);
  } else if (e->type() == QEvent::MouseMove) {
    mouseMoveEvent(e, w, h);
    emit cameraMoved();
  }
}

void Z3DTrackballInteractionHandler::rollEvent(QMouseEvent *e, int w, int h)
{
  if (e->type() == QEvent::MouseButtonPress) {
    setState(ROLL);
    mousePressEvent(e, w, h);
  } else if (e->type() == QEvent::MouseButtonRelease) {
    mouseReleaseEvent(e, w, h);
  } else if (e->type() == QEvent::MouseMove) {
    mouseMoveEvent(e, w, h);
    emit cameraMoved();
  }
}

void Z3DTrackballInteractionHandler::rollEvent(QWheelEvent *e, int w, int h)
{
  setState(ROLL);
  wheelEvent(e, w, h);
  emit cameraMoved();
}

void Z3DTrackballInteractionHandler::keyRotateEvent(QKeyEvent *e, int, int)
{
  bool accepted = false;
  if (e->key() == Qt::Key_Left) {
    m_camera->rotate(m_keyPressAngle, m_camera->vectorEyeToWorld(glm::vec3(0.f, 1.f, 0.f)));
    accepted = true;
  } else if (e->key() == Qt::Key_Right) {
    m_camera->rotate(-m_keyPressAngle, m_camera->vectorEyeToWorld(glm::vec3(0.f, 1.f, 0.f)));
    accepted = true;
  } else if (e->key() == Qt::Key_Up) {
    m_camera->rotate(-m_keyPressAngle, m_camera->vectorEyeToWorld(glm::vec3(1.f, 0.f, 0.f)));
    accepted = true;
  } else if (e->key() == Qt::Key_Down) {
    m_camera->rotate(m_keyPressAngle, m_camera->vectorEyeToWorld(glm::vec3(1.f, 0.f, 0.f)));
    accepted = true;
  }
  if (accepted) {
    e->accept();
    emit cameraMoved();
  }
}

void Z3DTrackballInteractionHandler::keyShiftEvent(QKeyEvent *e, int w, int h)
{
  bool accepted = false;
  glm::ivec2 center(w/2, h/2);
  if (e->key() == Qt::Key_Left) {
    shift(center, center + glm::ivec2(-m_keyPressDistance, 0), w, h);
    accepted = true;
  } else if (e->key() == Qt::Key_Right) {
    shift(center, center + glm::ivec2(m_keyPressDistance, 0), w, h);
    accepted = true;
  } else if (e->key() == Qt::Key_Up) {
    shift(center, center + glm::ivec2(0, m_keyPressDistance), w, h);
    accepted = true;
  } else if (e->key() == Qt::Key_Down) {
    shift(center, center + glm::ivec2(0, -m_keyPressDistance), w, h);
    accepted = true;
  }
  if (accepted) {
    e->accept();
    emit cameraMoved();
  }
}

void Z3DTrackballInteractionHandler::keyDollyEvent(QKeyEvent *e, int, int)
{
  bool accepted = false;
  float factor = m_mouseMotionFactor * 0.1f * m_mouseWheelMotionFactor;
  if (e->key() == Qt::Key_Up || e->key() == Qt::Key_Equal) {
    m_camera->dolly(std::pow(1.1f, factor));
    accepted = true;
  }
  else if (e->key() == Qt::Key_Down || e->key() == Qt::Key_Minus) {
    m_camera->dolly(std::pow(1.1f, -factor));
    accepted = true;
  }
  if (accepted) {
    e->accept();
    emit cameraMoved();
  }
}

void Z3DTrackballInteractionHandler::keyRollEvent(QKeyEvent *e, int, int)
{
  bool accepted = false;
  if (e->key() == Qt::Key_Left) {
    m_camera->roll(m_keyPressAngle);
    accepted = true;
  } else if (e->key() == Qt::Key_Right) {
    m_camera->roll(-m_keyPressAngle);
    accepted = true;
  }
  if (accepted) {
    e->accept();
    emit cameraMoved();
  }
}

void Z3DTrackballInteractionHandler::mousePressEvent(QMouseEvent *e, int, int h)
{
  m_lastMousePosition = glm::ivec2(e->x(), h - e->y());
  e->ignore();
}

void Z3DTrackballInteractionHandler::mouseReleaseEvent(QMouseEvent *e, int, int)
{
  setState(NONE);
  e->ignore();
}

void Z3DTrackballInteractionHandler::mouseMoveEvent(QMouseEvent *e, int w, int h)
{
  e->ignore();

  glm::ivec2 newMouse(e->x(), h- e->y());

  if (m_state == ROTATE) {
    rotate(m_lastMousePosition, newMouse, w, h);
    e->accept();
  } else if (m_state == SHIFT) {
    shift(m_lastMousePosition, newMouse, w, h);
    e->accept();
  } else if (m_state == ROLL) {
    roll(m_lastMousePosition, newMouse, w, h);
    e->accept();
  }

  m_lastMousePosition = newMouse;
}

void Z3DTrackballInteractionHandler::wheelEvent(QWheelEvent *e, int, int)
{
  e->ignore();

  if (m_state == DOLLY) {
    float factor = m_mouseMotionFactor * 0.2f * m_mouseWheelMotionFactor;
    bool dollyIn = ( m_mouseWheelUpDollyIn && (e->delta() > 0)) ||
        (!m_mouseWheelUpDollyIn && (e->delta() < 0));
    if (!dollyIn)
      factor = -factor;
    m_camera->dolly(std::pow(1.1f, factor));
    e->accept();
  } else if (m_state == ROLL) {
    bool rollLeft = ( m_mouseWheelUpRollLeft && (e->delta() > 0)) ||
        (!m_mouseWheelUpRollLeft && (e->delta() < 0));
    if (rollLeft)
      m_camera->roll(m_keyPressAngle);
    else
      m_camera->roll(-m_keyPressAngle);
    e->accept();
  }
  setState(NONE);
}

void Z3DTrackballInteractionHandler::shift(glm::ivec2 mouseStart, glm::ivec2 mouseEnd, int w, int h)
{
  glm::ivec4 viewport(0, 0, w, h);
  float centerDepth = m_camera->worldToScreen(m_camera->getCenter(), viewport).z;
  glm::vec3 startInWorld = m_camera->screenToWorld(glm::vec3(glm::vec2(mouseStart), centerDepth), viewport);
  glm::vec3 endInWorld = m_camera->screenToWorld(glm::vec3(glm::vec2(mouseEnd), centerDepth), viewport);
  glm::vec3 vec = endInWorld - startInWorld;
  if (m_moveObjects) {
    emit objectsMoved(vec.x, vec.y, vec.z);
  } else {
    // camera move in opposite direction
    m_camera->setCamera(m_camera->getEye() - vec, m_camera->getCenter() - vec);
  }
}

void Z3DTrackballInteractionHandler::rotate(glm::ivec2 mouseStart, glm::ivec2 mouseEnd, int w, int h)
{
  glm::ivec2 dPos = mouseEnd - mouseStart;

  double delta_elevation = -36.0 / h;
  double delta_azimuth = -36.0 / w;

  double rxf = dPos.x * delta_azimuth * m_mouseMotionFactor;
  double ryf = dPos.y * delta_elevation * m_mouseMotionFactor;

  m_camera->azimuth(rxf);
  m_camera->elevation(ryf);
}

void Z3DTrackballInteractionHandler::roll(glm::ivec2 mouseStart, glm::ivec2 mouseEnd, int w, int h)
{
  glm::dvec2 center(w/2., h/2.);
  double newAngle = glm::degrees(std::atan2(mouseEnd.y - center.y, mouseEnd.x - center.x));
  double oldAngle = glm::degrees(std::atan2(mouseStart.y - center.y, mouseStart.x - center.x));

  m_camera->roll(newAngle - oldAngle);
}
