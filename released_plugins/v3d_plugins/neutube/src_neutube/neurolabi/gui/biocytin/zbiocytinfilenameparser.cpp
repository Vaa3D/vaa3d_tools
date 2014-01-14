#include "zbiocytinfilenameparser.h"
#include <iostream>
#include <cmath>

#include "zstring.h"

using namespace std;

const static ZBiocytinFileNameParser::TSuffixMap::value_type rawMap[] = {
  ZBiocytinFileNameParser::TSuffixMap::value_type(
  ZBiocytinFileNameParser::EDIT, ".Edit"),
  ZBiocytinFileNameParser::TSuffixMap::value_type(
  ZBiocytinFileNameParser::PROJECTION, ".Proj"),
  ZBiocytinFileNameParser::TSuffixMap::value_type(
  ZBiocytinFileNameParser::MASK, ".Mask"),
  ZBiocytinFileNameParser::TSuffixMap::value_type(
  ZBiocytinFileNameParser::ROI, ".Roi")
};

const int MapEntryCount = sizeof(rawMap) / sizeof(rawMap[0]);

ZBiocytinFileNameParser::TSuffixMap
ZBiocytinFileNameParser::m_suffixMap(rawMap, rawMap + MapEntryCount);

ZBiocytinFileNameParser::ZBiocytinFileNameParser()
{
}

string ZBiocytinFileNameParser::getCorePath(const string &path)
{
  ZString name = ZString::removeFileExt(path);

  for (TSuffixMap::const_iterator iter = m_suffixMap.begin();
       iter != m_suffixMap.end(); ++iter) {
    if (name.endsWith(iter->second, ZString::CASE_INSENSITIVE)) {
      name = name.substr(0, name.size() - iter->second.size());
      break;
    }
  }

  return name;
}

string ZBiocytinFileNameParser::getCoreName(const string &path)
{
  return getCorePath(ZString::getBaseName(path));
}

ZBiocytinFileNameParser::ESuffixRole
ZBiocytinFileNameParser::getRole(const std::string &path)
{
  ZString name = ZString::removeFileExt(ZString::getBaseName(path));

  for (TSuffixMap::const_iterator iter = m_suffixMap.begin();
       iter != m_suffixMap.end(); ++iter) {
    if (name.endsWith(iter->second, ZString::CASE_INSENSITIVE)) {
      return iter->first;
    }
  }

  return ORIGINAL;
}

std::string ZBiocytinFileNameParser::getSuffix(ESuffixRole role)
{
  return m_suffixMap[role];
}

int ZBiocytinFileNameParser::getTileIndex(const string &path)
{
  int index = ZString(path).lastInteger();

  return abs(index);
}

std::string ZBiocytinFileNameParser::getSwcEditPath(const string &path)
{
  return getCorePath(path) + getSuffix(EDIT) + ".swc";
}
