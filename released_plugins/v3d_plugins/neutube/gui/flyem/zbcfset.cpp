#include "zbcfset.h"

#include <iostream>
#include "zhdf5reader.h"

using namespace std;

FlyEm::ZBcfSet::ZBcfSet()
{

}

FlyEm::ZBcfSet::ZBcfSet(const std::string &featurePath)
{
  setPath(featurePath);
}

ZMatrix* FlyEm::ZBcfSet::load(const std::string &name)
{
  ZHdf5Reader reader;

  ZMatrix *matrix = NULL;

  if (reader.open(m_featurePath + "/" + name + ".h5")) {
    mylib::Array *array = reader.readArray(name);

#ifdef _DEBUG_2
    mylib::printArrayInfo(array);
    cout << ((double*)array->data)[0] << endl;
#endif
    matrix = new ZMatrix(array->dims[0], array->dims[1]);
    matrix->copyValue((double*) array->data);

    mylib::Kill_Array(array);

    reader.close();
  }

  return matrix;
}

ZMatrix* FlyEm::ZBcfSet::load(const vector<string> &name)
{
  ZMatrix *matrix = NULL;
  for (vector<string>::const_iterator iter = name.begin(); iter != name.end();
       ++iter) {
    ZHdf5Reader reader;
    if (reader.open(m_featurePath + "/" + *iter + ".h5")) {
      mylib::Array *array = reader.readArray(*iter);

      if (matrix == NULL) {
        matrix = new ZMatrix(array->dims[0], array->dims[1]);
        matrix->copyValue((double*) array->data);
      } else {
        int oldColumnNumber = matrix->getColumnNumber();
        matrix->resize(matrix->getRowNumber(),
                       matrix->getColumnNumber() + array->dims[1]);
        matrix->copyColumnValue((double*) array->data, oldColumnNumber,
                                array->dims[1]);
      }

      mylib::Kill_Array(array);

      reader.close();
    } else {
      cout << "Failed to open " << *iter << endl;
    }
  }

  return matrix;
}
