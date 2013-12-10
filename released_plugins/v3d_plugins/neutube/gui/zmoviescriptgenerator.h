#ifndef ZMOVIESCRIPTGENERATOR_H
#define ZMOVIESCRIPTGENERATOR_H

#include <map>
#include <vector>
#include <string>
#include <ostream>

class ZMovieScriptGenerator
{
public:
  ZMovieScriptGenerator();

  void writeCast(std::ostream &stream, int indentLevel);
  void writeAction(std::ostream &stream,
                   const std::vector<std::string> &actions, int indentLevel);
  void writeAction(std::ostream &stream, const std::string &id,
                   const std::vector<std::string> &actions, int indentLevel);
  void writeAction(std::ostream &stream, const std::vector<std::string> &id,
                   const std::vector<std::string> &actions, int indentLevel);
  void writeShowAction(std::ostream &stream, int indentLevel);
  void addActor(const std::string &id, const std::string &source);

  void writePlotStart(std::ostream &stream, int indentLevel);
  void writePlotEnd(std::ostream &stream, int indentLevel);
  void writeDuration(std::ostream &stream, int duration, int indentLevel);
  void writeActionStart(std::ostream &stream, int indentLevel);
  void writeActionEnd(std::ostream &stream, int indentLevel);
  void writeSceneStart(std::ostream &stream, int indentLevel);
  void writeSceneEnd(std::ostream &stream, int indentLevel);

  void writeCameraStart(std::ostream &stream, int indentLevel);
  void writeCameraReset(std::ostream &stream,
                        const std::vector<std::string> &reset, int indentLevel);
  void writeCameraMove(std::ostream &stream,
                       const std::vector<std::string> &camera, int indentLevel);
  void writeCameraRotate(std::ostream &stream,
                         const std::vector<std::string> &camera, int indentLevel);
  void writeCameraEnd(std::ostream &stream, int indentLevel);

  inline int getIndent(int level = 1) const { return 2 * level; }

  void clear();

private:
  std::map<std::string, std::string> m_cast;
};

#endif // ZMOVIESCRIPTGENERATOR_H
