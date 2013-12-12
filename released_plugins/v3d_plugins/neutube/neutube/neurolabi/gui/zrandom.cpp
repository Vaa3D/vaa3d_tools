#include "zrandom.h"
#include <qglobal.h>
#if (QT_VERSION >= QT_VERSION_CHECK(4, 8, 0))
#include <QThreadStorage>
#define QTHREAD_STORAGE_SUPPORTED 1
#endif
#include "QsLog.h"

#if defined(_WIN32) || defined(_WIN64)

#include <intrin.h>
#define rdtsc  __rdtsc

#else

//  For everything else
static unsigned long long rdtsc() {
  unsigned int lo,hi;
  __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
  return ((unsigned long long)hi << 32) | lo;
}

#endif

ZRandom::ZRandom()
  :m_size(-2), m_idx(-1)
{
  m_eng.seed(rdtsc());
}

ZRandom &ZRandom::getInstance()
{
  // should be thread local, use qt or use boost thread_specific_ptr or wait for c++11 thread_local keyword
#if defined(QTHREAD_STORAGE_SUPPORTED)
  static QThreadStorage<ZRandom> globalZRandom;
  return globalZRandom.localData();
#else
  static ZRandom obj;
  return obj;
#endif
}

void ZRandom::uniqueRandInit(int maxValue, int minValue)
{
#if defined(QTHREAD_STORAGE_SUPPORTED)
  assert(maxValue >= minValue);
  static QThreadStorage<RandomGeneratorForShuffle*> rg;    // in c++11, we could use m_eng directly
  if (!rg.hasLocalData())
    rg.setLocalData(new RandomGeneratorForShuffle(*this));
  m_uniqueValues.resize(maxValue-minValue+1);
  m_size = maxValue-minValue+1;
  for (int i = 0; i < m_size; i++)
    m_uniqueValues[i] = i + minValue;
  std::random_shuffle(m_uniqueValues.begin(), m_uniqueValues.end(), *rg.localData());
  m_idx = 0;
#endif
}

// must call uniqueRandInit first!!
int ZRandom::uniqueRandNext()
{
  if (m_idx == m_size) {
    LERROR() << "No more number.";
    return std::numeric_limits<int>::min();
  }
  else
    return m_uniqueValues[m_idx++];
}
