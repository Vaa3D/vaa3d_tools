#ifndef ZSTATISTICSUTILS_H
#define ZSTATISTICSUTILS_H

#include <algorithm>
#include <cmath>
#include <numeric>
#include <cassert>
#include <QList>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QtConcurrent/QtConcurrentMap>
#else
#include <QtConcurrentMap>
#endif

#include <utility>

#define MULTITHREAD_THRESHOLD 1e8

template <typename RandomAccessIterator>
struct MinMaxElementMap {

  typedef typename std::pair<RandomAccessIterator,RandomAccessIterator> result_type;

  std::pair<RandomAccessIterator,RandomAccessIterator>
  operator()(const std::pair<RandomAccessIterator,RandomAccessIterator> &range) const {
    std::pair<RandomAccessIterator,RandomAccessIterator> res;
    res.first = range.first;
    res.second = range.first;
    for (RandomAccessIterator it = range.first; it != range.second; ++it) {
      if (*it < *res.first)
        res.first = it;
      if (*it > *res.second)
        res.second = it;
    }
    return res;
  }
};

template <typename RandomAccessIterator>
std::pair<RandomAccessIterator,RandomAccessIterator>
minMaxElement(RandomAccessIterator begin, RandomAccessIterator end, bool multithread = true)
{
  assert(end > begin);
  MinMaxElementMap<RandomAccessIterator> func;
  typedef typename std::pair<RandomAccessIterator,RandomAccessIterator> ResultType;
  if (!multithread || end-begin < MULTITHREAD_THRESHOLD) {
    return func(std::make_pair(begin, end));
  } else {
    size_t size = end - begin;
    size_t numBlock = std::min(size, (size_t)QThread::idealThreadCount()*2);
    size_t pixelPerBlock = size / numBlock;
    QList<std::pair<RandomAccessIterator,RandomAccessIterator> > allRange;
    for (size_t i=0; i<numBlock; ++i) {
      allRange.push_back(std::make_pair(begin+i*pixelPerBlock,
                                        (i==numBlock-1) ? end : begin+(i+1)*pixelPerBlock));
    }

    QList<ResultType> values =  QtConcurrent::blockingMapped(allRange, func);
    typename QList<ResultType>::iterator it = values.begin();
    ResultType res = *it;
    ++it;
    for (; it != values.end(); ++it) {
      if (*(it->first) < *res.first)
        res.first = it->first;
      if (*(it->second) > *res.second)
        res.second = it->second;
    }
    return res;
  }
}

template<typename RandomAccessIterator, typename ResultType>
struct SumRangeMap {

  typedef ResultType result_type;

  ResultType
  operator()(const std::pair<RandomAccessIterator,RandomAccessIterator> &range) const {
    return std::accumulate(range.first, range.second, ResultType(0));
  }
};

template<typename RandomAccessIterator, typename ResultType>
ResultType sumRange(RandomAccessIterator begin, RandomAccessIterator end, ResultType init, bool multithread = true)
{
  if (!multithread || end-begin < MULTITHREAD_THRESHOLD)
    return std::accumulate(begin, end, init);
  else {
    assert(end > begin);
    size_t size = end - begin;
    size_t numBlock = std::min(size, (size_t)QThread::idealThreadCount()*2);
    size_t pixelPerBlock = size / numBlock;
    QList<std::pair<RandomAccessIterator,RandomAccessIterator> > allRange;
    for (size_t i=0; i<numBlock; ++i) {
      allRange.push_back(std::make_pair(begin+i*pixelPerBlock,
                                        (i==numBlock-1) ? end : begin+(i+1)*pixelPerBlock));
    }

    QList<ResultType> values =  QtConcurrent::blockingMapped(allRange,
                                                             SumRangeMap<RandomAccessIterator, ResultType>());
    return std::accumulate(values.begin(), values.end(), init);
  }
}

template <class RandomAccessIterator>
double mean(RandomAccessIterator begin, RandomAccessIterator end, bool multithread = true)
{
  //typedef typename std::iterator_traits<RandomAccessIterator>::value_type ValueType;
  //typedef typename NumTraits<ValueType>::NonInteger ResultType;
  typedef double ResultType;

  ResultType sum = sumRange(begin, end, static_cast<ResultType>(0), multithread);
  return sum / (end-begin);
}

