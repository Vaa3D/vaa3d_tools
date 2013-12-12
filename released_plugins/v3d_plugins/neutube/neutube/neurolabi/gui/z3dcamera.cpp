#include "z3dcamera.h"
#include <cmath>

Z3DCamera::Z3DCamera()
  : m_eye(0.f, 0.f, 0.f)
  , m_center(0.f, 0.f, -1.f)
  , m_upVector(0.f, 1.f, 0.f)
  //, m_projectionType(Orthographic)
  , m_projectionType(Perspective)
  , m_fieldOfView(45)
  , m_aspectRatio(1.f)
  , m_nearDist(.1f)
  , m_farDist(50.f)
  , m_windowAspectRatio(1.f)
  , m_eyeSeparationAngle(8.f)
{
  updateCamera();
  updateFrustum();
  invalidViewMatrix();
  invalidProjectionMatrix();
}

Z3DCamera::~Z3DCamera()
{
}

void Z3DCamera::setFieldOfView(float fov)
{
  m_fieldOfView = glm::clamp(fov, 10.f, 170.f);
  updateFrustum();
  invalidProjectionMatrix();
}

void Z3DCamera::setAspectRatio(float ar)
{
  m_aspectRatio = ar;
  updateFrustum();
  invalidProjectionMatrix();
}

void Z3DCamera::setNearDist(float nd)
{
  m_nearDist = nd;
  updateFrustum();
  invalidProjectionMatrix();
}

void Z3DCamera::setWindowAspectRatio(float war)
{
  m_windowAspectRatio = war;
  updateFrustum();
  invalidProjectionMatrix();
}

void Z3DCamera::setEyeSeparationAngle(float degree)
{
  m_eyeSeparationAngle = glm::clamp(degree, 1.f, 80.f);
  updateCamera();
  invalidStereoProjectionMatrix();
  invalidStereoViewMatrix();
}

void Z3DCamera::setCamera(glm::vec3 eye, glm::vec3 center, glm::vec3 upVector)
{
  m_eye = eye;
  m_center = center;
  m_upVector = glm::normalize(upVector);
  updateCamera();
  invalidViewMatrix();
}

void Z3DCamera::setFrustum(float fov, float ratio, float nearDist, float farDist)
{
  m_fieldOfView = fov;
  m_aspectRatio = ratio;
  m_nearDist = nearDist;
  m_farDist = farDist;
  updateFrustum();
  invalidProjectionMatrix();
}

void Z3DCamera::resetCamera(const std::vector<double> &bound, ResetCameraOptions options)
{
  glm::vec3 center;
  center[0] = (bound[0] + bound[1])/2.0;
  center[1] = (bound[2] + bound[3])/2.0;
  center[2] = (bound[4] + bound[5])/2.0;

  if (!(options & PreserveCenterDistance)) {
    float w1 = bound[1] - bound[0];
    float w2 = bound[3] - bound[2];
    float w3 = bound[5] - bound[4];
    w1 *= w1;
    w2 *= w2;
    w3 = 0;
    //w3 *= w3;
    float radius = w1 + w2 + w3;
    radius = (radius==0)?(1.0):(radius);

    // compute the radius of the enclosing sphere
    //radius = std::sqrt(radius)*0.5 + m_eyeSeparation/2.f;
    radius = std::sqrt(radius)*0.5;

    // (from VTK) compute the distance from the intersection of the view frustum with the
    // bounding sphere. Basically in 2D draw a circle representing the bounding
    // sphere in 2D then draw a horizontal line going out from the center of
    // the circle. That is the camera view. Then draw a line from the camera
    // position to the point where it intersects the circle. (it will be tangent
    // to the circle at this point, this is important, only go to the tangent
    // point, do not draw all the way to the view plane). Then draw the radius
    // from the tangent point to the center of the circle. You will note that
    // this forms a right triangle with one side being the radius, another being
    // the target distance for the camera, then just find the target dist using
    // a sin.
    double angle = glm::radians(m_fieldOfView);
    if (m_aspectRatio < 1.0) {  // use horizontal angle to calculate
      angle = 2.0*std::atan(std::tan(angle*0.5)*m_aspectRatio);
    }

    //m_centerDist = radius/std::sin(angle*0.5);
    m_centerDist = radius/std::sin(angle*0.5) + (bound[5] - bound[4]) / 2.0;
  }
  if (!(options & PreserveViewVector)) {
    m_viewVector = glm::vec3(0.f, 0.f, 1.f);
    m_upVector = glm::vec3(0.f, -1.f, 0.f);
  }
  glm::vec3 eye = center - m_centerDist*m_viewVector;
  setCamera(eye, center, m_upVector);

  resetCameraNearFarPlane(bound);
}

