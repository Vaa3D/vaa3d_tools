#include "z3dcameraparameter.h"
#include <QWidget>
#include <QGroupBox>
#include "zwidgetsgroup.h"

Z3DCameraParameter::Z3DCameraParameter(const QString &name, const Z3DCamera &value, QObject *parent)
  : ZSingleValueParameter<Z3DCamera>(name, value, parent)
  , m_projectionType("Projection Type")
  , m_eye("Eye Position", value.getEye(), glm::vec3(-std::numeric_limits<float>::max()),
          glm::vec3(std::numeric_limits<float>::max()))
  , m_center("Center Position", value.getCenter(), glm::vec3(-std::numeric_limits<float>::max()),
             glm::vec3(std::numeric_limits<float>::max()))
  , m_upVector("Up Vector", value.getUpVector(), glm::vec3(-1.f), glm::vec3(1.f))
  , m_eyeSeparationAngle("Eye Separation Angle", value.getEyeSeparationAngle(), 1.f, 80.f)
  , m_fieldOfView("Field of View", value.getFieldOfView(), 10.f, 170.f)
  , m_nearDist("Near Distance", value.getNearDist(), 1e-10, std::numeric_limits<float>::max())
  , m_farDist("Far Distance", value.getFarDist(), 1e-10, std::numeric_limits<float>::max())
  , m_receiveWidgetSignal(true)
{
  m_projectionType.addOptions("Perspective", "Orthographic");
  if (value.isPerspectiveProjection())
    m_projectionType.select("Perspective");
  else
    m_projectionType.select("Orthographic");
  connect(&m_projectionType, SIGNAL(valueChanged()), this, SLOT(updateProjectionType()));

  m_eye.setSingleStep(1e-10);
  m_eye.setDecimal(10);
  //m_eye.setWidgetOrientation(Qt::Horizontal);
  m_eye.setStyle("SPINBOX");
  connect(&m_eye, SIGNAL(valueChanged()), this, SLOT(updateEye()));

  m_center.setSingleStep(1e-10);
  m_center.setDecimal(10);
  //m_center.setWidgetOrientation(Qt::Horizontal);
  m_center.setStyle("SPINBOX");
  connect(&m_center, SIGNAL(valueChanged()), this, SLOT(updateCenter()));

  m_upVector.setSingleStep(1e-10);
  m_upVector.setDecimal(10);
  m_upVector.setStyle("SPINBOX");
  connect(&m_upVector, SIGNAL(valueChanged()), this, SLOT(updateUpVector()));

  m_eyeSeparationAngle.setSingleStep(.1);
  m_eyeSeparationAngle.setDecimal(1);
  connect(&m_eyeSeparationAngle, SIGNAL(valueChanged()), this, SLOT(updateEyeSeparationAngle()));

  m_fieldOfView.setSingleStep(.1);
  m_fieldOfView.setDecimal(1);
  connect(&m_fieldOfView, SIGNAL(valueChanged()), this, SLOT(updateFieldOfView()));

  m_nearDist.setSingleStep(1e-10);
  m_nearDist.setDecimal(10);
  m_nearDist.setStyle("SPINBOX");
  m_nearDist.setRange(1e-10, value.getFarDist());
  connect(&m_nearDist, SIGNAL(valueChanged()), this, SLOT(updateNearDist()));

  m_farDist.setSingleStep(1e-10);
  m_farDist.setDecimal(10);
  m_farDist.setStyle("SPINBOX");
  m_farDist.setRange(value.getNearDist(), std::numeric_limits<float>::max());
  connect(&m_farDist, SIGNAL(valueChanged()), this, SLOT(updateFarDist()));
}

void Z3DCameraParameter::flipViewDirection()
{
  glm::vec3 referenceCenter = getCenter();
  glm::vec3 eyePosition = getEye();

  glm::vec3 viewVector = eyePosition - referenceCenter;
  setEye(referenceCenter - viewVector);
}

