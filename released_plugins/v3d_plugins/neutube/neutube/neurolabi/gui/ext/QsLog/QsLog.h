// Copyright (c) 2013, Razvan Petru
// All rights reserved.

// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:

// * Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice, this
//   list of conditions and the following disclaimer in the documentation and/or other
//   materials provided with the distribution.
// * The name of the contributors may not be used to endorse or promote products
//   derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef QSLOG_H
#define QSLOG_H

#include "QsLogLevel.h"
#include "QsLogDest.h"
#include <QDebug>
#include <QString>

#define QS_LOG_VERSION "2.0b1"

namespace QsLogging
{
class Destination;
class LoggerImpl; // d pointer

class Logger
{
public:
    static Logger& instance()
    {
        static Logger staticLog;
        return staticLog;
    }

    //! Adds a log message destination. Don't add null destinations.
    void addDestination(DestinationPtr destination);
    void removeDestination(DestinationPtr destination);
    //! Logging at a level < 'newLevel' will be ignored
    void setLoggingLevel(Level newLevel);
    //! The default level is INFO
    Level loggingLevel() const;

    //! The helper forwards the streaming to QDebug and builds the final
    //! log message.
    class Helper
    {
    public:
        explicit Helper(Level logLevel) :
            level(logLevel),
            qtDebug(&buffer) {}
        ~Helper();
        QDebug& stream(){ return qtDebug; }

    private:
        void writeToLog();

        Level level;
        QString buffer;
        QDebug qtDebug;
    };

private:
    Logger();
    Logger(const Logger&);
    Logger& operator=(const Logger&);
    ~Logger();

    void enqueueWrite(const QString& message, Level level);
    void write(const QString& message, Level level);

    LoggerImpl* d;

    friend class LogWriterRunnable;
};

} // end namespace

//! Logging macros: define QS_LOG_LINE_NUMBERS to get the file and line number
//! in the log output.
#ifndef QS_LOG_LINE_NUMBERS
#define QLOG_TRACE() \
    if (QsLogging::Logger::instance().loggingLevel() > QsLogging::TraceLevel) {} \
    else QsLogging::Logger::Helper(QsLogging::TraceLevel).stream()
#define QLOG_DEBUG() \
    if (QsLogging::Logger::instance().loggingLevel() > QsLogging::DebugLevel) {} \
    else QsLogging::Logger::Helper(QsLogging::DebugLevel).stream()
#define QLOG_INFO()  \
    if (QsLogging::Logger::instance().loggingLevel() > QsLogging::InfoLevel) {} \
    else QsLogging::Logger::Helper(QsLogging::InfoLevel).stream()
#define QLOG_WARN()  \
    if (QsLogging::Logger::instance().loggingLevel() > QsLogging::WarnLevel) {} \
    else QsLogging::Logger::Helper(QsLogging::WarnLevel).stream()
#define QLOG_ERROR() \
    if (QsLogging::Logger::instance().loggingLevel() > QsLogging::ErrorLevel) {} \
    else QsLogging::Logger::Helper(QsLogging::ErrorLevel).stream()
#define QLOG_FATAL() \
    if (QsLogging::Logger::instance().loggingLevel() > QsLogging::FatalLevel) {} \
    else QsLogging::Logger::Helper(QsLogging::FatalLevel).stream()
#else
#define QLOG_TRACE() \
    if (QsLogging::Logger::instance().loggingLevel() > QsLogging::TraceLevel) {} \
    else  QsLogging::Logger::Helper(QsLogging::TraceLevel).stream() << __FILE__ << '@' << __LINE__
#define QLOG_DEBUG() \
    if (QsLogging::Logger::instance().loggingLevel() > QsLogging::DebugLevel) {} \
    else QsLogging::Logger::Helper(QsLogging::DebugLevel).stream() << __FILE__ << '@' << __LINE__
#define QLOG_INFO()  \
    if (QsLogging::Logger::instance().loggingLevel() > QsLogging::InfoLevel) {} \
    else QsLogging::Logger::Helper(QsLogging::InfoLevel).stream() << __FILE__ << '@' << __LINE__
#define QLOG_WARN()  \
    if (QsLogging::Logger::instance().loggingLevel() > QsLogging::WarnLevel) {} \
    else QsLogging::Logger::Helper(QsLogging::WarnLevel).stream() << __FILE__ << '@' << __LINE__
