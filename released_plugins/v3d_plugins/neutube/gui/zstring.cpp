#include "zstring.h"

#include <string.h>
#include <stdlib.h>
#include <sstream>
#include <iostream>
#include <algorithm>

#include "tz_error.h"
#include "tz_utilities.h"

using namespace std;

#if defined(_WIN32) || defined(_WIN64)
  const char ZString::FileSeparator = '\\';
#else
  const char ZString::FileSeparator = '/';
#endif

ZString::ZString() : string()
{
  init();
}

ZString::ZString(const ZString &str) : string(str.c_str())
{
  init();
}

ZString::ZString(const char *s) : string(s)
{
  init();
}

ZString::ZString(const char *s, size_t n) : string(s, n)
{
  init();
}

ZString::ZString(const std::string &str) : string(str)
{
  init();
}

ZString::ZString(const std::string &str, size_t pos, size_t n) :
  string(str, pos, n)
{
  init();
}

#if defined(_QT_GUI_USED_)
ZString::ZString(const QString &str) : string(str.toStdString())
{
  init();
}
#endif

ZString::~ZString()
{
  if (m_workspace != NULL) {
    Kill_String_Workspace(m_workspace);
  }
}

int ZString::firstInteger()
{
  return String_First_Integer(c_str());
}

int ZString::lastInteger()
{
  return String_Last_Integer(c_str());
}

int ZString::lastInteger(const string &str)
{
  return String_Last_Integer(str.c_str());
}

vector<int> ZString::toIntegerArray()
{
  int n;
  int *array = String_To_Integer_Array(c_str(), NULL, &n);

  vector<int> valueArray(n);
  for (int i = 0; i < n; i++) {
    valueArray[i] = array[i];
  }

  return valueArray;
}

vector<double> ZString::toDoubleArray()
{
  int n;
  double *array = String_To_Double_Array(c_str(), NULL, &n);

  vector<double> valueArray(n);
  for (int i = 0; i < n; i++) {
    valueArray[i] = array[i];
  }

  return valueArray;
}

double ZString::firstDouble()
{
  vector<double> valueArray = toDoubleArray();

  return valueArray[0];
}

double ZString::lastDouble()
{
  vector<double> valueArray = toDoubleArray();

  return valueArray.back();
}

std::vector<std::string> ZString::toWordArray(const string &delim)
{
  std::vector<std::string> wordArray;

  char *str = strdup(c_str());

  char *word = strtok(str, delim.c_str());

  while (word != NULL) {
    wordArray.push_back(word);
    word = strtok(NULL, delim.c_str());
  }

  free(str);

  return wordArray;
}

std::vector<std::string> ZString::tokenize(char c)
{
  std::vector<size_t> tokenPos;
  std::vector<std::string> wordArray;

  if (!empty()) {
#ifdef _DEBUG_2
    cout << wordArray.size() << endl;
#endif

    size_t currPos = find_first_of(c);

    while (currPos != std::string::npos) {
      tokenPos.push_back(currPos);
      currPos = find_first_of(c, currPos + 1);
    }

    if (tokenPos.size() > 0) {
      if (tokenPos[0] == 0) {
        wordArray.push_back("");
      } else {
        wordArray.push_back(substr(0, tokenPos[0]));
      }
    }

    for (size_t i = 0; i < tokenPos.size() - 1; ++i) {
#ifdef _DEBUG_2
      cout << substr(tokenPos[i] + 1, tokenPos[i + 1] - tokenPos[i] - 1) << endl;
#endif
      wordArray.push_back(
            substr(tokenPos[i] + 1, tokenPos[i + 1] - tokenPos[i] - 1));
    }
    wordArray.push_back(substr(tokenPos.back()));
  }

  return wordArray;
}

bool ZString::readLine(FILE *fp)
{
  bool success = true;

  char *line = Read_Line(fp, m_workspace);

  if (line != NULL) {
    this->assign(line);
  } else {
    success = false;
  }

  return success;
}

