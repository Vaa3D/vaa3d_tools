#ifndef ZBENCHTIMER_H
#define ZBENCHTIMER_H

/* usage:
  bench with repeats:
    ZBenchTimer bt;
    BENCHANDPRINT(bt,10,5,testFun());
  bench without repeats:
    ZBenchTimer bt;
    bt.start();
    testFun();
    bt.stopAndPrint();
  */

//This header has to be ahead of windows.h to avoid macro conflict (ERROR)
#include "QsLog.h"

#if defined(_WIN32) || defined(_WIN64)
# include <windows.h>
#elif defined(__APPLE__) && defined(__MACH__)
#include <mach/mach_time.h>
#else
# include <unistd.h>
# include <time.h>
#endif

#include <iostream>
#include <limits>
#include <sstream>


#define BENCH(TIMER,TRIES,REP,CODE,FUNCNAME) { \
  TIMER.reset(); \
  TIMER.setName(FUNCNAME); \
  for(int i=0; i<TRIES; ++i){ \
    TIMER.start(); \
    for(int j=0; j<REP; ++j){ \
      CODE; \
    } \
    TIMER.stop(); \
  } \
}

#define BENCHANDPRINT(TIMER,TRIES,REP,CODE,FUNCNAME) { \
  TIMER.reset(); \
  TIMER.setName(FUNCNAME); \
  for(int i=0; i<TRIES; ++i){ \
    TIMER.start(); \
    for(int j=0; j<REP; ++j){ \
      CODE; \
    } \
    TIMER.stop(); \
  } \
  TIMER.print(); \
}

class ZBenchTimer
{
public:
  ZBenchTimer(const std::string &funName = "");

  inline void reset()
  {
    m_best = std::numeric_limits<double>::max();
    m_worst = -1;
    m_total = 0;
    m_rep = 0;
    m_time = 0;
    m_pauseTime = 0;
    m_totalPauseTime = 0;
    m_paused = false;
    m_average = 0;
    m_averagePauseTime = 0;
  }

  inline void start()
  {
    m_time = 0.0;
    m_pauseTime = 0.0;
    m_paused = false;
    m_start = getCpuTicks();
  }

  void stop();

  void pause();

  void resume();

  inline void stopAndPrint(std::ostream& s = std::cout)
  {
    stop();
    print(s);
  }

  // elapsed time in seconds
  inline double time() { return m_time; }

  // average elapsed time in seconds.
  inline double average() { return m_average; }

  // best elapsed time in seconds
  inline double best() { return m_best; }

  // total elapsed time in seconds.
  inline double total() { return m_total; }

  // elapsed pause time in seconds
  inline double pauseTime() { return m_pauseTime; }

  // total elapsed pause time in seconds.
  inline double totalPauseTime() { return m_totalPauseTime; }

  inline void setName(const std::string &str) { m_name = str; }

  std::ostream& print(std::ostream& s = std::cout) const;

#if defined(_WIN64) || defined(_WIN32)
  inline LARGE_INTEGER getCpuTicks()
  {
    LARGE_INTEGER queryTicks;
    QueryPerformanceCounter(&queryTicks);
    return queryTicks;
  }
#elif defined(__APPLE__) && defined(__MACH__)
  inline uint64_t getCpuTicks()
  {
    uint64_t tm;
    tm = mach_absolute_time();
    return tm;
  }
#else
  inline timespec getCpuTicks()
  {
    timespec ts;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
    return ts;
  }
#endif

protected:
#if defined(_WIN32) || defined(_WIN64)
  double m_frequency;
  LARGE_INTEGER m_start;
#elif defined(__APPLE__) && defined(__MACH__)
  uint64_t m_start;
#else
  timespec m_start;
#endif
  double m_time;
  double m_best;
  double m_worst;
  double m_average;
  int m_rep;
  double m_total;
  std::string m_name;

  double m_pauseTime;
  double m_totalPauseTime;
  double m_averagePauseTime;
  bool m_paused;
};

inline std::ostream& operator <<(std::ostream & s, const ZBenchTimer& m)
{
  return m.print(s);
}

// qDebug output
QDebug& operator << (QDebug s, const ZBenchTimer& m);


#endif // ZBENCHTIMER_H
