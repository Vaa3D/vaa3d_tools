#include "zlogmessagereporter.h"
#include <fstream>
#include <iostream>

ZLogMessageReporter::ZLogMessageReporter()
{
}

void ZLogMessageReporter::report(
    const std::string &title, const std::string &message, EMessageType msgType)
{
  std::ofstream stream;

  if (msgType == Debug) {
    ZMessageReporter::report(std::cout, title, message, msgType);
  } else {
    switch (msgType) {
    case Information:
      stream.open(m_infoFile.c_str(), std::ios_base::out);
      break;
    case Warning:
      stream.open(m_warnFile.c_str(), std::ios_base::out | std::ios_base::app);
      break;
    case Error:
      stream.open(m_errorFile.c_str(), std::ios_base::out | std::ios_base::app);
      break;
    default:
      break;
    }

    ZMessageReporter::report(stream, title, message, msgType);

     stream.close();
  }
}
