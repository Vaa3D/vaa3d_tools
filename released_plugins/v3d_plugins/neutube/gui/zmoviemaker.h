#ifndef ZMOVIEMAKER_H
#define ZMOVIEMAKER_H

#include <string>
#include <vector>
#ifdef __GLIBCXX__
#include <tr1/memory>
#else
#include <memory>
#endif

#include "zmoviescript.h"
#include "zmovieactor.h"
#include "zmoviescene.h"
#include "zmoviephotographer.h"
#include "zmoviesceneclipper.h"

class ZStackDoc;

class ZMovieMaker
{
public:
  ZMovieMaker();
  ~ZMovieMaker();

public:
  void recruitCast();
  void dismissCast();
  void prepareStage();

  //Make a movie under <filePath>. It returns the number of frames created.
  int make(const std::string &filePath);

  static std::string getFramePath(const std::string &dirPath, int index);

  void makeSlideShow(const std::string &filePath);

  //Update action.
  //Return value:
  //  0: no action left
  //  1: background scene
  //  2: normal scene
  int updateAction();

  //Update scene
  //Return value:
  //  0: no action left
  //  1: background scene
  //  2: normal scene
  int updateScene();

  void setupAction(const ZMovieScene &scene);
  void act(int timeStep);
  ZStackDoc *getAcademy();
  ZMovieActor* getActor(std::string id);

  inline int getMovieWidth() { return m_width; }
  inline int getMovieHeight() { return m_height; }

  inline double getFrameInterval() { return m_frameInterval; }
  inline void setFrameInterval(int t) { m_frameInterval = t; }

  inline void setScript(const ZMovieScript script) {
    m_script = script;
  }

  void printSummary();

  inline int getActionTimeStep() { return 10; }
  inline void setFrameSize(int width, int height) {
    m_width = width;
    m_height = height;
  }

  inline void setBackgroundColor(int r, int g, int b) {
    m_backgroundColor.setRgb(r, g, b);
  }

private:
  std::tr1::shared_ptr<ZStackDoc> m_academy;
  ZMovieStage *m_stage;
  ZMovieSceneClipperState m_clipperState; //To store double values for integer parameters
  std::vector<ZMovieActor*> m_cast;
  ZMovieScript m_script;
  ZMoviePhotographer m_photographer;
  int m_width;
  int m_height;
  int m_frameInterval; //Always use interger value if possible
  QColor m_backgroundColor;
};

#endif // ZMOVIEMAKER_H
