#ifndef ZMOVIESCRIPT_H
#define ZMOVIESCRIPT_H

#include <vector>
#include <string>
#include <map>
#include "zmoviescene.h"

class ZMovieScript
{
public:
  ZMovieScript();

public:
  ZMovieScene* start();
  ZMovieScene* nextScene(int dt);
  void setCurrentScene(const ZMovieScene &scene) {
    m_currentScene = scene;
  }

  inline ZMovieScene* getCurrentScene() {
    return &m_currentScene;
  }

  std::string getActorSource(std::string id) {
    return m_cast[id];
  }

  const std::map<std::string, std::string>& getCast() const{
    return m_cast;
  }

  inline void addScene(const ZMovieScene &scene) {
    m_plot.push_back(scene);
  }

  bool loadScript(const std::string &filePath);

  void addActor(const std::string &id, const std::string &source);

  void printSummary() const;
  void print() const;

  inline const std::vector<ZMovieScene>& getPlot() const {
    return m_plot;
  }

  inline bool isStarted() const {
    return m_currentTime >= 0;
  }

private:
  int m_currentTime;
  std::map<std::string, std::string> m_cast;
  std::vector<ZMovieScene> m_plot;
  ZMovieScene m_currentScene;

};

#endif // ZMOVIESCRIPT_H
