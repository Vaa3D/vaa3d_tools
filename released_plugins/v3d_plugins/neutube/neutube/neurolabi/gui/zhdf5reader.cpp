#include "zhdf5reader.h"

#include <string>
#include "tz_utilities.h"

using namespace std;

ZHdf5Reader::ZHdf5Reader() : m_file(NULL_FILE)
{
}

ZHdf5Reader::ZHdf5Reader(const std::string &source) : m_file(NULL_FILE)
{
  open(source);
}

ZHdf5Reader::~ZHdf5Reader()
{
  close();
}

bool ZHdf5Reader::open(const std::string &source)
{
  close();

  m_source = source;

#if defined(_ENABLE_HDF5_)
  m_file = H5Fopen(source.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
#endif

  return (m_file != NULL_FILE);
}

void ZHdf5Reader::close()
{
#if defined(_ENABLE_HDF5_)
  if (m_file != NULL_FILE) {
    H5Fclose(m_file);
    m_file = NULL_FILE;
  }
#endif
}

mylib::Array* ZHdf5Reader::readArray(const std::string &dataPath)
{
  mylib::Array *array = NULL;

#if defined(_ENABLE_HDF5_)
  hid_t dset = H5Dopen(m_file, dataPath.c_str(), H5P_DEFAULT);
  hid_t space = H5Dget_space(dset);
  hid_t datatype = H5Dget_type(dset);
  //H5T_class_t dataClass = H5Tget_class(datatype);
  mylib::Value_Type arrayType;

  hid_t nativeType;

  if (H5Tequal(datatype, H5T_STD_I8BE) || H5Tequal(datatype, H5T_STD_I8LE)) {
    arrayType = mylib::UINT8_TYPE;
    nativeType = H5T_NATIVE_CHAR;
  } else if (H5Tequal(datatype, H5T_STD_I8BE) || H5Tequal(datatype, H5T_STD_I8LE)) {
    arrayType = mylib::INT8_TYPE;
    nativeType = H5T_NATIVE_UCHAR;
  } else if (H5Tequal(datatype, H5T_STD_U16BE) || H5Tequal(datatype, H5T_STD_U16LE)) {
    arrayType = mylib::UINT16_TYPE;
    nativeType = H5T_NATIVE_UINT16;
  } else if (H5Tequal(datatype, H5T_STD_I16BE) || H5Tequal(datatype, H5T_STD_I16LE)) {
    arrayType = mylib::INT16_TYPE;
    nativeType = H5T_NATIVE_INT16;
  } else if (H5Tequal(datatype, H5T_STD_U32BE) || H5Tequal(datatype, H5T_STD_U32LE)) {
    arrayType = mylib::UINT32_TYPE;
    nativeType = H5T_NATIVE_UINT32;
  } else if (H5Tequal(datatype, H5T_STD_I32BE) || H5Tequal(datatype, H5T_STD_I32LE)) {
    arrayType = mylib::INT32_TYPE;
    nativeType = H5T_NATIVE_INT32;
  } else if (H5Tequal(datatype, H5T_STD_I64BE) || H5Tequal(datatype, H5T_STD_I64LE)) {
    arrayType = mylib::INT64_TYPE;
    nativeType = H5T_NATIVE_INT64;
  } else if (H5Tequal(datatype, H5T_STD_U64BE) || H5Tequal(datatype, H5T_STD_U64LE)) {
    arrayType = mylib::UINT64_TYPE;
    nativeType = H5T_NATIVE_UINT64;
  } else if (H5Tequal(datatype, H5T_IEEE_F32BE) || H5Tequal(datatype, H5T_IEEE_F32LE)) {
    arrayType = mylib::FLOAT32_TYPE;
    nativeType = H5T_NATIVE_FLOAT;
  } else if (H5Tequal(datatype, H5T_IEEE_F64BE) || H5Tequal(datatype, H5T_IEEE_F64LE)) {
    arrayType = mylib::FLOAT64_TYPE;
    nativeType = H5T_NATIVE_DOUBLE;
  } else {
    arrayType = mylib::UNKNOWN_TYPE;
    nativeType = H5T_NATIVE_CHAR;
  }

  if (arrayType != mylib::UNKNOWN_TYPE) {
    //int ndim = H5Sget_simple_extent_ndims(datatype);

    //H5Tget_array_dims(datatype, dims);
    int ndim = H5Sget_simple_extent_ndims(space);
    hsize_t dims[ndim];
    H5Sget_simple_extent_dims(space, dims, NULL);


    //hid_t type = H5Tget_native_type(datatype, H5T_DIR_ASCEND);

    mylib::Dimn_Type arrayDims[ndim];
    for (int i = 0; i < ndim; ++i) {
      arrayDims[i] = dims[i];
    }

    mylib::Array_Kind kind = mylib::PLAIN_KIND;


    if (arrayType != mylib::UNKNOWN_TYPE) {
      array = mylib::Make_Array(kind, arrayType, ndim, arrayDims);
      H5Dread(dset, nativeType, H5S_ALL, H5S_ALL, H5P_DEFAULT, array->data);
    }
  }

  H5Tclose(datatype);
  //H5Tclose(nativeType);
  H5Dclose(dset);
  H5Sclose(space);
#endif

  return array;
}

typedef struct _Hdf5PrintOpData {
  int indent;
  char *path;
  hid_t file_id;
} Hdf5PrintOpData;

herr_t ZHdf5Reader::printObjectInfo(hid_t loc_id, const char *name, void *opdata)
{
#ifdef _USE_HDF5_
  H5G_stat_t statbuf;

  H5Gget_objinfo(loc_id, name, 0, &statbuf);
  int indent = 0;
  if (opdata != NULL) {
    indent = ((Hdf5PrintOpData*) opdata)->indent;
  }

  switch (statbuf.type) {
  case H5G_GROUP:
       printf("%*sGroup: %s\n", indent, "", name);
       if (opdata != NULL) {
         ((Hdf5PrintOpData*) opdata)->indent += 2;
         string path = ((Hdf5PrintOpData*) opdata)->path;
         sprintf(((Hdf5PrintOpData*) opdata)->path, "%s%s/",
                 ((Hdf5PrintOpData*) opdata)->path, name);
#ifdef _DEBUG_2
         printf("path: %s\n", ((Hdf5PrintOpData*) opdata)->path);
#endif
         H5Giterate(((Hdf5PrintOpData*) opdata)->file_id,
             ((Hdf5PrintOpData*) opdata)->path, NULL,
             printObjectInfo, opdata);

         sprintf(((Hdf5PrintOpData*) opdata)->path, "%s", path.c_str());
         ((Hdf5PrintOpData*) opdata)->indent -= 2;
       }
       break;
  case H5G_DATASET:
       printf("%*sDataset: %s\n", indent, "", name);
       break;
  case H5G_TYPE:
       printf("%*sNamed datatype: %s\n", indent, "", name);
       break;
  default:
       printf("%*sUnidenfied object: %s\n", indent, "", name);
  }
#else
  UNUSED_PARAMETER(loc_id);
  UNUSED_PARAMETER(opdata);
  UNUSED_PARAMETER(name);
#endif
  return 0;
}

void ZHdf5Reader::printInfo()
{
#ifdef _USE_HDF5_
  if (m_file != NULL_FILE) {
    Hdf5PrintOpData opdata;
    opdata.indent = 0;
    char path[500];
    path[0] = '/';
    path[1] = '\0';
    opdata.path = path;
    opdata.file_id = m_file;
    H5Giterate(m_file, "/", NULL, printObjectInfo, &opdata);
  }
#endif
}
