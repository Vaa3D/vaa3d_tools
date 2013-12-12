#include "zmatrix.h"
#if defined(_QT_GUI_USED_)
#include <QDebug>
#endif
#include <ostream>
#include <sstream>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <string.h>

#include "tz_error.h"
#include "tz_darray.h"

using namespace std;

ZMatrix::ZMatrix()
{
  m_rowNumber = 0;
  m_columnNumber = 0;
  m_data = NULL;
}

ZMatrix::ZMatrix(int rowNumber, int columnNumber)
{
  m_rowNumber = rowNumber;
  m_columnNumber = columnNumber;
  m_data = (double**) calloc(rowNumber, sizeof(double*));
  for (int i = 0; i < rowNumber; i++) {
    m_data[i] = (double*) calloc(columnNumber, sizeof(double));
  }
}

ZMatrix::~ZMatrix()
{
  clear();
}

void ZMatrix::resize(int rowNumber, int columnNumber)
{
  TZ_ASSERT(rowNumber >= 0, "Invalid row number");
  TZ_ASSERT(columnNumber >= 0, "Invalid column number");

  //Free unwanted rows
  if (rowNumber < getRowNumber()) {
    for (int i = rowNumber; i < getRowNumber(); ++i) {
      free(m_data[i]);
      m_data[i] = NULL;
    }
    m_rowNumber = rowNumber;
  }

  //Realloc rows
  if (rowNumber != getRowNumber()) {
    m_data = (double**) realloc(m_data, rowNumber * sizeof(double*));
    if (rowNumber > getRowNumber()) {
      for (int i = getRowNumber(); i < rowNumber; ++i) {
        m_data[i] = (double*) calloc(columnNumber, sizeof(double));
      }
    }
  }

  //Reset columns
  if (columnNumber != getColumnNumber()) {
    for (int i = 0; i < getRowNumber(); ++i) {
      m_data[i] = (double*) realloc(m_data[i], columnNumber * sizeof(double));
    }
  }

  m_rowNumber = rowNumber;
  m_columnNumber = columnNumber;
}

void ZMatrix::setConstant(double value)
{
  for (int i = 0; i < m_rowNumber; i++) {
    for (int j = 0; j < m_columnNumber; j++) {
      set(i, j, value);
    }
  }
}

int ZMatrix::sub2index(int row, int col) const
{
  int index = -1;
  if ((row >= 0) && (row < m_rowNumber) &&
      (col >= 0) && (col < m_columnNumber)) {
    index = row * m_columnNumber + col;
  }

  return index;
}

pair<int, int> ZMatrix::index2sub(int index) const
{
  pair<int, int> sub;
  sub.first = index / m_columnNumber;
  sub.second = index % m_columnNumber;

  return sub;
}

void ZMatrix::debugOutput()
{
  for (int i = 0; i < m_rowNumber; i++) {
    for (int j = 0; j < m_columnNumber; j++) {
      cout << at(i, j) << " ";
    }
    cout << endl;
  }
  /*
  ostringstream stream;

  stream << m_rowNumber << " x " << m_columnNumber << " matrix\n";
  for (int i = 0; i < m_rowNumber; i++) {
    if (i > 10) {
      stream << "...\n";
      break;
    }
    for (int j = 0; j < m_columnNumber; j++) {
      if (j > 10) {
        stream << " ... ";
        break;
      }
      stream << m_data[2][4] << " ";
    }
    stream << '\n';
  }

  cout << stream.str().c_str();
  */
}

void ZMatrix::copyValue(double *data)
{
  for (int i = 0; i < getRowNumber(); ++i) {
    memcpy(m_data[i], data + getColumnNumber() * i,
           getColumnNumber() * sizeof(double));
  }

#ifdef _DEBUG_2
  cout << "copied:" << endl;
  for (int i = 0; i < getRowNumber(); i++) {
    for (int j = 0; j < getColumnNumber(); j++) {
      cout << getValue(i, j) << " ";
    }
    cout << endl;
  }
  cout << endl;
#endif
}

void ZMatrix::copyColumnValue(double *data, int columnStart, int columnNumber)
{
  if (columnNumber + columnStart > getColumnNumber()) {
    resize(getRowNumber(), columnNumber + columnStart);
  }

  for (int i = 0; i < getRowNumber(); ++i) {
    memcpy(m_data[i] + columnStart, data + columnNumber * i,
           columnNumber * sizeof(double));
  }
}

bool ZMatrix::exportCsv(const string &path)
{
  std::ofstream stream(path.c_str());

  if (!stream.is_open()) {
    return false;
  }

  for (int i = 0; i < m_rowNumber; i++) {
    for (int j = 0; j < m_columnNumber; j++) {
      stream << at(i, j);
      if (j != m_columnNumber - 1) {
        stream << ",";
      }
    }
    stream << endl;
  }

  stream.close();

  return true;
}

double ZMatrix::getRowMax(int row, int *index) const
{
  size_t arrayIndex;

  double v = darray_max(m_data[row], getColumnNumber(), &arrayIndex);

  if (index != NULL) {
    *index = (int) arrayIndex;
  }

  return v;
}

void ZMatrix::clear()
{
  for (int i = 0; i < m_rowNumber; i++) {
    free(m_data[i]);
    m_data[i] = NULL;
  }

  free(m_data);
  m_data = NULL;

  m_rowNumber = 0;
  m_columnNumber = 0;
}

void ZMatrix::importTextFile(const string &filePath)
{
  clear();

  double *value = darray_load_matrix(filePath.c_str(), NULL, &m_columnNumber,
                                     &m_rowNumber);
  m_data = (double**) calloc(m_rowNumber, sizeof(double*));
  for (int i = 0; i < m_rowNumber; i++) {
    m_data[i] = (double*) calloc(m_columnNumber, sizeof(double));
  }

  copyValue(value);

  free(value);
}
