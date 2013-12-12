#include <iostream>

#include "zmoviescript.h"
#include "zjsonparser.h"
#include "zmoviecamera.h"

using namespace std;

ZMovieScript::ZMovieScript() : m_currentTime(-1)
{
}

ZMovieScene* ZMovieScript::start()
{
  if (m_plot.empty()) {
    return NULL;
  }

  m_currentTime = 0;

  setCurrentScene(m_plot[0]);

  return getCurrentScene();
}

ZMovieScene* ZMovieScript::nextScene(int dt)
{
  if (m_currentTime < 0) { //The first scene
    start();
  } else {
    int time = 0;
    m_currentTime += dt; //Increase current time
    //ZMovieScene currentScene;
    bool isNewScene = false;
    for (std::vector<ZMovieScene>::const_iterator iter = m_plot.begin();
         iter != m_plot.end(); ++iter) {
      m_currentScene = *iter;
      time += iter->getDuration();
      if (time >= m_currentTime) {
        break;
      }
    }

    if (time - m_currentScene.getDuration() + dt == m_currentTime) {
      isNewScene = true;
    }

    m_currentScene.setNewScene(isNewScene);

    if (time < m_currentTime) {
      return NULL;
    }
  }

  return getCurrentScene();
}

void ZMovieScript::addActor(const std::string &id, const std::string &source)
{
  if (m_cast.count(id) > 0) {
    cout << "WARNING: The actor " << id << " already exists." << endl;
  }
  m_cast[id] = source;
}

bool ZMovieScript::loadScript(const std::string &filePath)
{
  m_cast.clear();
  m_plot.clear();

  ZJsonObject movieObject;
  if (movieObject.load(filePath)) {
    ZJsonArray sceneArray;
    sceneArray.set(movieObject["plot"], false);
    ZJsonArray castArray;
    castArray.set(movieObject["cast"], false);

    for (size_t i = 0; i < castArray.size(); ++i) {
      ZJsonObject castObject(castArray.at(i), false);
      m_cast[ZJsonParser::stringValue(castObject["id"])] =
          ZJsonParser::stringValue(castObject["source"]);
    }


    for (size_t i = 0; i < sceneArray.size(); ++i) {
      ZMovieScene scene;
      scene.loadJsonObject(ZJsonObject(sceneArray.at(i), false));
#ifdef _DEBUG_
      scene.print();
#endif
      addScene(scene);
    }

    return true;
  }

  return false;
}

void ZMovieScript::printSummary() const
{
  cout << "Movie script" << endl;
  cout << "  " << m_cast.size() << " actors" << endl;
  cout << "  " << m_plot.size() << " scenes" << endl;
}

void ZMovieScript::print() const
{
  for (map<std::string, std::string>::const_iterator iter = m_cast.begin();
       iter != m_cast.end(); ++iter) {
    cout << iter->first << ": " << iter->second << endl;
  }

  for (size_t i = 0; i < m_plot.size(); ++i) {
    m_plot[i].print();
  }
}
