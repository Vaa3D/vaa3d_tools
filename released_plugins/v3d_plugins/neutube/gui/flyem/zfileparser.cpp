#include "zfileparser.h"
#include <sstream>
#include <fstream>
#include <iostream>
#include "zstring.h"
#include "tz_error.h"
#include "zerror.h"

using namespace FlyEm;
using namespace std;

ZFileParser::ZFileParser()
{
}

map<int, string> ZFileParser::loadBodyList(string filePath, string workDir)
{
  map<int, string> bodyIdMap;

  FILE *fp = fopen(filePath.c_str(), "r");

  if (fp != NULL) {
    ZString line;
    while (line.readLine(fp)) {
      int bodyId = line.firstInteger();
      if (bodyId > 0) {
        line.replace(bodyId, "");
        line.replace(" ", "");
        line.replace("\t", "");
        line.replace("(", "_");
        line.replace(")", "_");
        line.replace("?", "_x_");
        line.replace("\\", "_");
        line.replace("/", "_");
        line.replace("-", "_");

        ostringstream stream;
        if (workDir.empty()) {
          stream << line << "_" << bodyId << ".swc";
        } else {
          stream << workDir << '/' << line << "_" << bodyId << ".swc";
        }
        bodyIdMap[bodyId] = stream.str();
      }
    }
    fclose(fp);
  } else {
    RECORD_WARNING(true, "Cannot open file " + filePath);
  }

  return bodyIdMap;
}

bool ZFileParser::writeVaa3dMakerFile(std::string filePath,
                                      vector<ZVaa3dMarker> markerArray)
{
  ofstream stream(filePath.c_str());
  if (!stream.is_open()) {
    RECORD_WARNING(true, "Cannot open file " + filePath);
    return false;
  }

  for (vector<ZVaa3dMarker>::iterator iter = markerArray.begin();
       iter != markerArray.end(); ++iter) {
    stream << iter->toString() << endl;
  }

  stream.close();

  return true;
}

bool ZFileParser::writeVaa3dApoFile(std::string filePath,
                                    vector<ZVaa3dApo> markerArray)
{
  ofstream stream(filePath.c_str());
  if (!stream.is_open()) {
    RECORD_WARNING(true, "Cannot open file " + filePath);
    return false;
  }

  for (vector<ZVaa3dApo>::iterator iter = markerArray.begin();
       iter != markerArray.end(); ++iter) {
    stream << iter->toString() << endl;
  }

  stream.close();

  return true;
}

string ZFileParser::bodyNameToFileName(const string bodyName)
{
  ZString str(bodyName);

  int bodyId = str.firstInteger();

  str.replace(bodyId, "");
  str.replace(" ", "");
  str.replace("\t", "");
  str.replace("(", "_");
  str.replace(")", "_");
  str.replace("?", "_x_");
  str.replace("\\", "_");
  str.replace("/", "_");
  str.replace("-", "_");

  ostringstream stream;
  stream << str << "_" << bodyId;

  return stream.str();
}

std::vector<ZVaa3dMarker> ZFileParser::readVaa3dMarkerFile(
    const std::string &filePath)
{
  //TZ_ERROR(ERROR_PART_FUNC);

  //UNUSED_PARAMETER(filePath.c_str());

  std::vector<ZVaa3dMarker> markerArray;

  ZString str;
  FILE *fp = fopen(filePath.c_str(), "r");

  if (fp != NULL) {
    while (str.readLine(fp)) {
      std::vector<string> fieldArray = str.tokenize(',');
      if (fieldArray.size() >= 10) {
        markerArray.resize(markerArray.size() + 1);
        ZString field = fieldArray[0] + "," + fieldArray[1] + "," +
            fieldArray[2] + "," + fieldArray[3];
        std::vector<double> valueArray = field.toDoubleArray();
        markerArray.back().setCenter(valueArray[0], valueArray[1],
            valueArray[2]);
        markerArray.back().setRadius(valueArray[3]);
        markerArray.back().setType(String_Last_Integer(fieldArray[4].c_str()));
        markerArray.back().setName(fieldArray[5]);
        markerArray.back().setComment(fieldArray[6]);
        field = fieldArray[7] + "," + fieldArray[8] + "," + fieldArray[9];
        std::vector<int> color = field.toIntegerArray();
        markerArray.back().setColor(color[0], color[1], color[2]);
      }
      /*
      std::vector<double> valueArray = str.toDoubleArray();
      if (valueArray.size() == 8) {
        markerArray.resize(markerArray.size() + 1);
        markerArray.back().setCenter(valueArray[0], valueArray[1],
            valueArray[2]);
        markerArray.back().setRadius(valueArray[3]);
        markerArray.back().setType(valueArray[4]);
        markerArray.back().setColor(
              valueArray[5], valueArray[6], valueArray[7]);
      }
      */
    }

    fclose(fp);
  } else {
    RECORD_WARNING(true, "Cannot open file " + filePath);
  }

  return markerArray;
}
