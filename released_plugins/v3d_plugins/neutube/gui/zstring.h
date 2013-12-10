#ifndef ZSTRING_H
#define ZSTRING_H

#include <string>
#if defined(_QT_GUI_USED_)
#include <QString>
#endif
#include <vector>

#include "tz_string.h"

class ZString : public std::string
{
public:
  ZString();
  ZString(const ZString &str);
  ZString(const std::string& str);
  ZString ( const std::string& str, size_t pos, size_t n = npos );
  ZString ( const char * s, size_t n );
  ZString ( const char * s );
  ZString ( size_t n, char c );
#if defined(_QT_GUI_USED_)
  ZString(const QString &str);
#endif
  template<class InputIterator> ZString (
      InputIterator begin, InputIterator end) : std::string(begin, end)
  {
    init();
  }
  ~ZString();

public:
  static const char FileSeparator;

  enum ECaseSensitivity {
    CASE_SENSITIVE, CASE_INSENSITIVE
  };

  inline void init() { m_workspace = New_String_Workspace(); }

  int firstInteger();
  int lastInteger();
  /*!
   * \brief Extract the last integer in a string
   * \param Input string.
   * \return Returns the last integer of \a str. It returns 0 if there is no
   *         integer in the string.
   */
  static int lastInteger(const std::string &str);
  double firstDouble();
  double lastDouble();
  std::string firstQuotedWord();

  std::vector<int> toIntegerArray();
  std::vector<double> toDoubleArray();
  std::vector<std::string> toWordArray(const std::string &delim = ", \n");
  std::vector<std::string> tokenize(char c);

  bool readLine(FILE *fp);
  bool contains(const std::string &str);
  bool containsDigit();
  std::string& replace(const std::string &from, const std::string &to);
  std::string& replace(int from, const std::string &to);
  bool startsWith(const std::string &str, ECaseSensitivity cs = CASE_SENSITIVE);
  bool endsWith(const std::string &str, ECaseSensitivity cs = CASE_SENSITIVE);
  void trim();

  void toLower();
  void toUpper();

  std::string toLower() const;
  std::string toUpper() const;

  void appendNumber(int num, int pad = 0);

  static ZString dirPath(const std::string &path);
  ZString dirPath();
  bool isAbsolutePath() const;
  static bool isAbsolutePath(const std::string &path);
  static std::string absolutePath(const std::string &dir,
                                  const std::string &relative);
  static std::string fullPath(const std::string &dir, const std::string &fname,
                              const std::string &ext);
  static std::string fullPath(const std::vector<std::string> &parts);
  static std::string removeFileExt(const std::string &str);
  static std::string getBaseName(const std::string &str);

  ZString absolutePath(const std::string &dir) const;
  std::vector<std::string> fileParts() const;
  ZString toFileExt();
  ZString toFileName();

  ZString &operator= (const ZString &str);

  std::string changeExt(const std::string &newext) const;
  std::string changeDirectory(const std::string &newdir);

  inline const ZString& constRef() const {
    return *this;
  }

private:
  String_Workspace *m_workspace;
};

#endif // ZSTRING_H