void Z3DCamera::resetCamera(double xmin, double xmax, double ymin, double ymax, double zmin, double zmax,
                            ResetCameraOptions options)
{
  std::vector<double> bound(6);
  bound[0] = xmin;
  bound[1] = xmax;
  bound[2] = ymin;
  bound[3] = ymax;
  bound[4] = zmin;
  bound[5] = zmax;
  resetCamera(bound, options);
}

void Z3DCamera::resetCameraNearFarPlane(const std::vector<double> &bound)
{
  double a = m_viewVector[0];
  double b = m_viewVector[1];
  double c = m_viewVector[2];
  double d = -(a*m_eye[0] + b*m_eye[1] + c*m_eye[2]);

  // Set the max near clipping plane and the min far clipping plane
  double range[2];
  range[0] = a*bound[0] + b*bound[2] + c*bound[4] + d;
  range[1] = 1e-18;

  // Find the closest / farthest bounding box vertex
  for (int k = 0; k < 2; k++ ) {
    for (int j = 0; j < 2; j++ ) {
      for (int i = 0; i < 2; i++ ) {
        double dist = a*bound[i] + b*bound[2+j] + c*bound[4+k] + d;
        range[0] = (dist<range[0])?(dist):(range[0]);
        range[1] = (dist>range[1])?(dist):(range[1]);
      }
    }
  }

  // Do not let the range behind the camera throw off the calculation.
  if (range[0] < 0.0) {
    range[0] = 0.0;
  }

  // Give ourselves a little breathing room
  range[0] = 0.99*range[0] - (range[1] - range[0])*0.5;
  range[1] = 1.01*range[1] + (range[1] - range[0])*0.5;

  // Make sure near is not bigger than far
  //range[0] = (range[0] >= range[1])?(0.01*range[1]):(range[0]);

  // Make sure near is at least some fraction of far - this prevents near
  // from being behind the camera or too close in front.
  double nearClippingPlaneTolerance = 0.001;

  // make sure the front clipping range is not too far from the far clippnig
  // range, this is to make sure that the zbuffer resolution is effectively
  // used
  if (range[0] < nearClippingPlaneTolerance*range[1]) {
    range[0] = nearClippingPlaneTolerance*range[1];
  }

  m_nearDist = range[0];
  m_farDist = range[1];
  updateFrustum();
  invalidProjectionMatrix();
}

void Z3DCamera::resetCameraNearFarPlane(double xmin, double xmax, double ymin, double ymax, double zmin, double zmax)
{
  std::vector<double> bound(6);
  bound[0] = xmin;
  bound[1] = xmax;
  bound[2] = ymin;
  bound[3] = ymax;
  bound[4] = zmin;
  bound[5] = zmax;
  resetCameraNearFarPlane(bound);
}

glm::mat4 Z3DCamera::getViewMatrix(Z3DEye eye)
{
  if (!m_viewMatricesIsValid[eye]) {
    glm::vec3 adjust(0.f,0.f,0.f);
    if (eye == LeftEye)
      adjust = m_strafeVector * -m_eyeSeparation/2.f;
    else if (eye == RightEye)
      adjust = m_strafeVector * m_eyeSeparation/2.f;
    m_viewMatrices[eye] = glm::lookAt(m_eye+adjust, m_center+adjust, m_upVector);
    m_viewMatricesIsValid[eye] = true;
  }
  return m_viewMatrices[eye];
}

