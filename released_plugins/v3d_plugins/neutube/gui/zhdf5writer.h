#ifndef ZHDF5WRITER_H
#define ZHDF5WRITER_H

#include <string>
#include <vector>

#include "zhdf5_header.h"
#include "mylib/array.h"

/**
 * @brief The class for writing hdf5 files
 *
 * Usage:
 *  ZHdfWriter writer;
 *  if (writer.open("test.h5")) {
 *    ZHdfReader reader;
 *    reader.open("read.h5");
 *    mylib::Array *array = reader.readArray("/test/test_array");
 *    writer.createGroup("/test");
 *    writer.writeArray("/test/test_array", array);
 *  } else {
 *    cout << "Cannot open the file." << endl;
 *  }
 */
class ZHdf5Writer
{
public:
  ZHdf5Writer();
  ZHdf5Writer(const std::string &filePath);
  ~ZHdf5Writer();

public:
  bool open(const std::string &filePath);
  bool open(const std::string &filePath, unsigned flags);
  void close();
  void createGroup(const std::string &group);
  void writeArray(const std::string &path, const mylib::Array *array);
  void writeDoubleArray(const std::string &path,
                        const std::vector<std::vector<double> > &feature);

private:
  hid_t m_file;
};

#endif // ZHDF5WRITER_H
