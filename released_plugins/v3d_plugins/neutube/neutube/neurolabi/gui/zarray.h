#ifndef ZARRAY_H
#define ZARRAY_H

#include "mylib/array.h"

class ZArray
{
public:
  typedef mylib::Value_Type Value_Type;
  typedef mylib::Dimn_Type Dimn_Type;

public:
  ZArray();
  ZArray(Value_Type type, int ndims, mylib::Dimn_Type *dims);
  ZArray(const ZArray &array); //always deep copy
  ~ZArray();

  inline int ndims() { return m_data->ndims; }
  inline int dim(int index) { return m_data->dims[index]; }
  inline Value_Type valueType() { return m_data->type; }

  void printInfo();

private:
  mylib::Array *m_data;
};

#endif // ZARRAY_H
