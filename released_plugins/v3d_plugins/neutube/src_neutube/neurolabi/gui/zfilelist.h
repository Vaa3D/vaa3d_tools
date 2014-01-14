#ifndef ZFILELIST_H
#define ZFILELIST_H

#include <string>
#include "tz_file_list.h"

class ZFileList
{
public:
  ZFileList();
  ZFileList(int n);
  ~ZFileList();

public:
  enum ESortingOption {
    NO_SORT, SORT_BY_LAST_NUMBER, SORT_ALPHABETICALLY
  };

  void load(const std::string &dir, const std::string &ext,
            ESortingOption option = NO_SORT);
  void importFromXml(const std::string &filePath);

  char* getFilePath(int index) const;
  inline int size() const { return m_fileList.file_number; }

  int startNumber() const;
  int endNumber() const;

  void setFilePath(int index, std::string path);

private:
  File_List m_fileList;
};

#endif // ZFILELIST_H
