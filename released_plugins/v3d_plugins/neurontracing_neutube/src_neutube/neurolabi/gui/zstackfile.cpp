#include "zstackfile.h"

#include <iostream>
#include <string.h>

#include "zstack.hxx"
#include "tz_image_io.h"
#include "c_stack.h"
#include "zfiletype.h"
#include "zfilelist.h"
#include "zstring.h"
#include "zxmldoc.h"
#include "tz_error.h"
#include "zhdf5reader.h"
#include "zobject3dscan.h"
#include "zobject3d.h"

using namespace std;

ZStackFile::ZStackFile() : m_numWidth(0), m_firstNum(0), m_lastNum(0),
  m_channel(-1), m_dimFlip(false)
{
}

ZStackFile::ZStackFile(const ZStackFile &file)
{
  m_type = file.m_type;
  m_urlList = file.m_urlList;
  m_prefix = file.m_prefix;
  m_suffix = file.m_suffix;
  m_numWidth = file.m_numWidth;
  m_firstNum = file.m_firstNum;
  m_lastNum = file.m_lastNum;
  m_channel = file.m_channel;
  m_dimFlip = file.m_dimFlip;
}

void ZStackFile::loadStackDocument(const Stack_Document *doc)
{
  TZ_ASSERT(doc != NULL, "Null pointer");

#ifdef _DEBUG_
  cout << "Loading stack document ..." << endl;
#endif

  m_urlList.clear();

  switch (doc->type) {
  case STACK_DOC_TIF_FILE:
  case STACK_DOC_LSM_FILE:
  case STACK_DOC_RAW_FILE:
  case STACK_DOC_SWC_FILE:
    m_type = SINGLE_FILE;
    m_urlList.push_back(Stack_Document_File_Path(doc));
    break;
  case STACK_DOC_FILE_BUNDLE:
  {
    m_type = FILE_BUNDLE;
    File_Bundle_S* fileBundle = (File_Bundle_S*) doc->ci;
    m_prefix = fileBundle->prefix;
    m_suffix = fileBundle->suffix;
    m_numWidth = fileBundle->num_width;
    m_lastNum = fileBundle->last_num;
  }
    break;
  case STACK_DOC_FILE_LIST:
  {
    m_type = FILE_LIST;
    File_List *list = (File_List*) doc->ci;
    for (int i = 0; i < list->file_number; ++i) {
      m_urlList.push_back(list->file_path[i]);
    }
  }
    break;
  }

  m_channel = doc->channel;
}

string ZStackFile::firstUrl() const
{
  string url;

  if (!m_urlList.empty()) {
  switch (m_type) {
  case SINGLE_FILE:
  case FILE_LIST:
    url = m_urlList[0];
    break;
  case IMAGE_SERIES:
  {
    ZFileList *fileList = toFileList();
    url = fileList->getFilePath(0);
    delete fileList;
    break;
  }
  case FILE_BUNDLE:
  {
    url= fileBundlePath(m_firstNum);
  }
  default:
    break;
  }
  }

  return url;
}

void ZStackFile::retrieveAttribute(
    int *kind, int *width, int *height, int *depth) const
{
  TZ_ASSERT(kind != NULL && width != NULL && height != NULL && depth != NULL,
            "Null pointer");

  std::string filePath = firstUrl();

  switch (ZFileType::fileType(filePath)) {
  case ZFileType::TIFF_FILE:
    Tiff_Attribute(filePath.c_str(), 0, kind, width, height, depth);
    break;
  case ZFileType::LSM_FILE:
    Tiff_Attribute(filePath.c_str(), 1, kind, width, height, depth);
    break;
  case ZFileType::PNG_FILE:
    Png_Attribute(filePath.c_str(), kind, width, height);
    *depth = 1;
    break;
  default:
    break;
  }

  switch (m_type) {
  case FILE_BUNDLE:
    *depth = m_lastNum - m_firstNum + 1;
    break;
  case FILE_LIST:
    *depth = m_urlList.size();
    break;
  case IMAGE_SERIES:
    *depth = countImageSeries();
    break;
  default:
    break;
  }
}

