#ifndef ZHDF5READER_H
#define ZHDF5READER_H

#include <string>

#include "zhdf5_header.h"
#include "mylib/array.h"

/**
 * @brief The class for reading hdf5 files
 *
 * Usage:
 *  ZHdf5Reader reader;
 *  if (reader.open("test.h5")) {
 *    mylib::Array *array = reader.readArray("/test/test_array");
 *    if (array != NULL) {
 *      mylib::printArrayInfo(array);
 *    }
 *  } else {
 *    cout << "Cannot read the file." << endl;
 *  }
 */
class ZHdf5Reader
{
public:
  ZHdf5Reader();
  ZHdf5Reader(const std::string &source);
  ~ZHdf5Reader();

public:
  bool open(const std::string &source);
  void close();
  mylib::Array* readArray(const std::string &dataPath);

  static herr_t printObjectInfo(hid_t loc_id, const char *name, void *opdata);
  void printInfo();

private:
  std::string m_source;
  hid_t m_file;
};

#endif // ZHDF5READER_H
