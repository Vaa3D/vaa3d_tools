#ifndef ZBIOCYTINFILENAMEPARSER_H
#define ZBIOCYTINFILENAMEPARSER_H

#include <string>
#include <map>

class ZBiocytinFileNameParser
{
public:
  ZBiocytinFileNameParser();

  enum ESuffixRole {
    EDIT, PROJECTION, MASK, ROI, ORIGINAL
  };

  static std::string getCoreName(const std::string &path);
  static ESuffixRole getRole(const std::string &path);
  static std::string getSuffix(ESuffixRole role);
  static int getTileIndex(const std::string &path);

  typedef std::map<ESuffixRole, std::string> TSuffixMap;

private:
  static TSuffixMap m_suffixMap;
};

#endif // ZBIOCYTINFILENAMEPARSER_H
