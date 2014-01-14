#include "zmoviescene.h"

#include <map>

#include "zstackdoc.h"
#include "z3dwindow.h"
#include "zjsonparser.h"
#include "tz_error.h"
#include "z3dinteractionhandler.h"
#include "zmoviecamera.h"

using namespace std;

ZMovieScene::ZMovieScene() : m_duration(0), m_isNewScene(true),
  m_isBackground(false)
{
}
/*
bool ZMovieScene::toSingleShot(double dt, ZMovieScene *scene)
{
  if (dt > m_duration) {
    return false;
  }

  if (m_duration > 0.0) {
    scene->setCameraRotation(m_cameraRotation.getAxis(),
                            m_cameraRotation.getAngle() / m_duration);
  }

  scene->setDuration(0);

  *scene = *this;

  return true;
}
*/
bool ZMovieScene::isDurationTag(const char *tag)
{
  return strcmp(tag, "duration") == 0 || strcmp(tag, "Duration") == 0;
}

bool ZMovieScene::isBackgroundTag(const char *tag)
{
  return strcmp(tag, "background") == 0 || strcmp(tag, "Background") == 0;
}

bool ZMovieScene::isActionListTag(const char *tag)
{
  return strcmp(tag, "actionList") == 0 || strcmp(tag, "ActionList") == 0 ||
      strcmp(tag, "action") == 0 || strcmp(tag, "Action") == 0;
}

bool ZMovieScene::isIdTag(const char *tag)
{
  return strcmp(tag, "id") == 0 || strcmp(tag, "Id") == 0;
}

bool ZMovieScene::isMovingTag(const char *tag)
{
  return strcmp(tag, "moving") == 0 || strcmp(tag, "Moving") == 0 ||
      strcmp(tag, "move") == 0 || strcmp(tag, "Move") == 0;
}

bool ZMovieScene::isFadingTag(const char *tag)
{
  return eqstr(tag, "fade") || eqstr(tag, "Fade");
}

bool ZMovieScene::isTransitTag(const char *tag)
{
  return eqstr(tag, "transit") || eqstr(tag, "Transit");
}

bool ZMovieScene::isMovingToTag(const char *tag)
{
  return strcmp(tag, "movingTo") == 0 || strcmp(tag, "MovingTo") == 0 ||
      strcmp(tag, "moveTo") == 0 || strcmp(tag, "MoveTo") == 0;
}

bool ZMovieScene::isVisibleTag(const char *tag)
{
  return strcmp(tag, "visible") == 0 || strcmp(tag, "Visible") == 0 ||
      strcmp(tag, "isVisible") == 0 || strcmp(tag, "IsVisible") == 0;
}

bool ZMovieScene::isColorTag(const char *tag)
{
  return strcmp(tag, "color") == 0 || strcmp(tag, "Color") == 0;
}

bool ZMovieScene::isAlphaTag(const char *tag)
{
  return strcmp(tag, "alpha") == 0 || strcmp(tag, "Alpha") == 0;
}

bool ZMovieScene::isCameraTag(const char *tag)
{
  return strcmp(tag, "camera") == 0 || strcmp(tag, "Camera") == 0;
}

bool ZMovieScene::isClipperTag(const char *tag)
{
  return strcmp(tag, "clipper") == 0 || strcmp(tag, "Clipper") == 0;
}

void ZMovieScene::loadJsonObject(const ZJsonObject &obj)
{
  m_actionList.clear();

  map<string, json_t*> entryMap = obj.toEntryMap();
  for (map<string, json_t*>::const_iterator iter = entryMap.begin();
       iter != entryMap.end(); ++iter) {
#ifdef _DEBUG_2
    cout << "Scene key: " << iter->first << endl;
#endif
    if (isDurationTag(iter->first.c_str())) {
      setDuration(ZJsonParser::numberValue(iter->second));
    } else if (isBackgroundTag(iter->first.c_str())) {
      setBackground(ZJsonParser::booleanValue(iter->second));
    } else if (isActionListTag(iter->first.c_str())) {
      ZJsonArray actionList;
      TZ_ASSERT(ZJsonParser::isArray(iter->second), "array");

      actionList.set(iter->second, false);
      for (size_t index = 0; index < actionList.size(); ++index) {
        ZJsonObject actionObject(actionList.at(index), false);
        map<string, json_t*> actionEntry = actionObject.toEntryMap();
        MovieAction action;

        for (std::map<string, json_t*>::const_iterator actionIter = actionEntry.begin();
             actionIter != actionEntry.end(); ++actionIter) {
          if (isIdTag(actionIter->first.c_str())) {
            action.actorId = ZJsonParser::stringValue(actionIter->second);
          } else if (isVisibleTag(actionIter->first.c_str())) {
            action.isVisible = ZJsonParser::booleanValue(actionIter->second);
          } else if (isMovingTag(actionIter->first.c_str())) {
            action.movingOffset.setX(
                  ZJsonParser::numberValue(actionIter->second, 0));
            action.movingOffset.setY(
                  ZJsonParser::numberValue(actionIter->second, 1));
            action.movingOffset.setZ(
                  ZJsonParser::numberValue(actionIter->second, 2));
          } else if (isColorTag(actionIter->first.c_str())) {
            action.settingColor = true;
            action.red = ZJsonParser::numberValue(actionIter->second, 0);
            action.green = ZJsonParser::numberValue(actionIter->second, 1);
            action.blue = ZJsonParser::numberValue(actionIter->second, 2);
          } else if (isAlphaTag(actionIter->first.c_str())) {
            action.settingAlpha = true;
            action.alpha = ZJsonParser::numberValue(actionIter->second);
          } else if (isFadingTag(actionIter->first.c_str())) {
            action.fadingFactor = ZJsonParser::numberValue(actionIter->second);
          } else if (isTransitTag(actionIter->first.c_str())) {
            if (ZJsonParser::isNumber(actionIter->second)) {
              action.transitFactor[0] = ZJsonParser::numberValue(actionIter->second);
              action.transitFactor[1] = action.transitFactor[0];
              action.transitFactor[2] = action.transitFactor[1];
            } else if (ZJsonParser::isArray(actionIter->second)){
              action.transitFactor[0] = ZJsonParser::numberValue(actionIter->second, 0);
              action.transitFactor[1] = ZJsonParser::numberValue(actionIter->second, 1);
              action.transitFactor[2] = ZJsonParser::numberValue(actionIter->second, 2);
            }
          }
        }
        addAction(action);
      }
    } else if (isCameraTag(iter->first.c_str())) {
      ZJsonObject cameraObject(iter->second, false);
      m_camera.loadJsonObject(cameraObject);
      /*
      json_t *rotationObj = cameraObject["rotate"];
      if (rotationObj != NULL) {
        m_cameraRotation.loadJsonObject(ZJsonObject(rotationObj, false));
      }
      */
    } else if (isClipperTag(iter->first.c_str())) {
      for (size_t i = 0; i < ZJsonParser::arraySize(iter->second); ++i) {
        ZJsonObject clipperObject(ZJsonParser::arrayValue(iter->second, i), false);
        ZMovieSceneClipper clipper;
        clipper.loadJsonObject(clipperObject);
        m_clipperArray.push_back(clipper);
      }
    }
  }
}