void ZStackFile::import(const string &filePath)
{
#ifdef _DEBUG_2
  cout << "Importing " << filePath << endl;
#endif

  m_urlList.clear();

  switch (ZFileType::fileType(filePath)) {
  case ZFileType::TIFF_FILE:
  case ZFileType::LSM_FILE:
  case ZFileType::V3D_RAW_FILE:
  case ZFileType::PNG_FILE:
  case ZFileType::V3D_PBD_FILE:
  case ZFileType::MYERS_NSP_FILE:
  case ZFileType::OBJECT_SCAN_FILE:
  case ZFileType::DVID_OBJECT_FILE:
  case ZFileType::JPG_FILE:
#ifdef _DEBUG_2
    cout << filePath << endl;
    cout << filePath.find("*") << endl;
#endif
    if (filePath.find_last_of("*") != string::npos) {
      m_type = IMAGE_SERIES;
    } else {
      m_type = SINGLE_FILE;
    }
    m_urlList.push_back(filePath);
    break;
  case ZFileType::XML_FILE:
    importXmlFile(filePath);
    break;
  case ZFileType::JSON_FILE:
    importJsonFile(filePath);
    break;
  default:
    break;
  }
}

void ZStackFile::importImageSeries(const string &filePath)
{
  m_urlList.clear();
  m_type = IMAGE_SERIES;
  m_urlList.push_back(filePath);
}

void ZStackFile::importSobjList(const std::vector<string> &fileList)
{
  m_urlList = fileList;
  m_type = SOBJ_LIST;
}

bool ZStackFile::isStackTag(const string &tag)
{
  return tag == "zstack";
}

bool ZStackFile::isFirstNumTag(const string &tag)
{
  return tag == "first_num";
}

bool ZStackFile::isTypeTag(const string &tag)
{
  return tag == "format" || tag == "type";
}

bool ZStackFile::isLastNumTag(const string &tag)
{
  return tag == "last_num";
}

bool ZStackFile::isNumWidthTag(const string &tag)
{
  return tag == "num_width";
}

bool ZStackFile::isPrefixTag(const string &tag)
{
  return tag == "prefix";
}

bool ZStackFile::isSuffixTag(const string &tag)
{
  return tag == "suffix";
}

bool ZStackFile::isUrlTag(const string &tag)
{
  return tag == "url";
}

bool ZStackFile::isChannelTag(const string &tag)
{
  return tag == "channel";
}

bool ZStackFile::isDimFlipTag(const string &tag)
{
  return tag == "dimflip";
}

void ZStackFile::setType(const string &str)
{
#ifdef _DEBUG_2
  cout << str << endl;
#endif

  if (str == "image series" || str == "series") {
    m_type = IMAGE_SERIES;
  } else if (str == "file list" || str == "list") {
    m_type = FILE_LIST;
  } else if (str == "file bundle" || str == "bundle") {
    m_type = FILE_BUNDLE;
  } else if (str == "standalone" || str == "single file"
             || str == "single") {
    m_type = SINGLE_FILE;
  } else if (str == "hdf5"){
    m_type = HDF5_DATASET;
  } else {
    m_type = UNIDENTIFIED;
  }
}

