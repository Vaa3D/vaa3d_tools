#ifndef ZINTMAP_H
#define ZINTMAP_H

#include <map>
#include <string>
#include <vector>
#include <ostream>
#include <iostream>
#if defined(_WIN32) || defined(_WIN64)
#undef NORMAL_PRINT
#endif

class ZIntMap : public std::map<int, int>
{
public:
  enum EPrintOption {
    NORMAL_PRINT, SUMMARY, KEY_GROUP
  };

  int load(const std::string &filePath);
  std::vector<int> getKeyArray(int value);
  void incValue(int key);
  void print(std::ostream &stream = std::cout,
             EPrintOption option = NORMAL_PRINT);
  void printSummary(std::ostream &stream = std::cout);

};

#endif // ZINTMAP_H
