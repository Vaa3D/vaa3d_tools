#include "zmoviescriptgenerator.h"

#include "tz_error.h"

using namespace std;

ZMovieScriptGenerator::ZMovieScriptGenerator()
{
}

void ZMovieScriptGenerator::addActor(const string &id, const string &source)
{
  m_cast[id] = source;
}

void ZMovieScriptGenerator::writeCast(ostream &stream, int indentLevel)
{
  string firtsLevelIndent = std::string(getIndent(indentLevel), ' ');
  string secondLevelIndent = std::string(getIndent(indentLevel + 1), ' ');
  string thirdLevelIndent = std::string(getIndent(indentLevel + 2), ' ');

  stream << firtsLevelIndent << "\"cast\": [";
  for (map<string, string>::const_iterator iter = m_cast.begin();
       iter != m_cast.end(); ++iter) {
    if (iter != m_cast.begin()) {
      stream << ",";
    }
    stream << endl;
    stream << secondLevelIndent << "{" << endl;
    stream << thirdLevelIndent << "\"id\": " << "\"" << iter->first
           << "\"," <<endl;
    stream << thirdLevelIndent << "\"source\": " << "\"" << iter->second
           << "\"" << endl;
    stream << secondLevelIndent << "}";
  }
  stream << endl << firtsLevelIndent << "]";
}

#define DOUBLE_QUOTE(str) (string("\"") + str + "\"")

void ZMovieScriptGenerator::writeShowAction(ostream &stream, int indentLevel)
{
  string firtsLevelIndent = std::string(getIndent(indentLevel), ' ');
  string secondLevelIndent = std::string(getIndent(indentLevel + 1), ' ');
  string thirdLevelIndent = std::string(getIndent(indentLevel + 2), ' ');

  stream << firtsLevelIndent << "\"action\": [";
  for (map<string, string>::const_iterator iter = m_cast.begin();
       iter != m_cast.end(); ++iter) {
    if (iter != m_cast.begin()) {
      stream << ",";
    }
    stream << endl;
    stream << secondLevelIndent << "{" << endl;
    stream << thirdLevelIndent << "\"id\": " << "\"" << iter->first
           << "\"," <<endl;
    stream << thirdLevelIndent << "\"visible\": true" << endl;
    stream << secondLevelIndent << "}";
  }
  stream << endl << firtsLevelIndent << "]";
}

void ZMovieScriptGenerator::writeAction(
    ostream &stream, const vector<string> &actions, int indentLevel)
{
  string firtsLevelIndent = std::string(getIndent(indentLevel), ' ');
  string secondLevelIndent = std::string(getIndent(indentLevel + 1), ' ');
  string thirdLevelIndent = std::string(getIndent(indentLevel + 2), ' ');

  stream << firtsLevelIndent << DOUBLE_QUOTE("action") << ": [";
  for (map<string, string>::const_iterator iter = m_cast.begin();
       iter != m_cast.end(); ++iter) {
    if (iter != m_cast.begin()) {
      stream << ",";
    }
    stream << endl;
    stream << secondLevelIndent << "{" << endl;
    stream << thirdLevelIndent << DOUBLE_QUOTE("id") << ": "
           << DOUBLE_QUOTE(iter->first);

    TZ_ASSERT(actions.size() % 2 == 0, "must be pairs");

    for (size_t actionIndex = 0; actionIndex < actions.size();
         actionIndex += 2) {
      stream << "," <<endl;
      stream << thirdLevelIndent << DOUBLE_QUOTE(actions[actionIndex]) << ": "
             << actions[actionIndex + 1];
    }
    stream << endl;
    stream << secondLevelIndent << "}";
  }
  stream << endl << firtsLevelIndent << "]";
}

void ZMovieScriptGenerator::writeAction(ostream &stream, const string &id,
                                        const vector<string> &actions,
                                        int indentLevel)
{
  string firstLevelIndent = std::string(getIndent(indentLevel), ' ');
  string secondLevelIndent = std::string(getIndent(indentLevel + 1), ' ');

  stream << firstLevelIndent << "{" << endl;
  stream << secondLevelIndent << DOUBLE_QUOTE("id") << ": "
         << DOUBLE_QUOTE(id);

  TZ_ASSERT(actions.size() % 2 == 0, "must be pairs");

  for (size_t actionIndex = 0; actionIndex < actions.size();
       actionIndex += 2) {
    stream << "," <<endl;
    stream << secondLevelIndent << DOUBLE_QUOTE(actions[actionIndex]) << ": "
           << actions[actionIndex + 1];
  }
  stream << endl;
  stream << firstLevelIndent << "}";
}

void ZMovieScriptGenerator::writeAction(
    ostream &stream, const std::vector<string> &id,
    const std::vector<string> &actions, int indentLevel)
{
  for (std::vector<string>::const_iterator iter = id.begin(); iter != id.end();
       ++iter) {
    if (iter != id.begin()) {
      stream << "," << endl;
    }

    writeAction(stream, *iter, actions, indentLevel);
  }
}