void ZStackFile::loadJsonObject(json_t *obj, const std::string &source)
{
  const char *format = NULL;
  m_type = UNIDENTIFIED;

  const char *stackKey = NULL;
  json_t *stackValue = NULL;
  json_object_foreach(obj, stackKey, stackValue) {
#ifdef _DEBUG_2
    cout << stackKey << endl;
#endif
    if (isTypeTag(stackKey)) {
      format = ZJsonParser::stringValue(stackValue);
      setType(format);
    } else if (isUrlTag(stackKey)) {
      if (m_type == FILE_LIST) {
        size_t arraySize = ZJsonParser::arraySize(stackValue);
        for (size_t i = 0; i < arraySize; i++) {
          json_t *obj = ZJsonParser::arrayValue(stackValue, i);
          ZString url = ZJsonParser::stringValue(obj);
          m_urlList.push_back(
                url.absolutePath(ZString(source).dirPath()).c_str());
        }
      } else {
        ZString url = ZJsonParser::stringValue(stackValue);
        m_urlList.push_back(
              url.absolutePath(ZString(source).dirPath()).c_str());
      }
    } else {
      if (isPrefixTag(stackKey)) {
        m_prefix = ZJsonParser::stringValue(stackValue);
      } else if (isSuffixTag(stackKey)) {
        m_suffix = ZJsonParser::stringValue(stackValue);
      } else if (isNumWidthTag(stackKey)) {
        m_numWidth = ZJsonParser::integerValue(stackValue);
      } else if (isFirstNumTag(stackKey)) {
        m_firstNum = ZJsonParser::integerValue(stackValue);
      } else if (isLastNumTag(stackKey)) {
        m_lastNum = ZJsonParser::integerValue(stackValue);
      } else if (isChannelTag(stackKey)) {
        m_channel = ZJsonParser::integerValue(stackValue);
      } else if (isDimFlipTag(stackKey)) {
        m_dimFlip = ZJsonParser::booleanValue(stackValue);
      }
    }
  }
}

void ZStackFile::importJsonFile(const std::string &filePath)
{
  if (!fexist(filePath.c_str())) {
    cout << filePath << " does not exist." << endl;
    return;
  }

  m_urlList.clear();

  ZJsonObject jsonObject;
  jsonObject.load(filePath);

  map<string, json_t*> entryMap = jsonObject.toEntryMap();

  bool stackFound = false;
  for (map<string, json_t*>::const_iterator iter = entryMap.begin();
       iter != entryMap.end(); ++iter) {
    if (isStackTag(iter->first)) {
      loadJsonObject(iter->second, filePath);
      stackFound = true;
      break;
    }
  }

  if (!stackFound) {
    cout << "No stack is found in " << filePath << endl;
  }
}


void ZStackFile::importXmlFile(const string &filePath)
{
  if (!fexist(filePath.c_str())) {
    cout << filePath << " does not exist." << endl;
    return;
  }

  ZXmlDoc doc;
  doc.parseFile(filePath);

  ZXmlNode root = doc.getRootElement();

  ZXmlNode node = root;
  if (!isStackTag(node.name())) {
    node = root.firstChild();
  }

  while (!node.empty()) {
    if (isStackTag(node.name())) {
      ZXmlNode data = node.firstChild();
      while (!data.empty()) {
#ifdef _DEBUG_
        cout << data.name() << endl;
        cout << data.stringValue() << endl;
#endif
        if (isTypeTag(data.name())) {
          setType(data.stringValue());
        } else if (isUrlTag(data.name())) {
          ZString url = data.stringValue();
          m_urlList.push_back(
                url.absolutePath(ZString(filePath).dirPath()).c_str());
        } else if (isPrefixTag(data.name())) {
          m_prefix = data.stringValue();
        } else if (isSuffixTag(data.name())) {
          m_suffix = data.stringValue();
        } else if (isNumWidthTag(data.name())) {
          m_numWidth = data.intValue();
        } else if (isFirstNumTag(data.name())) {
          m_firstNum = data.intValue();
        } else if (isLastNumTag(data.name())) {
          m_lastNum = data.intValue();
        } else if (isChannelTag(data.name())) {
          m_channel = data.intValue();
        }

        data = data.nextSibling();
      }
    }
    node = node.nextSibling();
  }
}