template<typename RandomAccessIterator, typename DiffIterator, typename ResultType>
struct StandardDeviationMap {
  StandardDeviationMap(RandomAccessIterator begin, DiffIterator diffbegin, ResultType mean)
    : begin(begin), diffbegin(diffbegin), meanV(mean)
  {}

  typedef ResultType result_type;

  ResultType
  operator()(const std::pair<size_t,size_t> &range) const {
    std::transform(begin+range.first, begin+range.second, diffbegin+range.first,
                   std::bind2nd(std::minus<double>(), meanV));
    return std::inner_product(diffbegin+range.first, diffbegin+range.second, diffbegin+range.first, 0.0);
  }

  RandomAccessIterator begin;
  DiffIterator diffbegin;
  ResultType meanV;
};

template <class RandomAccessIterator>
void meanAndStandardDeviation(RandomAccessIterator begin, RandomAccessIterator end,
                              double &meanV,
                              double &stdV,
                              bool bias = false,
                              bool multithread = true)
{
  typedef double ResultType;
  assert(end > begin);
  size_t size = end - begin;

  std::vector<ResultType> diff(size);
  meanV = mean(begin, end, multithread);
  ResultType sq_sum;
  if (!multithread || end-begin < MULTITHREAD_THRESHOLD) {
    std::transform(begin, end, diff.begin(),
                   std::bind2nd(std::minus<double>(), meanV));
    sq_sum = std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
  } else {
    size_t numBlock = std::min(size, (size_t)QThread::idealThreadCount()*2);
    size_t pixelPerBlock = size / numBlock;
    QList<std::pair<size_t,size_t> > allRange;
    for (size_t i=0; i<numBlock; ++i) {
      allRange.push_back(std::make_pair(i*pixelPerBlock,
                                        (i==numBlock-1) ? size : (i+1)*pixelPerBlock));
    }

    StandardDeviationMap<RandomAccessIterator,std::vector<ResultType>::iterator,ResultType> func(
      begin,
      diff.begin(),
      meanV);
    QList<ResultType> values =  QtConcurrent::blockingMapped(allRange, func);
    sq_sum = std::accumulate(values.begin(), values.end(), ResultType(0));
  }
  if (bias || size <= 1) {
    stdV = std::sqrt(sq_sum / size);
  } else {
    stdV = std::sqrt(sq_sum / (size - 1));
  }
}

template <class RandomAccessIterator>
double standardDeviation(RandomAccessIterator begin, RandomAccessIterator end, bool bias = false,
                         bool multithread = true)
{
  double meanV;
  double stdV;
  meanAndStandardDeviation(begin, end, meanV, stdV, bias, multithread);
  return stdV;
}

// if inplace is true, original container will be changed
template <class RandomAccessIterator>
double median(RandomAccessIterator begin, RandomAccessIterator end, bool inplace = false)
{
  typedef typename std::iterator_traits<RandomAccessIterator>::value_type ValueType;
  //typedef typename NumTraits<ValueType>::NonInteger ResultType;
  typedef double ResultType;
  assert(end > begin);
  size_t size = end - begin;
  size_t middleIdx = size/2;
  if (inplace) {
    RandomAccessIterator target = begin + middleIdx;
    std::nth_element(begin, target, end);

    if(size % 2 != 0){ //Odd number of elements
      return (ResultType)*target;
    } else{            //Even number of elements
      ResultType a = *target;
      return (a + *std::max_element(begin, target))/2.0;
    }
  } else {
    std::vector<ValueType> vec;
    vec.insert(vec.end(), begin, end);
    typename std::vector<ValueType>::iterator target = vec.begin() + middleIdx;
    std::nth_element(vec.begin(), target, vec.end());

    if(size % 2 != 0){ //Odd number of elements
      return (ResultType)*target;
    } else{            //Even number of elements
      ResultType a = *target;
      return (a + *std::max_element(vec.begin(), target))/2.0;
    }
  }
}

#endif // ZSTATISTICSUTILS_H