void ZMovieScriptGenerator::clear()
{
  m_cast.clear();
}

void ZMovieScriptGenerator::writePlotStart(ostream &stream, int indentLevel)
{
  string firstLevelIndent = std::string(getIndent(indentLevel), ' ');
  stream << firstLevelIndent << DOUBLE_QUOTE("plot") << ": [" << endl;
}

void ZMovieScriptGenerator::writePlotEnd(ostream &stream, int indentLevel)
{
  string firstLevelIndent = std::string(getIndent(indentLevel), ' ');
  stream << firstLevelIndent << "]";
}

void ZMovieScriptGenerator::writeDuration(ostream &stream, int duration,
                                          int indentLevel)
{
  string firstLevelIndent = std::string(getIndent(indentLevel), ' ');
  stream << firstLevelIndent << DOUBLE_QUOTE("duration") << ":" << duration;
}

void ZMovieScriptGenerator::writeActionStart(ostream &stream, int indentLevel)
{
  string firstLevelIndent = std::string(getIndent(indentLevel), ' ');
  stream << firstLevelIndent << DOUBLE_QUOTE("action") << ": [" << endl;
}

void ZMovieScriptGenerator::writeActionEnd(ostream &stream, int indentLevel)
{
  string firstLevelIndent = std::string(getIndent(indentLevel), ' ');
  stream << firstLevelIndent << "]";
}

void ZMovieScriptGenerator::writeSceneStart(ostream &stream, int indentLevel)
{
  string firstLevelIndent = std::string(getIndent(indentLevel), ' ');
  stream << firstLevelIndent << "{" << endl;
}

void ZMovieScriptGenerator::writeSceneEnd(ostream &stream, int indentLevel)
{
  string firstLevelIndent = std::string(getIndent(indentLevel), ' ');
  stream << firstLevelIndent << "}";
}

void ZMovieScriptGenerator::writeCameraStart(ostream &stream, int indentLevel)
{
  string firstLevelIndent = std::string(getIndent(indentLevel), ' ');
  stream << firstLevelIndent << DOUBLE_QUOTE("camera") << ": {" << endl;
}

void ZMovieScriptGenerator::writeCameraEnd(ostream &stream, int indentLevel)
{
  string firstLevelIndent = std::string(getIndent(indentLevel), ' ');
  stream << firstLevelIndent << "}";
}

void ZMovieScriptGenerator::writeCameraReset(
    ostream &stream, const vector<string> &reset, int indentLevel)
{
  string firstLevelIndent = std::string(getIndent(indentLevel), ' ');
  string secondLevelIndent = std::string(getIndent(indentLevel + 1), ' ');

  stream << firstLevelIndent << DOUBLE_QUOTE("reset") << ": {" << endl;

  TZ_ASSERT(reset.size() % 2 == 0, "must be pairs");

  for (size_t resetIndex = 0; resetIndex < reset.size();
       resetIndex += 2) {
    if (resetIndex > 0) {
      stream << "," <<endl;
    }
    stream << secondLevelIndent << DOUBLE_QUOTE(reset[resetIndex]) << ": "
           << reset[resetIndex + 1];
  }
  stream << endl;
  stream << firstLevelIndent << "}";
}

void ZMovieScriptGenerator::writeCameraMove(
    ostream &stream, const vector<string> &reset, int indentLevel)
{
  string firstLevelIndent = std::string(getIndent(indentLevel), ' ');
  string secondLevelIndent = std::string(getIndent(indentLevel + 1), ' ');

  stream << firstLevelIndent << DOUBLE_QUOTE("move") << ": {" << endl;

  TZ_ASSERT(reset.size() % 2 == 0, "must be pairs");

  for (size_t resetIndex = 0; resetIndex < reset.size();
       resetIndex += 2) {
    if (resetIndex > 0) {
      stream << "," <<endl;
    }
    stream << secondLevelIndent << DOUBLE_QUOTE(reset[resetIndex]) << ": "
           << reset[resetIndex + 1];
  }
  stream << endl;
  stream << firstLevelIndent << "}";
}

void ZMovieScriptGenerator::writeCameraRotate(
    ostream &stream, const vector<string> &reset, int indentLevel)
{
  string firstLevelIndent = std::string(getIndent(indentLevel), ' ');
  string secondLevelIndent = std::string(getIndent(indentLevel + 1), ' ');

  stream << firstLevelIndent << DOUBLE_QUOTE("rotate") << ": {" << endl;

  TZ_ASSERT(reset.size() % 2 == 0, "must be pairs");

  for (size_t resetIndex = 0; resetIndex < reset.size();
       resetIndex += 2) {
    if (resetIndex > 0) {
      stream << "," <<endl;
    }
    stream << secondLevelIndent << DOUBLE_QUOTE(reset[resetIndex]) << ": "
           << reset[resetIndex + 1];
  }
  stream << endl;
  stream << firstLevelIndent << "}";
}

