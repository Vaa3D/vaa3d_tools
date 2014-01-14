#ifndef ZSTACKDOCUMENT_H
#define ZSTACKDOCUMENT_H

#include <vector>
#include <string>
#include "tz_stack_document.h"
#include "tz_image_io.h"

class ZStack;

class ZStackDocument
{
public:
  ZStackDocument();
  ~ZStackDocument();

  enum EFileType {
    UNIDENTIFIED, TIF_FILE, LSM_FILE, FILE_BUNDLE,
    RAW_FILE, SWC_FILE, SFILE_LIST, FILE_SERIES
  };

  ZStack *readData();
  File_Bundle_S toFileBundleS();

private:
  EFileType m_type;
  std::vector<std::string> m_urlList;
  std::string m_prefix;
  std::string m_suffix;
  int m_numWidth;
  int m_firstNum;
  int m_lastNum;
  //Stack_Document *m_doc;
};

#endif // ZSTACKDOCUMENT_H
