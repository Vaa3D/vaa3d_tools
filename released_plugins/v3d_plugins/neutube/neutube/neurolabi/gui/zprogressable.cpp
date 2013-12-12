#include "zprogressable.h"

ZProgressable::ZProgressable()
{
  setDefaultProgressReporter();
}

ZProgressable::~ZProgressable()
{

}

void ZProgressable::destroyProgressReporter()
{
  if (m_progressReporter != &m_defaultProgressReporter) {
    delete m_progressReporter;
    setDefaultProgressReporter();
  }
}

void ZProgressable::setDefaultProgressReporter()
{
  m_progressReporter = &m_defaultProgressReporter;
}

void ZProgressable::setProgressReporter(ZProgressReporter *reporter)
{
   if (reporter == 0) {
     setDefaultProgressReporter();
   } else {
     m_progressReporter = reporter;
   }
 }

void ZProgressable::startProgress()
{
  m_progressReporter->start();
}

void ZProgressable::endProgress()
{
  m_progressReporter->end();
}

void ZProgressable::startProgress(double scale)
{
  m_progressReporter->start(scale);
}

void ZProgressable::endProgress(double scale)
{
  m_progressReporter->end(scale);
}

void ZProgressable::advanceProgress(double dp)
{
  m_progressReporter->advance(dp);
}
