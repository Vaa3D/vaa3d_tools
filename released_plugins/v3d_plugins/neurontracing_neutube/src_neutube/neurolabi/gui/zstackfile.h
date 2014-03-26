#ifndef ZSTACKFILE_H
#define ZSTACKFILE_H

#include <vector>
#include <string>
#include "tz_stack_document.h"
#include "tz_image_io.h"
#include "zjsonparser.h"
#include "tz_stack_document.h"

class ZStack;
class ZFileList;

class ZStackFile
{
public:
  ZStackFile();
  ZStackFile(const ZStackFile &file);

  enum EFileType {
    UNIDENTIFIED, SINGLE_FILE, FILE_BUNDLE, IMAGE_SERIES, FILE_LIST,
    SOBJ_LIST, HDF5_DATASET
  };

public:
  ZStack *readStack(ZStack *data = NULL);
  File_Bundle_S toFileBundleS() const;
  ZFileList *toFileList() const;
  void import(const std::string &filePath);
  void importImageSeries(const std::string &filePath);
  void importSobjList(const std::vector<std::string> &fileList);

  void loadJsonObject(json_t *obj, const std::string &source);
  void loadStackDocument(const Stack_Document *doc);

  void importJsonFile(const std::string &filePath);
  void importXmlFile(const std::string &filePath);

  void retrieveAttribute(int *kind, int *width, int *height, int *depth) const;

  std::string firstUrl() const;
  inline std::string prefix() const { return m_prefix; }
  inline std::string suffix() const { return m_suffix; }
  inline int numWidth() const { return m_numWidth; }
  inline int firstNum() const { return m_firstNum; }
  inline int channel() const { return m_channel; }

  inline void setChannel(int channel) { m_channel = channel; }

  int countImageSeries() const;

  void print();

  static bool isStackTag(const std::string &tag);
  static bool isUrlTag(const std::string &tag);
  static bool isPrefixTag(const std::string &tag);
  static bool isSuffixTag(const std::string &tag);
  static bool isNumWidthTag(const std::string &tag);
  static bool isFirstNumTag(const std::string &tag);
  static bool isTypeTag(const std::string &tag);
  static bool isLastNumTag(const std::string &tag);
  static bool isChannelTag(const std::string &tag);
  static bool isDimFlipTag(const std::string &tag);

  std::string fileBundlePath(int n) const;

  void setType(const std::string &str);
  EFileType type() { return m_type; }

  void appendUrl(const std::string &path);

private:
  EFileType m_type;
  std::vector<std::string> m_urlList;
  std::string m_prefix;
  std::string m_suffix;
  int m_numWidth;
  int m_firstNum;
  int m_lastNum;
  int m_channel;
  bool m_dimFlip; //for hdf5 only
};

#endif // ZSTACKFILE_H
