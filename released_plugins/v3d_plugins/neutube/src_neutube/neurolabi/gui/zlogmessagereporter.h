#ifndef ZLOGMESSAGEREPORTER_H
#define ZLOGMESSAGEREPORTER_H

#include <string>
#include <fstream>
#include "zmessagereporter.h"

class ZLogMessageReporter : public ZMessageReporter
{
public:
  ZLogMessageReporter();
  void report(const std::string &title, const std::string &message,
              EMessageType msgType);

  virtual void setInfoFile(const std::string &f);

  virtual void setWarnFile(const std::string &f) {
    m_warnFile = f;
    m_warnStream.open(m_warnFile.c_str(), std::ios_base::out | std::ios_base::app);
  }

  virtual void setErrorFile(const std::string &f) {
    m_errorFile = f;
    m_errorStream.open(m_errorFile.c_str(), std::ios_base::out | std::ios_base::app);
  }

  inline const std::string& getInfoFile() const {
    return m_infoFile;
  }

  inline const std::string& getWarnFile() const {
    return m_warnFile;
  }

  inline const std::string& getErrorFile() const {
    return m_errorFile;
  }

private:
  std::string m_infoFile;
  std::ofstream m_infoStream;
  std::string m_warnFile;
  std::ofstream m_warnStream;
  std::string m_errorFile;
  std::ofstream m_errorStream;
  size_t m_maxFileSize;
};

#endif // ZLOGMESSAGEREPORTER_H
