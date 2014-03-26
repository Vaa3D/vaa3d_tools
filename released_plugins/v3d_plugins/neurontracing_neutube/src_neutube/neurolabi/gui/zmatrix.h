#ifndef ZMATRIX_H
#define ZMATRIX_H

#include <vector>
#include <string>

/**
 * @brief The ZMatrix class
 *
 * Note that each row is stored in one array.
 */
class ZMatrix
{
public:
  ZMatrix();
  ZMatrix(int rowNumber, int columnNumber);
  ~ZMatrix();

  /*!
   * \brief Clear matrix buffer.
   *
   * Row number and column number are set to 0.
   */
  void clear();

  void setConstant(double value);
  inline void set(int i, int j, double value) { m_data[i][j] = value; }

  /*!
   * \brief Set the value of the element at \a index
   *
   * nothing is done if index is out of range.
   */
  void set(int index, double value);

  inline double getValue(int i, int j) const { return m_data[i][j]; }

  inline void addValue(int i, int j, double dv) { m_data[i][j] += dv; }

  /*!
   * \brief Get the value by index.
   *
   * \return 0.0 if \a index is out of range.
   */
  double getValue(int index) const;

  inline double& at(int i, int j) { return m_data[i][j]; }
  inline int getSize() const { return m_rowNumber * m_columnNumber; }

  void resize(int rowNumber, int columnNumber);

  inline int getRowNumber() const { return m_rowNumber; }
  inline int getColumnNumber() const { return m_columnNumber; }
  int sub2index(int row, int col) const;
  std::pair<int, int> index2sub(int index) const;

  void debugOutput();

  void copyValue(double *data);
  void copyColumnValue(double *data, int columnStart, int columnNumber);

  /*!
   * \brief Copy a row to an array
   *
   * Copy the the \a columnStart - \a columnEnd elements at row \a row to
   * \a data. It only copies value within the valie range. Any element out of
   * range is treated as void, which does nothing on \a data but shifting its
   * pointer position. The user has to make sure \a data is sufficiently
   * allocated.
   *
   * \a return Actual number of values copied.
   */
  int copyRowValue(int row, int columnStart, int columnEnd, double *data);

  /*!
   * \brief Load double array from a file
   *
   * Loads matrix from a text file. All values in the orignal buffer are cleared
   * before the reading.
   */
  void importTextFile(const std::string &filePath);

  /*!
   * \brief Export the matrix into a CSV file
   * \param Output file path
   * \return true iff the matrix is exported successfully.
   */
  bool exportCsv(const std::string &path);

public: //Feature matrix functions
  std::vector<int> kmeans(int k);
  std::vector<int> weightedKmeans(int k);

  /*!
   * \brief Get the maximum value of a row
   * \param The specified value.
   * \return The maximum of value of row \row. The column index is stored in
   *         \a index if \a index is not NULL.
   */
  double getRowMax(int row, int *index = NULL) const;

private:
  double **m_data;
  int m_rowNumber;
  int m_columnNumber;
};


#endif // ZMATRIX_H