void Z3DCameraParameter::viewportChanged(const glm::ivec2 &viewport)
{
  m_value.setWindowAspectRatio(static_cast<float>(viewport.x) / viewport.y);
  emit valueChanged();
}

void Z3DCameraParameter::updateProjectionType()
{
  if (m_receiveWidgetSignal) {
    if (m_projectionType.isSelected("Perspective"))
      m_value.setProjectionType(Z3DCamera::Perspective);
    else
      m_value.setProjectionType(Z3DCamera::Orthographic);
    emit valueChanged();
  }
}

void Z3DCameraParameter::updateEye()
{
  if (m_receiveWidgetSignal) {
    m_value.setEye(m_eye.get());
    emit valueChanged();
  }
}

void Z3DCameraParameter::updateCenter()
{
  if (m_receiveWidgetSignal) {
    m_value.setCenter(m_center.get());
    emit valueChanged();
  }
}

void Z3DCameraParameter::updateUpVector()
{
  if (m_receiveWidgetSignal) {
    m_value.setUpVector(m_upVector.get());
    emit valueChanged();
  }
}

void Z3DCameraParameter::updateEyeSeparationAngle()
{
  if (m_receiveWidgetSignal) {
    m_value.setEyeSeparationAngle(m_eyeSeparationAngle.get());
    emit valueChanged();
  }
}

void Z3DCameraParameter::updateFieldOfView()
{
  if (m_receiveWidgetSignal) {
    m_value.setFieldOfView(m_fieldOfView.get());
    emit valueChanged();
  }
}

void Z3DCameraParameter::updateNearDist()
{
  if (m_receiveWidgetSignal) {
    m_value.setNearDist(m_nearDist.get());
    m_farDist.setRange(m_value.getNearDist(), std::numeric_limits<float>::max());
    emit valueChanged();
  }
}

void Z3DCameraParameter::updateFarDist()
{
  if (m_receiveWidgetSignal) {
    m_value.setFarDist(m_farDist.get());
    m_nearDist.setRange(1e-10, m_value.getFarDist());
    emit valueChanged();
  }
}

QWidget *Z3DCameraParameter::actualCreateWidget(QWidget *parent)
{
  ZWidgetsGroup *camera = new ZWidgetsGroup("Camera", NULL, 1);
  new ZWidgetsGroup(&m_projectionType, camera, 1);
  new ZWidgetsGroup(&m_eye, camera, 1);
  new ZWidgetsGroup(&m_center, camera, 1);
  new ZWidgetsGroup(&m_upVector, camera, 1);
  new ZWidgetsGroup(&m_eyeSeparationAngle, camera, 1);
  new ZWidgetsGroup(&m_fieldOfView, camera, 1);
  new ZWidgetsGroup(&m_nearDist, camera, 1);
  new ZWidgetsGroup(&m_farDist, camera, 1);

  QLayout *lw = camera->createLayout(NULL, false);
  //QWidget *widget = new QWidget();
  //widget->setLayout(lw);
  QGroupBox *groupBox = new QGroupBox("Camera Parameters", parent);
  groupBox->setLayout(lw);

  //widget->setParent(parent);
  delete camera;
  //return widget;
  return groupBox;
}

void Z3DCameraParameter::beforeChange(Z3DCamera &value)
{
  updateWidget(value);
}

void Z3DCameraParameter::updateWidget(Z3DCamera &value)
{
  m_receiveWidgetSignal = false;
  m_eye.set(value.getEye());
  m_center.set(value.getCenter());
  m_upVector.set(value.getUpVector());
  if (value.isPerspectiveProjection())
    m_projectionType.select("Perspective");
  else
    m_projectionType.select("Orthographic");
  m_eyeSeparationAngle.set(value.getEyeSeparationAngle());
  m_fieldOfView.set(value.getFieldOfView());
  m_nearDist.set(value.getNearDist());
  m_farDist.set(value.getFarDist());
  m_nearDist.setRange(1e-10, value.getFarDist());
  m_farDist.setRange(value.getNearDist(), std::numeric_limits<float>::max());
  m_receiveWidgetSignal = true;
}