int ZStackFile::countImageSeries() const
{
  int count = 0;

  if (m_type == IMAGE_SERIES) {
    ZFileList *fileList = toFileList();
    count = fileList->size();

    delete fileList;
  }

  return count;
}

std::string ZStackFile::fileBundlePath(int n) const
{
  if (m_type != FILE_BUNDLE || m_urlList.empty() ||
      n < m_firstNum || n > m_lastNum) {
    return "";
  }

  ZString str(m_prefix);
  str.appendNumber(n, m_numWidth);
  if (m_suffix.empty()) {
    str += ".tif";
  } else {
    str += m_suffix;
  }

  return str.absolutePath(m_urlList[0]);
}

ZFileList* ZStackFile::toFileList() const
{
  if (m_urlList.empty()) {
    return NULL;
  }

  ZFileList *fileList = NULL;
  ZString filePath = m_urlList[0];
  switch (m_type) {
  case SINGLE_FILE:
    fileList = new ZFileList(1);
    fileList->setFilePath(1, m_urlList[0]);
    break;
  case FILE_LIST:
    fileList = new ZFileList(m_urlList.size());
    for (size_t i = 0; i < m_urlList.size(); i++) {
      fileList->setFilePath(i, m_urlList[i]);
    }
    break;
  case IMAGE_SERIES:
    fileList = new ZFileList;
    fileList->load(filePath.dirPath(), filePath.toFileExt(),
                   ZFileList::SORT_BY_LAST_NUMBER);
    break;
  case FILE_BUNDLE:
    fileList = new ZFileList(m_lastNum - m_firstNum + 1);
    for (int i = m_firstNum; i <= m_lastNum; i++) {
      fileList->setFilePath(i - m_firstNum, fileBundlePath(i));
    }
    break;
  default:
    break;
  }

  return fileList;
}

File_Bundle_S ZStackFile::toFileBundleS() const
{
  File_Bundle_S fs;

  if (!m_urlList.empty()) {
    fs.prefix = const_cast<char*>(
          ZString::absolutePath(m_urlList[0], m_prefix).c_str());
    fs.suffix = const_cast<char*>(m_suffix.c_str());
  } else {
    fs.prefix = NULL;
    fs.suffix = NULL;
  }

  fs.first_num = m_firstNum;
  fs.last_num = m_lastNum;
  fs.num_width = m_numWidth;

  return fs;
}

