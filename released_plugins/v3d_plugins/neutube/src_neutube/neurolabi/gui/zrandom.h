#ifndef ZRANDOM_H
#define ZRANDOM_H

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/uniform_real_distribution.hpp>
#include <vector>

#define ZRandomInstance ZRandom::getInstance()
// always use ZRandomInstance or getInstance() to get the static instance of ZRandom, one engine is enough

// all these boost fun will be supported by c++11
class ZRandom
{
public:
  ZRandom();

  static ZRandom& getInstance();

  inline int randInt(int maxValue = std::numeric_limits<int>::max(), int minValue = 0)
  {
    return randIntType(maxValue, minValue);
  }

  template<typename IntType>
  inline IntType randIntType(IntType maxValue = std::numeric_limits<IntType>::max(), IntType minValue = 0)
  {
    boost::random::uniform_int_distribution<IntType> dist(minValue, maxValue);
    return dist(m_eng);
  }

  template<typename Real>
  inline Real randReal(Real maxValue = 1.0, Real minValue = 0.0)
  {
    boost::random::uniform_real_distribution<Real> dist(minValue, maxValue);
    return dist(m_eng);
  }

  inline double randDouble(double maxValue = 1.0, double minValue = 0.0)
  {
    return randReal(maxValue, minValue);
  }

  void uniqueRandInit(int maxValue, int minValue = 0);

  // must call uniqueRandInit first!!
  int uniqueRandNext();

private:
  boost::mt19937 m_eng;
  struct RandomGeneratorForShuffle
  {
    RandomGeneratorForShuffle(ZRandom& ref) : m_ref(ref) {}
    ZRandom &m_ref;
    ptrdiff_t operator() (ptrdiff_t i) { return m_ref.randInt(i); }
  };
  // for unique random
  std::vector<int> m_uniqueValues;
  int m_size;
  int m_idx;
};

#endif // ZRANDOM_H
