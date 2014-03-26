#ifndef ZQTBARPROGRESSREPORTER_H
#define ZQTBARPROGRESSREPORTER_H

#include <QProgressBar>
#include "zprogressreporter.h"

class ZQtBarProgressReporter : public ZProgressReporter
{
public:
  ZQtBarProgressReporter();

public:
  inline void setProgressBar(QProgressBar *progressBar) {
    m_progressBar = progressBar;
  }

  virtual void open();
  virtual void close();
  virtual void push();
  virtual void pull();

private:
  QProgressBar *m_progressBar;
};

#endif // ZQTBARPROGRESSREPORTER_H