ZStack* ZStackFile::readStack(ZStack *data)
{
  bool failed = false;

  if (!m_urlList.empty()) {
    switch (m_type) {
    case SINGLE_FILE:
    {
      Mc_Stack *stack = NULL;
      int offset[3] = {0, 0, 0};
      if (ZFileType::fileType(m_urlList[0].c_str()) ==
          ZFileType::OBJECT_SCAN_FILE ||
          ZFileType::fileType(m_urlList[0].c_str()) ==
                    ZFileType::DVID_OBJECT_FILE) {
        ZObject3dScan obj;
        if (obj.load(m_urlList[0])) {
          ZObject3d *obj3d = obj.toObject3d();

          Stack *tmpstack = obj3d->toStack(offset);
          stack = C_Stack::make(
                GREY, C_Stack::width(tmpstack), C_Stack::height(tmpstack),
                C_Stack::depth(tmpstack), 1);
          C_Stack::copyChannelValue(stack, 0, tmpstack);
          C_Stack::kill(tmpstack);
        }
      } else {
        stack = C_Stack::read(m_urlList[0].c_str(), m_channel);
      }

      if (stack == NULL) {
        failed = true;
      } else {
        if (data == NULL) {
          data = new ZStack();
        }
        data->setData(stack);
        data->setOffset(offset[0], offset[1], offset[2]);
        data->initChannelColors();
#ifdef _NEUTUBE_
        data->getLSMInfo(m_urlList[0].c_str());
#endif
      }
    }
      break;
    case FILE_BUNDLE:
    {
      File_Bundle_S fs = toFileBundleS();
      Stack *stack = Read_Stack_Planes_S(&fs);
      if (stack == NULL) {
        failed = true;
      } else {
        if (data == NULL) {
          data = new ZStack();
        }
        data->load(stack);
      }
    }
      break;
    case FILE_LIST:
    {
      Mc_Stack *stack = NULL;
      if (!m_urlList.empty()) {
        int nchannel = ZStack::getChannelNumber(m_urlList[0]);
        int kind, width, height, depth;

        retrieveAttribute(&kind, &width, &height, &depth);
        if (kind == COLOR) {
          kind = GREY;
        }
        stack = Make_Mc_Stack(kind, width, height, depth, nchannel);

        for (int i = 0; i < C_Stack::channelNumber(stack); i++) {
          for (size_t j = 0; j < m_urlList.size(); j++) {
            Stack *slice = Read_Sc_Stack(m_urlList[j].c_str(), i);
            C_Stack::copyPlaneValue(stack, slice->array, i, j);
            C_Stack::kill(slice);
          }
        }

        if (data == NULL) {
          data = new ZStack;
        }
        data->setData(stack);
      } else {
        failed = true;
      }
    }
      break;
    case IMAGE_SERIES:
    {
      ZFileList fileList;
      ZString str(m_urlList[0]);
      fileList.load(str.dirPath(), str.toFileExt(),
                    ZFileList::SORT_BY_LAST_NUMBER);

      if (fileList.size() > 0) {
        int nchannel = ZStack::getChannelNumber(fileList.getFilePath(0));
        Stack *slice = Read_Sc_Stack(fileList.getFilePath(0), 0);
        int kind = C_Stack::kind(slice);
        int width = C_Stack::width(slice);
        int height = C_Stack::height(slice);
        int depth = fileList.size();
        Mc_Stack *stack = Make_Mc_Stack(kind, width, height, depth, nchannel);
        C_Stack::kill(slice);

        for (int i = 0; i < nchannel; i++) {
          Stack *slice = Read_Sc_Stack(fileList.getFilePath(0), i);

          for (int j = 0; j < fileList.size(); j++) {
            slice = Read_Sc_Stack(fileList.getFilePath(j), i);
            C_Stack::copyPlaneValue(stack, slice->array, i, j);
            C_Stack::kill(slice);
          }
        }

        if (data == NULL) {
          data = new ZStack;
        }
        data->setData(stack);
      } else {
        failed = true;
      }
    }
      break;
    case HDF5_DATASET:
    {
      ZHdf5Reader reader;
      reader.open(m_urlList[0]);
      mylib::Array *array = reader.readArray(m_prefix);
      if (array != NULL) {
        if (data == NULL) {
          data = new ZStack();
        }
        int kind = GREY;
        switch (array->type) {
        case mylib::UINT8_TYPE:
          kind = GREY;
          break;
        case mylib::UINT16_TYPE:
          kind = GREY16;
          break;
        case mylib::FLOAT32_TYPE:
          kind = FLOAT32;
          break;
        case mylib::FLOAT64_TYPE:
          kind = FLOAT64;
          break;
        default:
          cout << "Unknown type." << endl;
          break;
        }

        int width = 0;
        int height = 1;
        int depth = 1;
        int channelNumber = 1;

        bool flipDim = m_dimFlip;

        if (flipDim) {
          width = array->dims[0];

          if (array->ndims > 1) {
            height = array->dims[1];
          }

          if (array->ndims > 2) {
            depth = array->dims[2];
          }

          if (array->ndims > 3) {
            channelNumber = array->dims[3];
          }
        } else {
          channelNumber = array->dims[0];

          if (array->ndims > 1) {
            depth = array->dims[1];
          }

          if (array->ndims > 2) {
            height = array->dims[2];
          }

          if (array->ndims > 3) {
            width = array->dims[3];
          }
        }

        Mc_Stack *stack = Make_Mc_Stack(kind, width, height, depth, channelNumber);
        data->setData(stack);

#define FLIP_DIM(inArray, outArray) \
        {\
          int area = depth * height; \
          size_t index = 0; \
          for (int ch= 0; ch < channelNumber; ++ch) {\
            for (int z = 0; z < depth; ++z) {\
              for (int y = 0; y < height; ++y) {\
                for (int x = 0; x < width; ++x) {\
                  outArray[index] =\
                  inArray[ch + (x * area + y * height + z) * channelNumber];\
                  ++index;\
                }\
              }\
            }\
          }\
        }

        if (flipDim) {
          //size_t voxelNumber = C_Stack::voxelNumber(stack);
          Image_Array ima1;
          Image_Array ima2;
          ima1.array = (uint8*) array->data;
          ima2.array = stack->array;
          switch (C_Stack::kind(stack)) {
          case GREY:
            FLIP_DIM(ima1.array8, ima2.array8);
            break;
          case GREY16:
            FLIP_DIM(ima1.array16, ima2.array16);
            break;
          case FLOAT32:
            FLIP_DIM(ima1.array32, ima2.array32);
            break;
          case FLOAT64:
            FLIP_DIM(ima1.array64, ima2.array64);
            break;
          }

        } else {
          memcpy(stack->array, array->data, data->getByteNumber());
        }

        Kill_Array(array);
      }
    }
      break;
    case SOBJ_LIST:
    {
      std::vector<ZObject3dScan> objArray(m_urlList.size());

      //Load each object
      for (size_t i = 0; i < m_urlList.size(); ++i) {
        objArray[i].load(m_urlList[i]);
      }

      Cuboid_I boundBox;
      objArray[0].getBoundBox(&boundBox);

      //Get Bound box
      for (size_t i = 1; i < objArray.size(); i++) {
        Cuboid_I subBoundBox;
        objArray[i].getBoundBox(&subBoundBox);
        Cuboid_I_Union(&boundBox, &subBoundBox, &boundBox);
      }

      int width, height, depth;
      Cuboid_I_Size(&boundBox, &width, &height, &depth);
      //Create stack
      Stack *stack = C_Stack::make(GREY, width, height, depth);

      int offset[3];
      for (int i = 0; i < 3; ++i) {
        offset[i] = -boundBox.cb[i];
      }
      for (size_t i = 0; i < objArray.size(); i++) {
        objArray[i].drawStack(stack, i + 1, offset);
      }
      if (data == NULL) {
        data = new ZStack;
      }
      data->load(stack);
    }
      break;
    default:
      break;
    }
  }

  if (failed) {
    cout << "Failed to read stack: " << endl;
    this->print();
    return NULL;
  }

  return data;
}

void ZStackFile::print()
{
  if (m_urlList.empty()) {
    cout << "No file associated." << endl;
  }
  switch(m_type) {
  case SINGLE_FILE:
    cout << m_urlList[0] << endl;
    break;
  case FILE_BUNDLE:
    cout << "File bundle:" << endl;
    cout << m_urlList[0] << endl;
    cout << "Number width" << m_numWidth << endl;
    cout << "First number" << m_firstNum << endl;
    cout << "Last number" << m_lastNum << endl;
    break;
  case FILE_LIST:
    cout << m_urlList.size() << " files:" << endl;
    for (size_t i = 0; i < m_urlList.size(); i++) {
      cout << m_urlList[i] << endl;
    }
    break;
  case IMAGE_SERIES:
    cout << "Image series: " << m_urlList[0] << endl;
    break;
  case HDF5_DATASET:
    cout << "HDF5 dataset: " << m_urlList[0] << " $ " << m_prefix << endl;
    break;
  default:
    cout << "Unidentified file." << endl;
    break;
  }
}

void ZStackFile::appendUrl(const string &path)
{
  m_urlList.push_back(path);
}