bool ZString::contains(const string &str)
{
  return find(str) != string::npos;
}

bool ZString::containsDigit()
{
  for (size_t i = 0; i < length(); ++i) {
    if (at(i) >= '0' && at(i) <= '9') {
      return true;
    }
  }

  return false;
}


string& ZString::replace(const string &from, const string &to)
{
  if (from.size() == 0) {
    return *this;
  }

  size_t found = find(from);

  while (found != string::npos) {
    string::replace(found, from.size(), to);
    found += to.size();
    found = find(from, found);
  }

  return *this;
}

string& ZString::replace(int from, const string &to)
{
  ostringstream stream;
  stream << from;
  string fromStr = stream.str();

  if (fromStr.size() == 0) {
    return *this;
  }

  size_t found = find(fromStr);

  while (found != string::npos) {
    string::replace(found, fromStr.size(), to);
    found += to.size();
    found = find(fromStr, found);
  }

  return *this;
}

bool ZString::startsWith(const string &str, ECaseSensitivity cs)
{
  string str1(c_str());
  string str2(str.c_str());

  if (cs == ZString::CASE_INSENSITIVE) {
    std::transform(str1.begin(), str1.end(), str1.begin(), ::tolower);
    std::transform(str2.begin(), str2.end(), str2.begin(), ::tolower);
  }

  return String_Starts_With(str1.c_str(), str2.c_str());
}

bool ZString::endsWith(const string &str, ECaseSensitivity cs)
{
  string str1(c_str());
  string str2(str.c_str());

  if (cs == ZString::CASE_INSENSITIVE) {
    std::transform(str1.begin(), str1.end(), str1.begin(), ::tolower);
    std::transform(str2.begin(), str2.end(), str2.begin(), ::tolower);
  }

  return String_Ends_With(str1.c_str(), str2.c_str());
}

void ZString::toLower()
{
  std::transform(begin(), end(), begin(), ::tolower);
}

void ZString::toUpper()
{
  std::transform(begin(), end(), begin(), ::toupper);
}

std::string ZString::toLower() const
{
  std::string str = *this;
  std::transform(str.begin(), str.end(), str.begin(), ::tolower);

  return str;
}

std::string ZString::toUpper() const
{
  std::string str = *this;
  std::transform(str.begin(), str.end(), str.begin(), ::toupper);

  return str;
}


void ZString::appendNumber(int num, int pad)
{
  static const int kMaxPadSize = 100;

  TZ_ASSERT(pad >= 0 && pad <= kMaxPadSize, "Invalid padding");

  if (pad >= 0 && pad <= kMaxPadSize) {
    char numStr[50];

    sprintf(numStr, "%d", num);

    int zeroNumber = pad - strlen(numStr);
    for (int i = 0; i < zeroNumber; i++) {
      (*this) += '0';
    }

    (*this) += numStr;
  } else {
    cerr << "Appending failed" << endl;
  }
}

void ZString::trim()
{
  char *str = strdup(c_str());
  strtrim(str);
  string::replace(0, strlen(str) + 1, str);

  free(str);
}

bool ZString::isAbsolutePath(const string &path)
{
#if defined(_WIN32) || defined(_WIN64)
  if (path.length() > 1) {
    if (path[2] == ':') {
      return true;
    }
  }
#else
  if (!path.empty()) {
    if (path[0] == FileSeparator) {
      return true;
    }
  }
#endif

  return false;
}

bool ZString::isAbsolutePath() const
{
  return isAbsolutePath(*this);
}

string ZString::absolutePath(const string &dir, const string &relative)
{
  ZString fullPath(relative);

  if (!isAbsolutePath(relative)) {
    if (dir.empty()) {
#if defined(_WIN32) || defined(_WIN64)
      fullPath = "C:\\" + fullPath;
#else
      fullPath = "/" + fullPath;
#endif
    } else {
      if (dir[dir.length() - 1] != FileSeparator) {
        fullPath = dir + FileSeparator + fullPath;
      } else {
        fullPath = dir + fullPath;
      }
    }
  }

  fullPath.replace("/./", "/");

  return fullPath;
}

