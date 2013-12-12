#include "zprogressreporter.h"
#include <iostream>

ZProgressReporter::ZProgressReporter() : m_progress(0), m_scale(1.0),
  m_sectionCount(0)
{
}

void ZProgressReporter::start()
{
  m_sectionCount++;
  if (m_sectionCount == 1) {
    open();
    m_progress = 0;
    m_scale = 1.0;
    push();
  }
}

void ZProgressReporter::end()
{
  m_sectionCount--;
  if (m_sectionCount == 0) {
    close();
  }
}

void ZProgressReporter::open()
{

}

void ZProgressReporter::close()
{

}

void ZProgressReporter::update(double progress)
{
  m_progress = progress;
  push();
}



void ZProgressReporter::push()
{
#ifdef _DEBUG_
  std::cout << "Progress: " << m_progress << std::endl;
#endif
}

void ZProgressReporter::pull()
{

}

void ZProgressReporter::start(double scale)
{
  startSubprogress(scale);
  ++m_sectionCount;
}

void ZProgressReporter::end(double scale)
{
  endSubprogress(scale);
  --m_sectionCount;
}

void ZProgressReporter::startSubprogress(double scale)
{
  m_scale *= scale;
}

void ZProgressReporter::endSubprogress(double scale)
{
  m_scale /= scale;
}

void ZProgressReporter::advance(double dp)
{
  m_progress += m_scale * dp;
  if (m_progress > 1.0) {
    m_progress = 1.0;
  }

  push();
}

/*
void ZProgressReporter::advanceTo(double progress)
{
  m_progress = progress;
  push();
}
*/
