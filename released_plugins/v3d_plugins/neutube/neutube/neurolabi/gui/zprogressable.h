#ifndef ZPROGRESSABLE_H
#define ZPROGRESSABLE_H

#include "zprogressreporter.h"

class ZProgressable
{
public:
  ZProgressable();
  virtual ~ZProgressable();

  void setProgressReporter(ZProgressReporter *reporter);
  void setDefaultProgressReporter();
  void destroyProgressReporter();

  void startProgress();
  void endProgress();
  void startProgress(double scale);
  void endProgress(double scale);
  void advanceProgress(double dp);

  inline ZProgressReporter* getProgressReporter() { return m_progressReporter; }

protected:
  ZProgressReporter m_defaultProgressReporter;
  ZProgressReporter *m_progressReporter;
};

#endif // ZPROGRESSABLE_H