string ZString::fullPath(
    const string &dir, const string &fname, const string &ext)
{
  bool addingSeparator = true;

  if (!dir.empty()) {
    if (dir[dir.length() - 1] == FileSeparator) {
      addingSeparator = false;
    }
  }

  if (addingSeparator) {
    return dir + FileSeparator + fname + '.' + ext;
  }

  return dir + fname + '.' + ext;
}

string ZString::fullPath(const vector<string> &parts)
{
  string path;

  if (!parts.empty()) {
    std::vector<std::string>::const_iterator iter = parts.begin();
    path = *iter;
    for (++iter; iter != parts.end(); ++iter) {
      bool addingSeparator = true;
      if (!path.empty()) {
        if (path[path.length() - 1] == FileSeparator) {
          addingSeparator = false;
        }
      }

      if (addingSeparator) {
        path += FileSeparator;
      }

      path += *iter;
    }
  }

  return path;
}

string ZString::getBaseName(const string &str)
{
  size_type pos = str.find_last_of(FileSeparator);

  return str.substr(pos + 1);
}

string ZString::removeFileExt(const string &str)
{
  size_type s1 = str.find_last_of(FileSeparator);
  size_type s2 = str.find_last_of('.');
  size_type pos = string::npos;

  if (s2 != string::npos) {
    if (s1 ==string::npos || s1 < s2) {
      pos = s2;
    }
  }

  return str.substr(0, pos);
}

ZString ZString::absolutePath(const string &dir) const
{
  return absolutePath(dir, *this);
}

ZString ZString::dirPath(const string &path)
{
  size_type s = path.find_last_of(FileSeparator);

  if (s != string::npos) {
    return path.substr(0, s);
  }

  return path;
}

ZString ZString::dirPath()
{
  size_type s = find_last_of(FileSeparator);

  if (s != string::npos) {
    return substr(0, s);
  }

  return *this;
}

vector<string> ZString::fileParts() const
{
  vector<string> parts;

  size_type s1 = find_last_of(FileSeparator);
  size_type s2 = find_last_of('.');

  if (s1 ==string::npos) {
    parts.push_back("");
    if (s2 == string::npos) {
      parts.push_back(*this);
    } else {
      parts.push_back(substr(0, s2));
      parts.push_back(substr(s2 + 1));
    }
  } else{
    parts.push_back(substr(0, s1 + 1));
    if (s1 < size() - 1) {
      if (s2 == string::npos || s2 < s1) {
        parts.push_back(substr(s1 + 1));
      } else {
        parts.push_back(substr(s1 + 1, s2 - s1 - 1));
        parts.push_back(substr(s2 + 1));
      }
    }
  }

  return parts;
}

ZString ZString::toFileExt()
{
  return ZString(fextn(c_str()));
}

ZString ZString::toFileName()
{
  size_type s = find_last_of(FileSeparator);

  if (s != string::npos) {
    return substr(s + 1);
  }

  return *this;
}

ZString& ZString::operator =(const ZString &str)
{
  string::operator=(str);

  return *this;
}

string ZString::changeExt(const string &newext) const
{
  vector<string> parts = fileParts();
  if (parts.size() >= 2) {
    parts.resize(3);
    parts[2] = newext;
  } else {
    return *this;
  }

  return fullPath(parts[0], parts[1], parts[2]);
}

string ZString::changeDirectory(const string &newdir)
{
  vector<string> parts = fileParts();
  parts[0] = newdir;
  if (parts.size() == 1) {
    return parts[0];
  } else if (parts.size() == 2) {
    return fullPath(parts);
  }

  return fullPath(parts[0], parts[1], parts[2]);
}

string ZString::firstQuotedWord()
{
  string word;
  size_t start = find('"');
  size_t end = find('"', start + 1);

  if (end != string::npos && end > start) {
    word = substr(start + 1, end - start - 1);
  }

  return word;
}
