#include "zmoviecamera.h"

#include <iostream>

#include "zjsonparser.h"
#include "z3dwindow.h"

using namespace std;

ZMovieCamera::ZMovieCamera()
{
  m_movingDirection[0] = VIEW_AXIS;
  m_movingDirection[1] = DEFINED_AXIS;
  m_hasReset[0] = false;
  m_hasReset[1] = false;
  m_hasReset[2] = false;
}

void ZMovieCamera::loadJsonObject(const ZJsonObject &obj)
{
  map<string, json_t*> entryMap = obj.toEntryMap();
  if (entryMap.count("rotate") > 0) {
    json_t* rotateObj = entryMap["rotate"];
    if (ZJsonParser::isObject(rotateObj)) {
      m_rotation.loadJsonObject(ZJsonObject(rotateObj, false));
    } else {
      cerr << "Invalid rotate object. Skip." << endl;
    }
  }

  if (entryMap.count("move") > 0) {
    ZJsonObject moveObj = ZJsonObject(entryMap["move"], false);
    if (!moveObj.isEmpty()) {
      json_t *value = moveObj["center"];
      if (value != NULL) {
        if (ZJsonParser::isArray(value)) {
          m_movingDirection[1] = DEFINED_AXIS;
          for (int k = 0; k < 3; ++k) {
            m_movingSpeed[1][k] = ZJsonParser::numberValue(value, k);
          }
        } else {
          m_movingDirection[1] = VIEW_AXIS;
          m_movingSpeed[1][0] = ZJsonParser::numberValue(value);
        }
      }

      value = moveObj["eye"];
      if (value != NULL) {
        if (ZJsonParser::isArray(value)) {
          m_movingDirection[0] = DEFINED_AXIS;
          for (int k = 0; k < 3; ++k) {
            m_movingSpeed[0][k] = ZJsonParser::numberValue(value, k);
          }
        } else {
          m_movingDirection[0] = VIEW_AXIS;
          m_movingSpeed[0][0] = ZJsonParser::numberValue(value);
        }
      }

      value = moveObj["up_vector"];
      if (value != NULL) {
        if (ZJsonParser::isArray(value)) {
          for (int k = 0; k < 3; ++k) {
            m_movingSpeed[2][k] = ZJsonParser::numberValue(value, k);
          }
        }
      }
    } else {
      cerr << "Invalid move object. Skip." << endl;
    }
  }

  if (entryMap.count("reset") > 0) {
    ZJsonObject resetObj = ZJsonObject(entryMap["reset"], false);
    if (!resetObj.isEmpty()) {
      json_t *value = resetObj["center"];
      if (value != NULL) {
        if (ZJsonParser::isArray(value)) {
          m_hasReset[1] = true;
          m_resetPosition[1].set(ZJsonParser::numberValue(value, 0),
                                 ZJsonParser::numberValue(value, 1),
                                 ZJsonParser::numberValue(value, 2));
        } else {
#ifdef _DEBUG_
          ZJsonParser::print("center", value, 0);
#endif
          cerr << "Invalid center object. Skip." << endl;
        }
      }

      value = resetObj["eye"];
      if (value != NULL) {
        if (ZJsonParser::isArray(value)) {
          m_hasReset[0] = true;
          m_resetPosition[0].set(ZJsonParser::numberValue(value, 0),
                                 ZJsonParser::numberValue(value, 1),
                                 ZJsonParser::numberValue(value, 2));
        } else {
          cerr << "Invalid center object. Skip." << endl;
        }
      }

      value = resetObj["up_vector"];
      if (value != NULL) {
        if (ZJsonParser::isArray(value)) {
          m_hasReset[2] = true;
          m_resetPosition[2].set(ZJsonParser::numberValue(value, 0),
                                 ZJsonParser::numberValue(value, 1),
                                 ZJsonParser::numberValue(value, 2));
        } else {
          cerr << "Invalid center object. Skip." << endl;
        }
      }
    } else{
      cerr << "Invalid reset object. Skip." << endl;
    }
  }
}

void ZMovieCamera::print() const
{
  m_rotation.print();
  cout << "Moving speed: " << m_movingSpeed << endl;
}

bool ZMovieCamera::hasReset(EControlPoint ref) const
{
  switch (ref) {
  case EYE:
    return m_hasReset[0];
  case CENTER:
    return m_hasReset[1];
  case UP_VECTOR:
    return m_hasReset[2];
  }

  return m_hasReset[0];  //To supress warning
}

void ZMovieCamera::reset(Z3DWindow *stage)
{
  if (hasReset(EYE)) {
    stage->getCamera()->setEye(glm::vec3(m_resetPosition[0].x(),
                                         m_resetPosition[0].y(),
                                         m_resetPosition[0].z()));
  }

  if (hasReset(CENTER)) {
    stage->getCamera()->setCenter(glm::vec3(m_resetPosition[1].x(),
                                            m_resetPosition[1].y(),
                                            m_resetPosition[1].z()));
  }

  if (hasReset(UP_VECTOR)) {
    stage->getCamera()->setUpVector(glm::vec3(m_resetPosition[2].x(),
                                              m_resetPosition[2].y(),
                                              m_resetPosition[2].z()));
  }
}

const ZPoint& ZMovieCamera::getMovingVelocity(EControlPoint ref) const
{
  switch (ref) {
  case EYE:
    return m_movingSpeed[0];
  case CENTER:
    return m_movingSpeed[1];
  case UP_VECTOR:
    return m_movingSpeed[2];
  default:
    break;
  }

  return m_movingSpeed[0]; //To supress warning
}

double ZMovieCamera::getMovingSpeed(EControlPoint ref) const
{
  switch (ref) {
  case EYE:
    if (m_movingDirection[0] == DEFINED_AXIS) {
      return m_movingSpeed[0].length();
    } else {
      return m_movingSpeed[0].x();
    }
    break;
  case CENTER:
    if (m_movingDirection[1] == DEFINED_AXIS) {
      return m_movingSpeed[1].length();
    } else {
      return m_movingSpeed[1].x();
    }
    break;
  case UP_VECTOR:
    return m_movingSpeed[2].length();
  default:
    break;
  }

  return 0.0; //To supress warning
}

ZMovieCamera::EMovingDirection ZMovieCamera::getMovingDirection(
    EControlPoint ref) const
{
  switch (ref) {
  case EYE:
    return m_movingDirection[0];
  case CENTER:
    return m_movingDirection[1];
  default:
    break;
  }

  return m_movingDirection[0]; //To supress warning
}
