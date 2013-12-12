#ifndef Z3DCAMERA_H
#define Z3DCAMERA_H

#include "zglmutils.h"
#include "z3dgl.h"
#include <map>
#include <QFlags>

//Z3DCamera is used for view transformation and projection transformation.

//During the view transformation, the eye, center, and upVector are used to
//generate the view matrix.
//During the projection transformation, the projectionType, nearDist, windowAspectRatio
//farDist, fieldOfView, and aspectRatio are used to generate projection matrix.
//The aspect ratio is the ratio of x (width) to y (height).

//The view matrix is returned by getViewMatrix().
//The projection matrix is returned by getProjectionMatrix().

class Z3DCamera
{
public:
  enum ProjectionType
  {
    Perspective, Orthographic
  };

  enum ___ResetCameraOption
  {
    ResetAll = 0x00,
    PreserveCenterDistance = 0x01,
    PreserveViewVector = 0x02
  };
  Q_DECLARE_FLAGS(ResetCameraOptions, ___ResetCameraOption)

  Z3DCamera();
  virtual ~Z3DCamera();

  glm::vec3 getEye() const { return m_eye; }
  void setEye(glm::vec3 eye) { m_eye = eye; updateCamera(); invalidViewMatrix(); }

  glm::vec3 getCenter() const { return m_center; }
  void setCenter(glm::vec3 center) { m_center = center; updateCamera(); invalidViewMatrix(); }

  // always return normalized vector
  glm::vec3 getUpVector() const { return m_upVector; }
  void setUpVector(glm::vec3 upVector) { m_upVector = glm::normalize(upVector); updateCamera(); invalidViewMatrix(); }

  ProjectionType getProjectionType() const { return m_projectionType; }
  void setProjectionType(ProjectionType pt) { m_projectionType = pt; invalidProjectionMatrix(); }
  bool isPerspectiveProjection() const { return m_projectionType == Perspective; }
  bool isOrthographicProjection() const { return m_projectionType == Orthographic; }

  float getFieldOfView() const { return m_fieldOfView; }
  void setFieldOfView(float fov);  // in degree

  float getAspectRatio() const { return m_aspectRatio; }
  void setAspectRatio(float ar);

  float getNearDist() const { return m_nearDist; }
  void setNearDist(float nd);

  float getFarDist() const { return m_farDist; }
  void setFarDist(float fd) { m_farDist = fd; invalidProjectionMatrix(); }

  float getWindowAspectRatio() const { return m_windowAspectRatio; }
  void setWindowAspectRatio(float war);

  float getEyeSeparationAngle() const { return m_eyeSeparationAngle; }
  void setEyeSeparationAngle(float degree);  // in degree

  // convinient functions to set many variables at once:

  // setCamera function will set eye, center, upVector and other derived values of camera based on input
  void setCamera(glm::vec3 eye, glm::vec3 center, glm::vec3 upVector);
  // setFrustum function will set fov, ratio, nearDist, farDist and other derived values of frustum based on input
  void setFrustum(float fov, float ratio, float nearDist, float farDist);

  // Automatically set up the camera based on a specified bounding box
  // (xmin,xmax, ymin,ymax, zmin,zmax). Camera will reposition itself so
  // that its focal point is the center of the bounding box, and adjust its
  // position (if PreserveCenterDistance is not set) and frustum to make sure everything inside bounding
  // box is visible. Initial view vector (vector defined from eye to center)
  // will be preserved based on the PreserveViewVector flag. By default it is not preserved and will be
  // reset to (0,0,1) (upVector will then be set to (0,-1,0)).
  // Result depends on current field of view and aspect ratio.
  void resetCamera(const std::vector<double> &bound, ResetCameraOptions options = ResetAll);
  void resetCamera(double xmin, double xmax, double ymin, double ymax, double zmin, double zmax,
                   ResetCameraOptions options = ResetAll);

  // Reset the camera near far plane based on the bounding box (see resetCamera).
  // This ensures that nothing is clipped by the near far planes
  void resetCameraNearFarPlane(const std::vector<double> &bound);
  void resetCameraNearFarPlane(double xmin, double xmax, double ymin, double ymax, double zmin, double zmax);

  glm::mat4 getViewMatrix(Z3DEye eye);
  glm::mat4 getProjectionMatrix(Z3DEye eye);

  // dist from eye to center
  float getCenterDist() const { return m_centerDist; }
  // normalized vector from eye to center
  glm::vec3 getViewVector() const { return m_viewVector; }
  // normalized vector represents right direction relative to the viewer's current orientation
  glm::vec3 getStrafeVector() const { return m_strafeVector; }
  // frustum
  float getLeft() const { return m_left; }
  float getRight() const { return m_right; }
  float getBottom() const { return m_bottom; }
  float getTop() const { return m_top; }

  // other matrix
  inline glm::mat3 getRotateMatrix(Z3DEye eye = CenterEye) { return glm::mat3(getViewMatrix(eye)); }

