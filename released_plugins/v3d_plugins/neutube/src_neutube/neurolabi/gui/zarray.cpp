#include "zarray.h"

#include <iostream>

ZArray::ZArray() : m_data(NULL)
{
}

ZArray::ZArray(ZArray::Value_Type type, int ndims, mylib::Dimn_Type *dims)
{
  m_data = Make_Array(mylib::PLAIN_KIND, type, ndims, dims);
}

ZArray::ZArray(const ZArray &array)
{
  if (array.m_data == NULL) {
    m_data = NULL;
  } else {
    m_data = mylib::Copy_Array(array.m_data);
  }
}

ZArray::~ZArray()
{
  if (m_data != NULL) {
    mylib::Kill_Array(m_data);
  }
}

void ZArray::printInfo()
{
  if (m_data == NULL) {
    std::cout << "Empty array: null data" << std::endl;
  } else {
    if (ndims() == 0) {
      std::cout << "Empty array: 0-dimentional" << std::endl;
    } else {
      std::cout << "Array(#): " << mylib::Array_Refcount(m_data) << std::endl;
      std::cout << "  size: " << dim(0);
      for (int i = 1; i < ndims(); i++) {
        std::cout << " x " << dim(i);
      }
      std::cout << std::endl;

      switch (valueType()) {
      case mylib::UINT8_TYPE:
        std::cout << "  uint8" << std::endl;
        break;
      case mylib::UINT16_TYPE:
        std::cout << "  uint16" << std::endl;
        break;
      case mylib::UINT32_TYPE:
        std::cout << "  uint32" << std::endl;
        break;
      case mylib::UINT64_TYPE:
        std::cout << "  uint64" << std::endl;
        break;
      case mylib::INT8_TYPE:
        std::cout << "  int8" << std::endl;
        break;
      case mylib::INT16_TYPE:
        std::cout << "  int16" << std::endl;
        break;
      case mylib::INT32_TYPE:
        std::cout << "  int32" << std::endl;
        break;
      case mylib::INT64_TYPE:
        std::cout << "  int64" << std::endl;
        break;
      case mylib::FLOAT32_TYPE:
        std::cout << "  single float" << std::endl;
      case mylib::FLOAT64_TYPE:
        std::cout << "  double float" << std::endl;
      default:
          std::cout << "  unknown type" << std::endl;
      }
    }
  }
}