glm::mat4 Z3DCamera::getProjectionMatrix(Z3DEye eye)
{
  if (!m_projectionMatricesIsValid[eye]) {
    if (m_projectionType == Orthographic) {
      m_projectionMatrices[eye] = glm::ortho(m_left, m_right, m_bottom, m_top, m_nearDist, m_farDist);
    } else {
      if (eye == CenterEye) {
        m_projectionMatrices[eye] = glm::frustum(m_left, m_right, m_bottom, m_top, m_nearDist, m_farDist);
      } else if (eye == LeftEye) {
        float frustumShift = (m_eyeSeparation/2.f)*m_nearDist/m_centerDist;
        m_projectionMatrices[eye] = glm::frustum(m_left+frustumShift, m_right+frustumShift,
                                                 m_bottom, m_top, m_nearDist, m_farDist);
      } else {  //RightEye
        float frustumShift = (m_eyeSeparation/2.f)*m_nearDist/m_centerDist;
        m_projectionMatrices[eye] = glm::frustum(m_left-frustumShift, m_right-frustumShift,
                                                 m_bottom, m_top, m_nearDist, m_farDist);
      }
    }
    m_projectionMatricesIsValid[eye] = true;
  }
  return m_projectionMatrices[eye];
}

bool Z3DCamera::operator ==(const Z3DCamera &rhs) const
{
  return (m_eye == rhs.m_eye) &&
      (m_center == rhs.m_center) &&
      (m_upVector == rhs.m_upVector) &&
      (m_projectionType == rhs.m_projectionType) &&
      (m_fieldOfView == rhs.m_fieldOfView) &&
      (m_aspectRatio == rhs.m_aspectRatio) &&
      (m_nearDist == rhs.m_nearDist) &&
      (m_farDist == rhs.m_farDist) &&
      (m_windowAspectRatio == rhs.m_windowAspectRatio) &&
      (m_eyeSeparation == rhs.m_eyeSeparation);
}

bool Z3DCamera::operator !=(const Z3DCamera &rhs) const
{
  return !(*this == rhs);
}

void Z3DCamera::dolly(float value)
{
  if (value <= 0.f || (m_centerDist < .01f && value > 1.f))
    return;
  glm::vec3 pos = m_center - m_viewVector * (m_centerDist / value);
  float maxV = 1e15;
  if (std::abs(pos.x) < maxV && std::abs(pos.y) < maxV  && std::abs(pos.z) < maxV )
    setEye(pos);
}

void Z3DCamera::roll(float angle)
{
  glm::vec3 up = glm::rotate(glm::angleAxis(angle, m_viewVector), m_upVector);
  setUpVector(up);
}

void Z3DCamera::azimuth(float angle)
{
  glm::vec3 eye = m_eye - m_center;
  eye = glm::rotate(glm::angleAxis(angle, m_upVector), eye);
  eye += m_center;
  setEye(eye);
}

void Z3DCamera::yaw(float angle)
{
  glm::vec3 center = m_center - m_eye;
  center = glm::rotate(glm::angleAxis(angle, m_upVector), center);
  center += m_eye;
  setCenter(center);
}

void Z3DCamera::elevation(float angle)
{
  rotate(angle, -m_strafeVector);
}

void Z3DCamera::pitch(float angle)
{
  rotate(angle, m_strafeVector, m_eye);
}

void Z3DCamera::zoom(float factor)
{
  if (factor <= 0.f)
    return;
  setFieldOfView(m_fieldOfView / factor);
}

void Z3DCamera::rotate(float angle, glm::vec3 axis, glm::vec3 point)
{
  rotate(glm::angleAxis(angle, glm::normalize(axis)), point);
}

