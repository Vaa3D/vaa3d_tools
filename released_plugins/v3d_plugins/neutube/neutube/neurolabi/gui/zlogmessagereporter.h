#ifndef ZLOGMESSAGEREPORTER_H
#define ZLOGMESSAGEREPORTER_H

#include <string>
#include "zmessagereporter.h"

class ZLogMessageReporter : public ZMessageReporter
{
public:
  ZLogMessageReporter();
  void report(const std::string &title, const std::string &message,
              EMessageType msgType);

  virtual void setInfoFile(const std::string &f) {
    m_infoFile = f;
  }

  virtual void setWarnFile(const std::string &f) {
    m_warnFile = f;
  }

  virtual void setErrorFile(const std::string &f) {
    m_errorFile = f;
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
  std::string m_warnFile;
  std::string m_errorFile;
};

#endif // ZLOGMESSAGEREPORTER_H
