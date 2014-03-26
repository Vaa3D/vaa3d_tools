#include "zintmap.h"

#include <iostream>

#include "tz_string.h"

using namespace std;

int ZIntMap::load(const std::string &filePath)
{
  clear();

  int recordNumber = 0;

  FILE *fp = fopen(filePath.c_str(), "r");

  if (fp != NULL) {
    String_Workspace *sw = New_String_Workspace();
    int row[2];
    /* For each row of the file */
    char *line = NULL;

    while ((line = Read_Line(fp, sw)) != NULL) {
      int n = 0;
      String_To_Integer_Array(line, row, &n);
      if (n == 2) {
        (*this)[row[0]] = row[1];
        recordNumber++;
      }
    }

    Kill_String_Workspace(sw);

    fclose(fp);
  }

  return recordNumber;
}

std::vector<int> ZIntMap::getKeyArray(int value)
{
  std::vector<int> keyArray;
  for (const_iterator iter = begin(); iter != end(); iter++) {
    if (iter->second == value) {
      keyArray.push_back(iter->first);
    }
  }

  return keyArray;
}

void ZIntMap::print(ostream &stream, EPrintOption option)
{
  printSummary(stream);

  switch (option) {
  case NORMAL_PRINT:
    for (const_iterator iter = begin(); iter != end(); iter++) {
      stream << iter->first << " " << iter->second << endl;
    }
    break;
  case KEY_GROUP:
  {
    map<int, vector<int> > group;
    for (const_iterator iter = begin(); iter != end(); iter++) {
      group[iter->second].push_back(iter->first);
    }

    for (map<int, vector<int> >::const_iterator iter = group.begin();
         iter != group.end(); ++iter) {
      vector<int> keyArray = iter->second;
      for (size_t i = 0; i < keyArray.size(); ++i) {
        stream << keyArray[i] << " ";
      }
      stream << ": " << iter->first << endl;
    }
  }
    break;
  default:
    break;
  }
}

void ZIntMap::incValue(int key)
{
  if (this->count(key) == 0) {
    (*this)[key] = 1;
  } else {
    (*this)[key]++;
  }
}

void ZIntMap::printSummary(std::ostream &stream)
{
  stream << "Int map: " << size() << " entries." << endl;
}
