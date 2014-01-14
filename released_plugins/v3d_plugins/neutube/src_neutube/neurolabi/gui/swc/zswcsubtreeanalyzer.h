#ifndef ZSWCSUBTREEANALYZER_H
#define ZSWCSUBTREEANALYZER_H

class ZSwcTree;

class ZSwcSubtreeAnalyzer
{
public:
  ZSwcSubtreeAnalyzer();

  /*!
   * \brief Label subtrees with contiguous integers starting from 1
   */
  int labelSubtree(ZSwcTree *tree) const;

  /*!
   * \brief Decompose the swc tree into subtrees
   *
   * all the subtrees will be under the virtual root.
   *
   * \return The number of subtrees
   */
  int decompose(ZSwcTree *tree) const;

  inline void setMinLength(double length) { m_minLength = length; }
  inline void setMaxLength(double length) { m_maxLength = length; }

  inline double getMinLength() const { return m_minLength; }

private:
  double m_minLength;
  double m_maxLength;
};

#endif // ZSWCSUBTREEANALYZER_H
