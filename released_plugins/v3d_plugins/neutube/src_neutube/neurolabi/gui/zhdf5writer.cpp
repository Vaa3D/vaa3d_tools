#include "zhdf5writer.h"

#include <string.h>

#include "tz_utilities.h"
#include "mylib/array.h"

ZHdf5Writer::ZHdf5Writer() : m_file(NULL_FILE)
{
}

ZHdf5Writer::ZHdf5Writer(const std::string &source)
{
  open(source);
}

ZHdf5Writer::~ZHdf5Writer()
{
  close();
}

bool ZHdf5Writer::open(const std::string &filePath)
{
  close();

#if defined(_ENABLE_HDF5_)
  if (fexist(filePath.c_str())) {
    m_file = H5Fopen(filePath.c_str(), H5F_ACC_RDWR, H5P_DEFAULT);
  } else {
    m_file = H5Fcreate(filePath.c_str(), H5F_ACC_EXCL, H5P_DEFAULT,
                       H5P_DEFAULT);
  }
#endif

  return (m_file != NULL_FILE);
}

bool ZHdf5Writer::open(const std::string &filePath, unsigned flags)
{
  close();

#if defined(_ENABLE_HDF5_)
  m_file = H5Fopen(filePath.c_str(), flags, H5P_DEFAULT);
#endif

  return (m_file != NULL_FILE);
}



void ZHdf5Writer::close()
{
#if defined(_ENABLE_HDF5_)
  if (m_file != NULL_FILE) {
    H5Fclose(m_file);
    m_file = NULL_FILE;
  }
#endif
}

void ZHdf5Writer::createGroup(const std::string &group)
{
#if defined(_ENABLE_HDF5_)
  if (m_file != NULL_FILE) {
    herr_t status = H5Gget_objinfo(m_file, group.c_str(), 0, NULL);
    if (status != 0) {
      hid_t group_id = H5Gcreate(m_file, group.c_str(), H5P_DEFAULT, H5P_DEFAULT,
                                 H5P_DEFAULT);
      H5Gclose(group_id);
    }
  }
#else
  UNUSED_PARAMETER(&group);
#endif
}

void ZHdf5Writer::writeArray(const std::string &path, const mylib::Array *array)
{
#if defined(_ENABLE_HDF5_)
  hsize_t dims[array->ndims];
  for (int i = 0; i < array->ndims; ++i) {
    dims[i] = array->dims[i];
  }

  hid_t type_id;
  hid_t mem_type_id;

  switch (array->type) {
  case mylib::INT8_TYPE:
    type_id = H5T_STD_I8BE;
    mem_type_id = H5T_NATIVE_INT8;
    break;
  case mylib::UINT8_TYPE:
    type_id = H5T_STD_U8BE;
    mem_type_id = H5T_NATIVE_UINT8;
    break;
  case mylib::INT16_TYPE:
    type_id = H5T_STD_I16BE;
    mem_type_id = H5T_NATIVE_INT16;
    break;
  case mylib::UINT16_TYPE:
    type_id = H5T_STD_U16BE;
    mem_type_id = H5T_NATIVE_UINT16;
    break;
  case mylib::INT32_TYPE:
    type_id = H5T_STD_I32BE;
    mem_type_id = H5T_NATIVE_INT32;
    break;
  case mylib::UINT32_TYPE:
    type_id = H5T_STD_U16BE;
    mem_type_id = H5T_NATIVE_UINT16;
    break;
  case mylib::INT64_TYPE:
    type_id = H5T_STD_I64BE;
    mem_type_id = H5T_NATIVE_INT64;
    break;
  case mylib::UINT64_TYPE:
    type_id = H5T_STD_U64BE;
    mem_type_id = H5T_NATIVE_UINT64;
    break;
  case mylib::FLOAT32_TYPE:
    type_id = H5T_IEEE_F32BE;
    mem_type_id = H5T_NATIVE_FLOAT;
    break;
  case mylib::FLOAT64_TYPE:
    type_id = H5T_IEEE_F64BE;
    mem_type_id = H5T_NATIVE_DOUBLE;
    break;
  default:
    type_id = H5T_STD_I8BE;
    mem_type_id = H5T_NATIVE_INT8;
    break;
  }

  hid_t dataSpace = H5Screate_simple(array->ndims, dims, NULL);
  hid_t dataset_id = H5Dcreate1(m_file, path.c_str(), type_id, dataSpace,
                                H5P_DEFAULT);

  H5Dwrite(dataset_id, mem_type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT,
           array->data);

  /* Close the dataset. */
  H5Sclose(dataSpace);
  H5Dclose(dataset_id);
#else
  UNUSED_PARAMETER(&path);
  UNUSED_PARAMETER(array);
#endif
}

void ZHdf5Writer::writeDoubleArray(
    const std::string &path, const std::vector<std::vector<double> > &feature)
{
  int ndim = 2;
  mylib::Dimn_Type dims[2];
  dims[0] = feature.size();
  dims[1] = feature[0].size();

  mylib::Array *array = mylib::Make_Array(
        mylib::PLAIN_KIND, mylib::FLOAT64_TYPE, ndim, dims);

  for (size_t i = 0; i < feature.size(); ++i) {
    memcpy((double *) array->data + dims[1] * i,
        &(feature[i][0]), dims[1] * sizeof(double));
  }

  writeArray(path, array);

  mylib::Kill_Array(array);
}
