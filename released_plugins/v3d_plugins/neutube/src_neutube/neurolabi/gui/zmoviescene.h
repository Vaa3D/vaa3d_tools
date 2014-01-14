#ifndef ZMOVIESCENE_H
#define ZMOVIESCENE_H

#include <string>
#include <vector>
#include <utility>
#include "zpoint.h"
#include "zjsonobject.h"
#include "z3drotation.h"
#include "zmoviesceneclipper.h"
#include "zmoviecamera.h"

struct MovieAction {
  std::string actorId;
  bool isVisible;
  bool settingColor;
  double red;
  double green;
  double blue;
  bool settingAlpha;
  double alpha;
  ZPoint movingOffset;
  double fadingFactor;
  double transitFactor[3];

  MovieAction() : isVisible(false), settingColor(false),
    red(0), green(0), blue(0), settingAlpha(false), alpha(1.0),
    fadingFactor(0.0) {
    transitFactor[0] = 0.0;
    transitFactor[1] = 0.0;
    transitFactor[2] = 0.0;
  }
};

class ZStackDoc;
class Z3DWindow;

class ZMovieScene
{
public:
  ZMovieScene();
  bool toSingleShot(double dt, ZMovieScene *scene);
  inline void setDuration(double duration) {
    m_duration = duration;
  }
  inline double getDuration() const {
    return m_duration;
  }

  inline size_t getActionNumber() const { return m_actionList.size(); }
  inline const MovieAction& getAction(size_t index) const {
    return m_actionList[index];
  }

  void loadJsonObject(const ZJsonObject &obj);

  inline void addAction(const MovieAction &action) {
    m_actionList.push_back(action);
  }

  /*
  inline void setCameraRotation(const ZPoint &axis, double rot) {
    m_cameraRotation.setAxis(axis);
    m_cameraRotation.setAngle(rot);
  }

  inline const Z3DRotation& getCameraRotation() const {
    return m_cameraRotation;
  }
  */

  void print() const;

  inline bool isSingleShot() { return m_duration == 0; }

  void updateCamera(Z3DWindow *stage, double t);
  void updateClip(Z3DWindow *stage, ZMovieSceneClipperState *state, double t);

  inline bool isNewScene() const { return m_isNewScene; }
  inline void setNewScene(bool isNew) {
    m_isNewScene = isNew;
  }

  inline bool isBackground() const { return m_isBackground; }
  inline void setBackground(bool bg) { m_isBackground = bg; }

private:
  bool isDurationTag(const char *tag);
  bool isBackgroundTag(const char *tag);
  bool isActionListTag(const char *tag);
  bool isIdTag(const char *tag);
  bool isMovingTag(const char *tag);
  bool isMovingToTag(const char *tag);
  bool isVisibleTag(const char *tag);
  bool isColorTag(const char *tag);
  bool isAlphaTag(const char*tag);
  bool isCameraTag(const char *tag);
  bool isClipperTag(const char *tag);
  bool isFadingTag(const char *tag);
  bool isTransitTag(const char *tag);

private:
  std::vector<MovieAction> m_actionList;
  ZMovieCamera m_camera;
  //Z3DRotation m_cameraRotation;
  std::vector<ZMovieSceneClipper> m_clipperArray;

  int m_duration;
  bool m_isNewScene;
  bool m_isBackground;
};

#endif // ZMOVIESCENE_H
