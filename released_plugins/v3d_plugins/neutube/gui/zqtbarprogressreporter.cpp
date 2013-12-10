#include "zqtbarprogressreporter.h"

#include <QApplication>

#include "tz_math.h"

ZQtBarProgressReporter::ZQtBarProgressReporter()
{
  m_progressBar = NULL;
}

void ZQtBarProgressReporter::open()
{
  if (m_progressBar != NULL) {
    m_progressBar->setRange(0, 100);
    m_progressBar->show();
  }
}

void ZQtBarProgressReporter::close()
{
  if (m_progressBar != NULL) {
    m_progressBar->reset();
    m_progressBar->hide();
  }
}

void ZQtBarProgressReporter::push()
{
  m_progressBar->setValue(iround(m_progress * 100.0));
  QApplication::processEvents();
}

void ZQtBarProgressReporter::pull()
{
  m_progress = 0.01 * m_progressBar->value();
}