#define QLOG_ERROR() \
    if (QsLogging::Logger::instance().loggingLevel() > QsLogging::ErrorLevel) {} \
    else QsLogging::Logger::Helper(QsLogging::ErrorLevel).stream() << __FILE__ << '@' << __LINE__
#define QLOG_FATAL() \
    if (QsLogging::Logger::instance().loggingLevel() > QsLogging::FatalLevel) {} \
    else QsLogging::Logger::Helper(QsLogging::FatalLevel).stream() << __FILE__ << '@' << __LINE__
#endif

// Log severity level constants.
#ifdef _WIN32
#undef ERROR
#endif
const QsLogging::Level FATAL   = QsLogging::FatalLevel;
const QsLogging::Level ERROR   = QsLogging::ErrorLevel;
const QsLogging::Level WARNING = QsLogging::WarnLevel;
const QsLogging::Level INFO    = QsLogging::InfoLevel;
const QsLogging::Level DEBUG   = QsLogging::DebugLevel;
const QsLogging::Level TRACE   = QsLogging::TraceLevel;

// support std string
QDebug& operator << (QDebug s, const std::string& m);
QDebug& operator << (QDebug s, const std::basic_string<wchar_t>& m);

// ---------------------- Logging Macro definitions --------------------------

#ifdef __llvm__
#if (defined(__clang__) || (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)))
#pragma GCC diagnostic ignored "-Wdangling-else"
#endif
#endif

// macros take line number, file and function as parameter
#define LTRACEF(file, line, function) \
   if (QsLogging::Logger::instance().loggingLevel() > QsLogging::TraceLevel) {} \
   else QsLogging::Logger::Helper(QsLogging::TraceLevel).stream() << (file) << '@' << (line) << "[" << (function) << "]"
#define LDEBUGF(file, line, function) \
   if (QsLogging::Logger::instance().loggingLevel() > QsLogging::DebugLevel) {} \
   else QsLogging::Logger::Helper(QsLogging::DebugLevel).stream() << (file) << '@' << (line) << "[" << (function) << "]"
#define LINFOF(file, line, function) \
   if (QsLogging::Logger::instance().loggingLevel() > QsLogging::InfoLevel) {} \
   else QsLogging::Logger::Helper(QsLogging::InfoLevel).stream() << (file) << '@' << (line) << "[" << (function) << "]"
#define LWARNF(file, line, function)  \
   if (QsLogging::Logger::instance().loggingLevel() > QsLogging::WarnLevel) {} \
   else QsLogging::Logger::Helper(QsLogging::WarnLevel).stream() << (file) << '@' << (line) << "[" << (function) << "]"
#define LERRORF(file, line, function) \
   if (QsLogging::Logger::instance().loggingLevel() > QsLogging::ErrorLevel) {} \
   else QsLogging::Logger::Helper(QsLogging::ErrorLevel).stream() << (file) << '@' << (line) << "[" << (function) << "]"
#define LFATALF(file, line, function) \
   if (QsLogging::Logger::instance().loggingLevel() > QsLogging::FatalLevel) {} \
   else QsLogging::Logger::Helper(QsLogging::FatalLevel).stream() << (file) << '@' << (line) << "[" << (function) << "]"

#ifndef __GNUC__
#define __PRETTY_FUNCTION__ __FUNCTION__
#endif

// macros with line number
#define LTRACE_LN() \
   if (QsLogging::Logger::instance().loggingLevel() > QsLogging::TraceLevel) {} \
   else QsLogging::Logger::Helper(QsLogging::TraceLevel).stream() << __FILE__ << '@' << __LINE__ << "[" << __PRETTY_FUNCTION__ << "]"
#define LDEBUG_LN() \
   if (QsLogging::Logger::instance().loggingLevel() > QsLogging::DebugLevel) {} \
   else QsLogging::Logger::Helper(QsLogging::DebugLevel).stream() << __FILE__ << '@' << __LINE__ << "[" << __PRETTY_FUNCTION__ << "]"
#define LINFO_LN()  \
   if (QsLogging::Logger::instance().loggingLevel() > QsLogging::InfoLevel) {} \
   else QsLogging::Logger::Helper(QsLogging::InfoLevel).stream() << __FILE__ << '@' << __LINE__ << "[" << __PRETTY_FUNCTION__ << "]"