  bool operator==(const Z3DCamera& rhs) const;
  bool operator!=(const Z3DCamera& rhs) const;

  // Divide the camera's distance from the focal point by the given
  // dolly value.  Use a value greater than one to dolly-in toward
  // the focal point, and use a value less than one to dolly-out away
  // from the focal point.
  void dolly(float value);

  // Rotate the camera about the view vector.  This will
  // spin the camera about its axis. angle in degree
  void roll(float angle);

  // Rotate the camera about the view up vector centered at the focal point.
  // Note that the view up vector is whatever was set via setUpVector, and is
  // not necessarily perpendicular to the direction of projection.  The
  // result is a horizontal rotation of the camera. angle in degree
  void azimuth(float angle);

  // Description:
  // Rotate the focal point about the view up vector, using the camera's
  // position as the center of rotation. Note that the view up vector is
  // whatever was set via setUpVector, and is not necessarily perpendicular
  // to the view vector.  The result is a horizontal rotation
  // of the scene. angle in degree
  void yaw(float angle);
  inline void pan(float angle) { yaw(angle); }

  // Description:
  // Rotate the camera about the cross product of the view up vector and
  // the view vector (point at left in screen), using the focal point as
  // the center of rotation. The result is a vertical rotation of the scene.
  // angle in degree
  void elevation(float angle);

  // Description:
  // Rotate the focal point about the cross product of the view vector
  // and the view up vector (point right in screen), using the camera's position
  // as the center of rotation.  The result is a vertical rotation of the camera.
  // angle in degree
  void pitch(float angle);
  inline void tilt(float angle) { pitch(angle); }

  // In perspective mode, decrease the view angle by the specified factor.
  // A value greater than 1 is a zoom-in, a value less than 1 is a zoom-out.
  // angle in degree
  void zoom(float factor);

  // rotate around a point
  // angle in degree, axis and point in worldspace, axis should be normalized
  void rotate(float angle, glm::vec3 axis, glm::vec3 point);
  void rotate(glm::quat quat, glm::vec3 point);
  // rotate around center (focus point)
  void rotate(float angle, glm::vec3 axis);
  void rotate(glm::quat quat);

  // convert between eye space and world space
  glm::vec3 vectorEyeToWorld(glm::vec3 vec, Z3DEye eye = CenterEye);
  glm::vec3 vectorWorldToEye(glm::vec3 vec, Z3DEye eye = CenterEye);
  glm::vec3 pointEyeToWorld(glm::vec3 pt, Z3DEye eye = CenterEye);
  glm::vec3 pointWorldToEye(glm::vec3 pt, Z3DEye eye = CenterEye);

  // world to screen, if point is clipped, its screen coord will be (-1,-1,-1)
  glm::vec3 worldToScreen(glm::vec3 wpt, glm::ivec4 viewport, Z3DEye eye = CenterEye);
  glm::vec3 screenToWorld(glm::vec3 spt, glm::ivec4 viewport, Z3DEye eye = CenterEye);

protected:
  void invalidViewMatrix()
  {
    m_viewMatricesIsValid[LeftEye] = false;
    m_viewMatricesIsValid[CenterEye] = false;
    m_viewMatricesIsValid[RightEye] = false;
  }

  void invalidProjectionMatrix()
  {
    m_projectionMatricesIsValid[LeftEye] = false;
    m_projectionMatricesIsValid[CenterEye] = false;
    m_projectionMatricesIsValid[RightEye] = false;
  }

  void invalidStereoViewMatrix()
  {
    m_viewMatricesIsValid[LeftEye] = false;
    m_viewMatricesIsValid[RightEye] = false;
  }

  void invalidStereoProjectionMatrix()
  {
    m_projectionMatricesIsValid[LeftEye] = false;
    m_projectionMatricesIsValid[RightEye] = false;
  }

  void updateCamera();
  void updateFrustum();

private:
  glm::vec3 m_eye;
  glm::vec3 m_center;
  glm::vec3 m_upVector;  // normalized
  ProjectionType m_projectionType;
  float m_fieldOfView; // fov in degree
  float m_aspectRatio;
  float m_nearDist;
  float m_farDist;
  float m_windowAspectRatio;
  float m_eyeSeparation;  // dist from left eye to right eye
  float m_eyeSeparationAngle;  // angle between two eyes to focus point in degree

  // derived camera variables
  glm::vec3 m_viewVector;  // normalized vector from eye to center (center - eye)
  float m_centerDist; // distance from eye to center
  glm::vec3 m_strafeVector; // normalized vector point at right in eye space
  // derived frustum variables
  float m_left;
  float m_right;
  float m_bottom;
  float m_top;

  std::map<Z3DEye,glm::mat4> m_viewMatrices;
  std::map<Z3DEye,bool> m_viewMatricesIsValid;
  std::map<Z3DEye,glm::mat4> m_projectionMatrices;
  std::map<Z3DEye,bool> m_projectionMatricesIsValid;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Z3DCamera::ResetCameraOptions)

#endif // Z3DCAMERA_H
