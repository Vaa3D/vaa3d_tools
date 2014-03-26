#include "zreportable.h"

ZReportable::ZReportable()
{
  m_reporter = &m_defaultReporter;
}

ZReportable::~ZReportable()
{
}

void ZReportable::destroyReporter()
{
  if (m_reporter != &m_defaultReporter) {
    delete m_reporter;
    m_reporter = &m_defaultReporter;
  }
}

void ZReportable::report(const std::string &title, const std::string &msg,
                        ZMessageReporter::EMessageType msgType)
{
  m_reporter->report(title, msg, msgType);
}