#define LWARN_LN()  \
   if (QsLogging::Logger::instance().loggingLevel() > QsLogging::WarnLevel) {} \
   else QsLogging::Logger::Helper(QsLogging::WarnLevel).stream() << __FILE__ << '@' << __LINE__ << "[" << __PRETTY_FUNCTION__ << "]"
#define LERROR_LN() \
   if (QsLogging::Logger::instance().loggingLevel() > QsLogging::ErrorLevel) {} \
   else QsLogging::Logger::Helper(QsLogging::ErrorLevel).stream() << __FILE__ << '@' << __LINE__ << "[" << __PRETTY_FUNCTION__ << "]"
#define LFATAL_LN() \
   if (QsLogging::Logger::instance().loggingLevel() > QsLogging::FatalLevel) {} \
   else QsLogging::Logger::Helper(QsLogging::FatalLevel).stream() << __FILE__ << '@' << __LINE__ << "[" << __PRETTY_FUNCTION__ << "]"

// macros without line number
#define LTRACE_NLN() \
   if (QsLogging::Logger::instance().loggingLevel() > QsLogging::TraceLevel) {} \
   else  QsLogging::Logger::Helper(QsLogging::TraceLevel).stream()
#define LDEBUG_NLN() \
   if (QsLogging::Logger::instance().loggingLevel() > QsLogging::DebugLevel) {} \
   else QsLogging::Logger::Helper(QsLogging::DebugLevel).stream()
#define LINFO_NLN()  \
   if (QsLogging::Logger::instance().loggingLevel() > QsLogging::InfoLevel) {} \
   else QsLogging::Logger::Helper(QsLogging::InfoLevel).stream()
#define LWARN_NLN()  \
   if (QsLogging::Logger::instance().loggingLevel() > QsLogging::WarnLevel) {} \
   else QsLogging::Logger::Helper(QsLogging::WarnLevel).stream()
#define LERROR_NLN() \
   if (QsLogging::Logger::instance().loggingLevel() > QsLogging::ErrorLevel) {} \
   else QsLogging::Logger::Helper(QsLogging::ErrorLevel).stream()
#define LFATAL_NLN() \
   if (QsLogging::Logger::instance().loggingLevel() > QsLogging::FatalLevel) {} \
   else QsLogging::Logger::Helper(QsLogging::FatalLevel).stream()

#ifdef QS_LOG_LINE_NUMBERS
#define LTRACE() LTRACE_LN()
#define LDEBUG() LDEBUG_LN()
// need line number for info level log?
#define LINFO() LINFO_NLN()
#define LWARN() LWARN_LN()
#define LERROR() LERROR_LN()
#define LFATAL() LFATAL_LN()
#else
#define LTRACE() LTRACE_NLN()
#define LDEBUG() LDEBUG_NLN()
#define LINFO() LINFO_NLN()
#define LWARN() LWARN_NLN()
#define LERROR() LERROR_NLN()
#define LFATAL() LFATAL_NLN()
#endif

// glog style

#define LOG(n) \
  if (QsLogging::Logger::instance().loggingLevel() > n) {} \
  else QsLogging::Logger::Helper(n).stream() << __FILE__ << '@' << __LINE__ << "[" << __PRETTY_FUNCTION__ << "]"

#define VLOG_IS_ON(n) true

#define VLOG(n) \
  if (QsLogging::Logger::instance().loggingLevel() > (QsLogging::Level)std::max(0,2-n)) {} \
  else QsLogging::Logger::Helper((QsLogging::Level)std::max(0,2-n)).stream() << __FILE__ << '@' << __LINE__ << "[" << __PRETTY_FUNCTION__ << "]"

// ---------------------------- CHECK helpers --------------------------------

#define LOG_IF(severity, condition) \
  if (!(condition)) {} \
  else LOG(severity)

// ---------------------------- CHECK macros ---------------------------------

// Check for a given boolean condition.
#define CHECK(condition) LOG_IF(FATAL, !(condition)) \
        << "Check failed: " #condition " "

#ifdef _DEBUG_
// Debug only version of CHECK
#define DCHECK(condition) LOG_IF(FATAL, !(condition)) \
        << "Check failed: " #condition " "
#else
// Optimized version - generates no code.
#define DCHECK(condition) if (false) LOG_IF(FATAL, !(condition)) \
        << "Check failed: " #condition " "
#endif  // _DEBUG_

