#include "zmessagereporter.h"

#include <iostream>
#include <ctime>

using namespace std;

int ZMessageReporter::m_count = 1;

ZMessageReporter::ZMessageReporter()
{
}

void ZMessageReporter::report(
    const std::string &title, const std::string &message, EMessageType msgType)
{
  if (msgType == Error) {
    report(cerr, title, message, msgType);
  } else {
    report(cout, title, message, msgType);
  }
  /*
  switch (msgType) {
  case INFORMATION:
    cout << title << endl << "  " << message << endl;
    break;
  case WARNING:
    cout  << title << endl << "  " << message << endl;
    break;
  case ERROR:
    cerr << "Error: " << title << endl << "  " << message << endl;
    break;
  case DEBUG:
    cout << "Debug: " << title << endl << "  " << message << endl;
    break;
  }
  */
}

void ZMessageReporter::report(std::ostream &stream,
                              const std::string &title,
                              const std::string &message,
                              EMessageType msgType)
{
  time_t timer;
  stream << "=====" << time(&timer) << "=====" << endl;
  stream << "#" << m_count << ": ";

  switch (msgType) {
  case Information:
    stream << title << endl << "  " << message << endl;
    break;
  case Warning:
    stream  << title << endl << "  " << message << endl;
    break;
  case Error:
    stream << "Error: " << title << endl << "  " << message << endl;
    break;
  case Debug:
    stream << "Debug: " << title << endl << "  " << message << endl;
    break;
  }

  ++m_count;
}