void Z3DCamera::rotate(glm::quat quat, glm::vec3 point)
{
  glm::vec3 eye = m_eye - point;
  eye = glm::rotate(quat, eye);
  eye += point;

  glm::vec3 center = m_center - point;
  center = glm::rotate(quat, center);
  center += point;

  glm::vec3 upVector = glm::rotate(quat, m_upVector);

  setCamera(eye, center, upVector);
}

void Z3DCamera::rotate(float angle, glm::vec3 axis)
{
  rotate(glm::angleAxis(angle, glm::normalize(axis)));
}

void Z3DCamera::rotate(glm::quat quat)
{
  glm::vec3 eye = m_eye - m_center;
  eye = glm::rotate(quat, eye);
  eye += m_center;

  glm::vec3 upVector = glm::rotate(quat, m_upVector);

  setCamera(eye, m_center, upVector);
}

glm::vec3 Z3DCamera::vectorEyeToWorld(glm::vec3 vec, Z3DEye eye)
{
  return glm::inverse(glm::mat3(getViewMatrix(eye))) * vec;
}

glm::vec3 Z3DCamera::vectorWorldToEye(glm::vec3 vec, Z3DEye eye)
{
  return glm::mat3(getViewMatrix(eye)) * vec;
}

glm::vec3 Z3DCamera::pointEyeToWorld(glm::vec3 pt, Z3DEye eye)
{
  return glm::applyMatrix(glm::inverse(getViewMatrix(eye)), pt);
}

glm::vec3 Z3DCamera::pointWorldToEye(glm::vec3 pt, Z3DEye eye)
{
  return glm::applyMatrix(getViewMatrix(eye), pt);
}

glm::vec3 Z3DCamera::worldToScreen(glm::vec3 wpt, glm::ivec4 viewport, Z3DEye eye)
{
  glm::vec4 clipSpacePos = getProjectionMatrix(eye) * getViewMatrix(eye) * glm::vec4(wpt, 1.f);
  if (clipSpacePos.w == 0.f)
    return glm::vec3(-1.f, -1.f, -1.f);
  glm::vec3 ndcSpacePos = glm::vec3(clipSpacePos.xyz()) / clipSpacePos.w;
  return ((ndcSpacePos + 1.f) / 2.f) * glm::vec3((float)viewport.z, (float)viewport.w, 1.f)
      + glm::vec3((float)viewport.x, (float)viewport.y, 0.f);
}

glm::vec3 Z3DCamera::screenToWorld(glm::vec3 spt, glm::ivec4 viewport, Z3DEye eye)
{
  return glm::unProject(spt, getViewMatrix(eye), getProjectionMatrix(eye), viewport);
}

void Z3DCamera::updateCamera()
{
  m_viewVector = glm::normalize(m_center - m_eye);
  m_centerDist = glm::length(m_center - m_eye);
  // make sure upVector is not parallel to viewVector
  if (std::abs(glm::dot(m_upVector, m_viewVector)) >= 0.9) {
    LWARN() << "Resetting view up since view plane normal is parallel";
    m_upVector = glm::cross(m_viewVector, glm::vec3(1.f, 0.f, 0.f));
    if (glm::dot(m_upVector,m_upVector) < 0.001)
      m_upVector = glm::cross(m_viewVector, glm::vec3(0.f, 1.f, 0.f));
    m_upVector = glm::normalize(m_upVector);
  }
  m_strafeVector = glm::cross(m_viewVector, m_upVector);
  m_eyeSeparation = 2.f * m_centerDist * std::tan(glm::radians(m_eyeSeparationAngle)/2.f);
}

void Z3DCamera::updateFrustum()
{
  float halfheight = std::tan(0.5f * glm::radians(m_fieldOfView)) * m_nearDist;
  m_top = halfheight;
  m_bottom = -halfheight;
  float halfwidth = halfheight * m_aspectRatio * m_windowAspectRatio;
  m_left = -halfwidth;
  m_right = halfwidth;
}