void ZMovieScene::print() const
{
  cout << "Duration: " << m_duration << endl;
  cout << m_actionList.size() << " actions:" << endl;
  for (size_t i = 0; i < m_actionList.size(); ++i) {
    cout << "  " << m_actionList[i].actorId << ": " << m_actionList[i].isVisible << endl;
    cout << "  Moving: " << m_actionList[i].movingOffset.toString() << endl;
  }
  cout << "Camera: " << endl;
  m_camera.print();

  cout << "Clipper: " << m_clipperArray.size() << endl;
  for (size_t i = 0; i < m_clipperArray.size(); ++i) {
    m_clipperArray[i].print();
  }
}

void ZMovieScene::updateCamera(Z3DWindow *stage, double t)
{
  if (isNewScene()) {
    m_camera.reset(stage);
  }

  const Z3DRotation& rotation = m_camera.getRotation();
  if (rotation.getAngle() != 0.0) {
    glm::vec3 axis(rotation.getAxis().x(), rotation.getAxis().y(),
                   rotation.getAxis().z());
    //stage->getInteractionHandler()->getTrackball()->rotate(
    //      axis, rotation.getAngle() * t);
    stage->getCamera()->rotate(glm::degrees(rotation.getAngle() * t),
                               stage->getCamera()->vectorEyeToWorld(
                                 glm::normalize(axis)));
  }

  //Move eye
  glm::vec3 direction = stage->getCamera()->getViewVector();
  const ZPoint &eyeMovingSpeed = m_camera.getMovingVelocity(ZMovieCamera::EYE);
  glm::vec3 offset;
  if (m_camera.getMovingDirection(ZMovieCamera::EYE) == ZMovieCamera::VIEW_AXIS) {
    double dist = m_camera.getMovingSpeed(ZMovieCamera::EYE) * t;
    offset = glm::vec3(direction[0] * dist, direction[1] * dist,
        direction[2] * dist);
  } else {
    offset = glm::vec3(eyeMovingSpeed.x() * t, eyeMovingSpeed.y() * t,
                       eyeMovingSpeed.z() * t);
  }

  stage->getCamera()->setEye(stage->getCamera()->getEye() + offset);

  //Move center
  const ZPoint &centerMovingSpeed = m_camera.getMovingVelocity(ZMovieCamera::CENTER);
  if (m_camera.getMovingDirection(ZMovieCamera::CENTER) == ZMovieCamera::VIEW_AXIS) {
    double dist = m_camera.getMovingSpeed(ZMovieCamera::CENTER) * t;
    offset = glm::vec3(direction[0] * dist, direction[1] * dist,
        direction[2] * dist);
  } else {
    offset = glm::vec3(centerMovingSpeed.x() * t, centerMovingSpeed.y() * t,
                       centerMovingSpeed.z() * t);
  }

  stage->getCamera()->setCenter(stage->getCamera()->getCenter() + offset);

  //Move up_vector
  const ZPoint &uvMovingSpeed = m_camera.getMovingVelocity(ZMovieCamera::UP_VECTOR);
  offset = glm::vec3(uvMovingSpeed.x() * t, uvMovingSpeed.y() * t,
                     uvMovingSpeed.z() * t);

  stage->getCamera()->setUpVector(stage->getCamera()->getUpVector() + offset);
}

void ZMovieScene::updateClip(Z3DWindow *stage,
                             ZMovieSceneClipperState *state, double t)
{
  for (vector<ZMovieSceneClipper>::const_iterator iter = m_clipperArray.begin();
       iter != m_clipperArray.end(); ++iter) {
    if (isNewScene()) {
      iter->reset(stage, state);
    } else {
      iter->clip(stage, state, t);
    }
  }
}
