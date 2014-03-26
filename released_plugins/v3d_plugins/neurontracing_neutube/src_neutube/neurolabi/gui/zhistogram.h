#ifndef ZHISTOGRAM_H
#define ZHISTOGRAM_H

#include <vector>

class ZHistogram
{
public:
  ZHistogram(double start = 0.0, double interval = 1.0);

  /*!
   * \brief Add data point
   *
   * Add the count of the bin containing \a x
   *
   *\param x Data point to add
   */
  void increment(double x);
  void addCount(double x, double dv);

  double getCount(double x) const;

  /*!
   * \brief Get the count in an arbitary bin
   *
   * \return The count betwee [x1, x2]. Returns 0 if x1 >= x2.
   */
  double getCount(double x1, double x2) const;

  /*!
   * \brief Density
   * \param x
   * \return count / interval_of_bin
   */
  double getDensity(double x) const;

  inline void setInterval(double interval) {
    m_interval = interval;
  }

  inline void setStart(double start) {
    m_start = start;
  }

  int getBinIndex(double x) const;

  /*!
   * \brief Clear the histogram.
   *
   * All counts are set to zero, but the interval and start point remain
   * unchanged.
   *
   */
  void clear();

  /*!
   * \brief Test if a histogram is empty.
   *
   * The histogram is empty iff the total count is 0.
   *
   */
  bool isEmpty() const;

  /*!
   * \brief Normalize the histogram to [0, 1].
   *
   * The maximum count of the histogram is scaled to 1 after normalization if
   * there is any non-zero count. Nothing is done if the histogram is empty.
   */
  void normalize();

  /*!
   * \brief Get the maximum count
   *
   * The bin index is stored in \a index if \a index is not NULL.
   *
   * \return The maximum count of the histogram
   */
  double getMaxCount(int *index = 0x0) const;

  /*!
   * \brief Print the histogram.
   */
  void print() const;

  /*!
   * \brief += operator
   *
   * Add two histograms. The bin interval is kept and the start point of the
   * histogram will also be reset if the start of \a hist is smaller.
   * This means that hist1 += hist2 and hist2 += hist1 may not result in the
   * same values.
   */
  ZHistogram& operator+= (const ZHistogram &hist);

  /*!
   * \brief Get an upper bound of the histogram
   *
   * An upper bound is a value v such that for any x > v, the density at x
   * is 0.
   *
   * \return An upper bound.
   */
  double getUpperBound() const;

  /*!
   * \brief Get the center of a bin.
   *
   * It returns a center even if \a binIndex is not within the recorded range.
   *
   * \return The center of a bin.
   */
  double getBinCenter(int binIndex) const;


  /*!
   * \brief Reset bins with a new interval.
   *
   * Nothing is done if \a interval is less than 0.
   *
   * \param interval The new interval.
   */
  void rebin(double interval);

  /*!
   * \brief Get start value of a bin.
   * \param index It can be negative.
   */
  double getBinStart(int index) const;

  /*!
   * \brief Get end value of a bin.
   * \param index It can be negative.
   */
  double getBinEnd(int index) const;

  /*!
   * \brief Compute the Jensen-Shannon divergence between two normalized histograms.
   */
  static double computeJsDivergence(const std::vector<double> &hist1,
                                    const std::vector<double> &hist2);

private:
  double m_start;
  double m_interval;

  std::vector<double> m_count;
};

#endif // ZHISTOGRAM_H