// ------------------------- CHECK_OP macros ---------------------------------

// Generic binary operator check macro. This should not be directly invoked,
// instead use the binary comparison macros defined below.
#define CHECK_OP(val1, val2, op) LOG_IF(FATAL, !(val1 op val2)) \
  << "Check failed: " #val1 " " #op " " #val2 " "

// Check_op macro definitions
#define CHECK_EQ(val1, val2) CHECK_OP(val1, val2, ==)
#define CHECK_NE(val1, val2) CHECK_OP(val1, val2, !=)
#define CHECK_LE(val1, val2) CHECK_OP(val1, val2, <=)
#define CHECK_LT(val1, val2) CHECK_OP(val1, val2, <)
#define CHECK_GE(val1, val2) CHECK_OP(val1, val2, >=)
#define CHECK_GT(val1, val2) CHECK_OP(val1, val2, >)

#ifdef _DEBUG_
// Debug only versions of CHECK_OP macros.
#define DCHECK_EQ(val1, val2) CHECK_OP(val1, val2, ==)
#define DCHECK_NE(val1, val2) CHECK_OP(val1, val2, !=)
#define DCHECK_LE(val1, val2) CHECK_OP(val1, val2, <=)
#define DCHECK_LT(val1, val2) CHECK_OP(val1, val2, <)
#define DCHECK_GE(val1, val2) CHECK_OP(val1, val2, >=)
#define DCHECK_GT(val1, val2) CHECK_OP(val1, val2, >)
#else
// These versions generate no code in optimized mode.
#define DCHECK_EQ(val1, val2) if (false) CHECK_OP(val1, val2, ==)
#define DCHECK_NE(val1, val2) if (false) CHECK_OP(val1, val2, !=)
#define DCHECK_LE(val1, val2) if (false) CHECK_OP(val1, val2, <=)
#define DCHECK_LT(val1, val2) if (false) CHECK_OP(val1, val2, <)
#define DCHECK_GE(val1, val2) if (false) CHECK_OP(val1, val2, >=)
#define DCHECK_GT(val1, val2) if (false) CHECK_OP(val1, val2, >)
#endif  // _DEBUG_

// ---------------------------CHECK_NOTNULL macros ---------------------------

// Helpers for CHECK_NOTNULL(). Two are necessary to support both raw pointers
// and smart pointers.
template <typename T>
T& CheckNotNullCommon(const char *file, int line, const char *names, T& t) {
  if (t == NULL) {
    LFATALF(file, line, names) << "";
  }
  return t;
}

template <typename T>
T* CheckNotNull(const char *file, int line, const char *names, T* t) {
  return CheckNotNullCommon(file, line, names, t);
}

template <typename T>
T& CheckNotNull(const char *file, int line, const char *names, T& t) {
  return CheckNotNullCommon(file, line, names, t);
}

// Check that a pointer is not null.
#define CHECK_NOTNULL(val) \
  CheckNotNull(__FILE__, __LINE__, "'" #val "' Must be non NULL", (val))

#ifdef _DEBUG_
// Debug only version of CHECK_NOTNULL
#define DCHECK_NOTNULL(val) \
  CheckNotNull(__FILE__, __LINE__, "'" #val "' Must be non NULL", (val))
#else
// Optimized version - generates no code.
#define DCHECK_NOTNULL(val) if (false)\
  CheckNotNull(__FILE__, __LINE__, "'" #val "' Must be non NULL", (val))
#endif  // _DEBUG_


// container

template<class IteratorType>
void logContainer(QsLogging::Level severity, const IteratorType &begin, const IteratorType &end,
                  int numberPerLine, const QString &name = "")
{
  if (QsLogging::Logger::instance().loggingLevel() > severity)
    return;
  if (severity == INFO)
    LINFO() << "Start container" << name;
  else
    LOG(severity) << "Start container" << name;
  IteratorType it = begin;
  while (it != end) {
    QsLogging::Logger::Helper helper(severity);
    int num = 0;
    while (num < numberPerLine && it != end) {
      helper.stream() << *it;
      ++it;
      ++num;
    }
  }
  if (severity == INFO)
    LINFO() << "End container" << name;
  else
    LOG(severity) << "End container" << name ;
}

#ifdef QS_LOG_DISABLE
#include "QsLogDisableForThisFile.h"
#endif

#endif // QSLOG_H
