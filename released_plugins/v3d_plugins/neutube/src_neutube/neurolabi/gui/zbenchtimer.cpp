#include "zbenchtimer.h"

ZBenchTimer::ZBenchTimer(const std::string &funName)
{
#if defined(_WIN32) || defined(_WIN64)
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);
    m_frequency = (double)freq.QuadPart;
#endif
    reset();
    if (!funName.empty()) {
      m_name = funName.substr(funName.find_first_not_of(" \t"));
    }
}

void ZBenchTimer::stop()
{
#if defined(_WIN64) || defined(_WIN32)
  LARGE_INTEGER stop = getCpuTicks();
  if (m_paused)
    m_pauseTime += ((stop.QuadPart - m_start.QuadPart) / m_frequency);
  else
    m_time += ((stop.QuadPart - m_start.QuadPart) / m_frequency);
#elif defined(__APPLE__) && defined(__MACH__)
  uint64_t elapsed = getCpuTicks() - m_start;
  static mach_timebase_info_data_t sTimebaseInfo = {0,0};
  if (sTimebaseInfo.denom == 0)
    (void) mach_timebase_info(&sTimebaseInfo);
  if (m_paused)
    m_pauseTime += 1e-9 * elapsed * sTimebaseInfo.numer / sTimebaseInfo.denom;
  else
    m_time +=  1e-9 * elapsed * sTimebaseInfo.numer / sTimebaseInfo.denom;
#else
  timespec end = getCpuTicks();
  if ((end.tv_nsec - m_start.tv_nsec)<0) {
    end.tv_sec = end.tv_sec - m_start.tv_sec - 1;
    end.tv_nsec = 1000000000 + end.tv_nsec - m_start.tv_nsec;
  } else {
    end.tv_sec -= m_start.tv_sec;
    end.tv_nsec -= m_start.tv_nsec;
  }
  if (m_paused)
    m_pauseTime += double(end.tv_sec) + 1e-9 * double(end.tv_nsec);
  else
    m_time += double(end.tv_sec) + 1e-9 * double(end.tv_nsec);
#endif
  m_paused = false;

  m_best = std::min(m_best, m_time);
  m_worst = std::max(m_worst, m_time);
  m_rep++;
  m_total += m_time;
  m_average = m_total / m_rep;
  m_totalPauseTime += m_pauseTime;
  m_averagePauseTime = m_totalPauseTime / m_rep;
}

void ZBenchTimer::pause()
{
  if (m_paused)
    return;

#if defined(_WIN64) || defined(_WIN32)
  LARGE_INTEGER stop = getCpuTicks();
  m_time += ((stop.QuadPart - m_start.QuadPart) / m_frequency);
#elif defined(__APPLE__) && defined(__MACH__)
  uint64_t elapsed = getCpuTicks() - m_start;
  static mach_timebase_info_data_t sTimebaseInfo = {0,0};
  if (sTimebaseInfo.denom == 0)
    (void) mach_timebase_info(&sTimebaseInfo);
  m_time +=  1e-9 * elapsed * sTimebaseInfo.numer / sTimebaseInfo.denom;
#else
  timespec end = getCpuTicks();
  if ((end.tv_nsec - m_start.tv_nsec)<0) {
    end.tv_sec = end.tv_sec - m_start.tv_sec - 1;
    end.tv_nsec = 1000000000 + end.tv_nsec - m_start.tv_nsec;
  } else {
    end.tv_sec -= m_start.tv_sec;
    end.tv_nsec -= m_start.tv_nsec;
  }
  m_time += double(end.tv_sec) + 1e-9 * double(end.tv_nsec);
#endif

  m_paused = true;
  m_start = getCpuTicks();
}

void ZBenchTimer::resume()
{
  if (!m_paused)
    return;

#if defined(_WIN64) || defined(_WIN32)
  LARGE_INTEGER stop = getCpuTicks();
  m_pauseTime += ((stop.QuadPart - m_start.QuadPart) / m_frequency);
#elif defined(__APPLE__) && defined(__MACH__)
  uint64_t elapsed = getCpuTicks() - m_start;
  static mach_timebase_info_data_t sTimebaseInfo = {0,0};
  if (sTimebaseInfo.denom == 0)
    (void) mach_timebase_info(&sTimebaseInfo);
  m_pauseTime +=  1e-9 * elapsed * sTimebaseInfo.numer / sTimebaseInfo.denom;
#else
  timespec end = getCpuTicks();
  if ((end.tv_nsec - m_start.tv_nsec)<0) {
    end.tv_sec = end.tv_sec - m_start.tv_sec - 1;
    end.tv_nsec = 1000000000 + end.tv_nsec - m_start.tv_nsec;
  } else {
    end.tv_sec -= m_start.tv_sec;
    end.tv_nsec -= m_start.tv_nsec;
  }
  m_pauseTime += double(end.tv_sec) + 1e-9 * double(end.tv_nsec);
#endif

  m_paused = false;
  m_start = getCpuTicks();
}

std::ostream& ZBenchTimer::print(std::ostream& s) const
{
  if (m_rep == 1) {
    s << "Function " << m_name << " took " << m_time << " seconds." << std::endl;
    if (m_pauseTime > 0)
      s << "Function " << m_name << " pause " << m_pauseTime << " seconds." << std::endl;
  } else if (m_rep > 1) {
    s << "Function " << m_name << " took on average " << m_average << " seconds.";
    s << " (out of " << m_rep << " repeats. best: " << m_best << "  worst: ";
    s << m_worst << ")" << std::endl;
    if (m_averagePauseTime > 0)
      s << "Function " << m_name << " pause on average " << m_averagePauseTime << " seconds." << std::endl;
  }
  s.flush();
  return s;
}

// qDebug output
QDebug& operator << (QDebug s, const ZBenchTimer& m)
{
  std::ostringstream oss;
  oss << m;
  std::string outstr = oss.str();
  // remove newline at last
  outstr.erase(outstr.end()-1);
  s.nospace() << outstr.c_str();
  return s.space();
}
