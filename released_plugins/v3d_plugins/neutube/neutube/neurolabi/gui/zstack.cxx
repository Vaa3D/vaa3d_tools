#include "zqtheader.h"

#ifdef _QT_GUI_USED_
#include <QDebug>
#include <QFileInfo>
#include <QDir>
#endif
#include <string.h>
#include <iostream>
#include <cmath>
#include "tz_utilities.h"
#include "tz_stack_lib.h"
#include "zstack.hxx"
#include "tz_fimage_lib.h"
#include "tz_xml_utils.h"
#include "tz_stack_relation.h"
#include "zfilelist.h"
#include "c_stack.h"
#include "tz_stack.h"
#include "tz_stack_objlabel.h"
#include "tz_int_histogram.h"
#include "zjsonparser.h"
#include "zfiletype.h"
#ifdef _NEUTUBE_
#include "QsLog.h"
#endif

using namespace std;

ZStack::ZStack() : m_stack(NULL), m_delloc(NULL), m_preferredZScale(1.0),
  m_isLSMFile(false)
{
  m_buffer[0] = '\0';
}

ZStack::ZStack(int kind, int width, int height, int depth,
               int nchannel, bool isVirtual)
  : m_preferredZScale(1.0), m_isLSMFile(false)
{  
  m_buffer[0] = '\0';
 // m_proj = NULL;
 // m_stat = NULL;

  Mc_Stack *stack = NULL;
  C_Stack::Mc_Stack_Deallocator *delloc = NULL;
  if (isVirtual) {
    stack = new Mc_Stack;
    stack->array = NULL;
    C_Stack::setAttribute(stack, kind, width, height, depth, nchannel);
    delloc = C_Stack::cppDelete;
  } else {
    stack = Make_Mc_Stack(kind, width, height, depth, nchannel);
    delloc = Kill_Mc_Stack;
  }

  m_delloc = NULL;
  setData(stack, delloc);
}

ZStack::ZStack(Mc_Stack *stack, C_Stack::Mc_Stack_Deallocator *dealloc)
{
  setData(stack, dealloc);
}

ZStack::~ZStack()
{
  clean();
}

size_t ZStack::getByteNumber(EStackUnit unit) const
{
  if (unit == WHOLE_DATA) {
    return getVoxelNumber() * channelNumber() * kind();
  } else {
    return getVoxelNumber(unit) * kind();
  }
}

void ZStack::setOffset(double dx, double dy, double dz)
{
  m_offset.set(dx, dy, dz);
}

size_t ZStack::getVoxelNumber(EStackUnit unit) const
{
  switch (unit) {
  case SINGLE_VOXEL:
    return 1;
  case SINGLE_ROW:
    return width();
  case SINGLE_PLANE:
    return getVoxelNumber(SINGLE_ROW) * height();
  case SINGLE_CHANNEL:
  case WHOLE_DATA:
    return getVoxelNumber(SINGLE_PLANE) * depth();
  }

  return 0;
}

void ZStack::setData(Mc_Stack *stack, C_Stack::Mc_Stack_Deallocator *delloc)
{
  deprecate(MC_STACK);
  m_stack = stack;
  m_delloc = delloc;
}

/*
ZStack::ZStack(const ZStack &src)
  : m_isLSMFile(false)
{
  init(src.channel());
  m_source = Copy_Stack_Document(src.source());
  m_preferredZScale = src.m_preferredZScale;
  for (int i=0; i<src.channel(); i++) {
    m_singleChannelStackVector[i] =
        new ZSingleChannelStack(*(src.m_singleChannelStackVector[i]));
  }
}
*/

const Stack* ZStack::c_stack(int c) const
{
  const void *dataArray = rawChannelData(c);

  if (dataArray != NULL) {
    m_stackView.resize(channelNumber());
    C_Stack::setAttribute(&(m_stackView[c]), kind(), width(), height(), depth());

    m_stackView[c].array = (uint8*) dataArray;
    m_stackView[c].text = m_buffer;

    return &(m_stackView[c]);
  }

  return NULL;
}

Stack* ZStack::c_stack(int c)
{
  return const_cast<Stack*>(
        static_cast<const ZStack&>(*this).c_stack(c));
}

const void* ZStack::rawChannelData(int c) const
{
  void *array = NULL;

  if (c >= 0 && c < channelNumber()) {
    array = m_stack->array + getByteNumber(SINGLE_CHANNEL) * c;
  }

  return array;
}

void* ZStack::rawChannelData(int c)
{
  return const_cast<void*>(static_cast<const ZStack&>(*this).rawChannelData(c));
}

ZSingleChannelStack* ZStack::singleChannelStack(int c)
{
  return const_cast<ZSingleChannelStack*>(
        static_cast<const ZStack&>(*this).singleChannelStack(c));
}

void ZStack::deprecateSingleChannelView(int c)
{
  if (c < (int) m_singleChannelStack.size()) {
    delete m_singleChannelStack[c];
    m_singleChannelStack[c] = NULL;
  }
}

bool ZStack::isSingleChannelViewDeprecated(int channel) const
{
  bool obselete = true;

  if (channel < (int) m_singleChannelStack.size()) {
    obselete = m_singleChannelStack[channel] == NULL;
  }

  return obselete;
}

const ZSingleChannelStack* ZStack::singleChannelStack(int c) const
{
  if (isSingleChannelViewDeprecated(c)) {
    if (c >= (int) m_singleChannelStack.size()) {
      m_singleChannelStack.resize(c + 1, NULL);
    }
    m_singleChannelStack[c] = new ZSingleChannelStack;
    m_singleChannelStack[c]->setData(const_cast<Stack*>(c_stack(c)), NULL);
  }

  return m_singleChannelStack[c];

  /*
  m_singleChannelStack.setData(const_cast<Stack*>(c_stack(c)), false);

  return &m_singleChannelStack;
  */
}

/*
bool ZStack::releaseOwnership(int c)
{
  return m_singleChannelStackVector[c]->releaseOwnership();
}
*/
void ZStack::shiftLocation(int *offset, int c, int width, int height, int depth)
{
  singleChannelStack(c)->shiftLocation(offset, width, height, depth);
}

Mc_Stack *ZStack::makeMcStack(const Stack *stack1, const Stack *stack2, const Stack *stack3)
{
  Mc_Stack *out = NULL;

  if (stack1 != NULL && stack1->kind == 1 && Stack_Same_Attribute(stack1, stack2)) {
    if (stack3 == NULL || Stack_Same_Attribute(stack2, stack3)) {
      out = Make_Mc_Stack(1, stack1->width, stack1->height, stack1->depth, 3);
      size_t volume = Stack_Voxel_Number(stack1);
      memcpy(out->array, stack1->array, volume);
      memcpy(out->array+volume, stack2->array, volume);
      if (stack3 != NULL) {
        memcpy(out->array+volume*2, stack3->array, volume);
      } else {
        memset(out->array + volume*2, 0, volume);
      }
    }
  }

  return out;
}

void ZStack::subMostCommonValue(int c)
{
  if (c < channelNumber())
  {
    singleChannelStack(c)->subMostCommonValue();
  }
}

Stack *ZStack::averageOfAllChannels()
{
  Stack *stack = NULL;
  int nchannel = channelNumber();
  if (nchannel == 1) {
    stack = Copy_Stack(c_stack());
  }
  if (nchannel > 1) {
    stack = Make_Stack(data()->kind, data()->width, data()->height, data()->depth);
    size_t nvoxel = getVoxelNumber();
    Image_Array ima;
    ima.array = stack->array;
    int k = kind();
    switch (k) {
    case GREY:
      for (size_t i=0; i<nvoxel; i++) {
        double value = 0.0;
        for (int j=0; j<nchannel; j++) {
          value += c_stack(j)->array[i];
        }
        value /= nchannel;
        ima.array8[i] = value;
      }
      break;
    case GREY16:
      for (size_t i=0; i<nvoxel; i++) {
        double value = 0.0;
        for (int j=0; j<nchannel; j++) {
          value += ((uint16*)c_stack(j)->array)[i];
        }
        value /= nchannel;
        ima.array16[i] = value;
      }
      break;
    case FLOAT32:
      for (size_t i=0; i<nvoxel; i++) {
        double value = 0.0;
        for (int j=0; j<nchannel; j++) {
          value += ((float*)c_stack(j)->array)[i];
        }
        value /= nchannel;
        ima.array32[i] = value;
      }
      break;
    case FLOAT64:
      for (size_t i=0; i<nvoxel; i++) {
        double value = 0.0;
        for (int j=0; j<nchannel; j++) {
          value += ((double*)c_stack(j)->array)[i];
        }
        value /= nchannel;
        ima.array64[i] = value;
      }
      break;
    }
  }
  return stack;
}


void ZStack::init()
{
  //m_singleChannelStackVector.resize(nchannel);
  m_preferredZScale = 1.0;
  //m_source = NULL;
}

bool ZStack::canMerge(const Stack *s1, const Stack *s2)
{
  if ((s1 && C_Stack::kind(s1) == 3) || (s2 && C_Stack::kind(s2) == 3))
    return false;
  if (s1 == NULL || s2 == NULL)
    return true;
  else
    return Stack_Same_Attribute(s1, s2);
}

void ZStack::deprecateDependent(EComponent component)
{
  switch (component) {
  case MC_STACK:
    //deprecate(STACK_PROJECTION);
    //deprecate(STACK_STAT);
    deprecate(SINGLE_CHANNEL_VIEW);
    break;
  case SINGLE_CHANNEL_VIEW:
    break;
    /*
  case STACK_PROJECTION:
    break;
  case STACK_STAT:
    break;
    */
  }
}

void ZStack::deprecate(EComponent component)
{
  deprecateDependent(component);

  switch (component) {
  case MC_STACK:  
    if (m_stack != NULL && m_delloc != NULL) {
      m_delloc(m_stack);
    }
    m_stack = NULL;
    m_delloc = NULL;
    break;
    /*
  case STACK_PROJECTION:
    delete m_proj;
    m_proj = NULL;
    break;
  case STACK_STAT:
    delete m_stat;
    m_stat = NULL;
    break;
    */
  case SINGLE_CHANNEL_VIEW:
    for (size_t i = 0; i < m_singleChannelStack.size(); ++i) {
      delete m_singleChannelStack[i];
      m_singleChannelStack[i] = NULL;
    }
    break;
  }
}

bool ZStack::isDeprecated(EComponent component) const
{
  switch (component) {
  case MC_STACK:
    return m_stack == NULL;
    break;
    /*
  case STACK_PROJECTION:
    return m_proj == NULL;
    break;
  case STACK_STAT:
    return m_stat == NULL;
    break;
    */
  case SINGLE_CHANNEL_VIEW:
    for (int c = 0; c < channelNumber(); ++c) {
      if (isSingleChannelViewDeprecated(c)) {
        return true;
      }
    }
    break;
  }

  return false;
}

void ZStack::clean()
{
  deprecate(ZStack::MC_STACK);

  /*
  if (m_stack != NULL) {
    m_delloc(m_stack);
  }
  */

  /*
  for (size_t i=0; i<m_singleChannelStackVector.size(); i++) {
    if (m_singleChannelStackVector[i] != NULL) {
      delete m_singleChannelStackVector[i];
    }
  }
  m_singleChannelStackVector.clear();
*/

#ifdef _NEUTUBE_
  for (size_t i=0; i<m_channelColors.size(); ++i)
    delete m_channelColors[i];
  m_channelColors.clear();
#endif
}

void ZStack::setChannelNumber(int nchannel)
{
  C_Stack::setChannelNumber(m_stack, nchannel);
}

void ZStack::initChannelColors()
{
#ifdef _NEUTUBE_
  if (m_channelColors.size() == (size_t)channelNumber()) {
    return;
  }
  for (int i=0; i<channelNumber(); ++i) {
    m_channelColors.push_back(new ZVec3Parameter(QString("Ch%1").arg(i+1),
                                                 glm::vec3(0.f)));
    m_channelColors[i]->setStyle("COLOR");
  }
  if (channelNumber() == 1)
    m_channelColors[0]->set(glm::vec3(1.f,1.f,1.f));
  else {
    m_channelColors[0]->set(glm::vec3(1.f,0.f,0.f));
    m_channelColors[1]->set(glm::vec3(0.f,1.f,0.f));
    if (channelNumber() > 2)
      m_channelColors[2]->set(glm::vec3(0.f,0.f,1.f));
  }
#endif
}

void ZStack::removeChannel(int c)
{
  if (c >= 0 && c < channelNumber()) {
    size_t byteNumber = getByteNumber(SINGLE_CHANNEL);
    for (int k = c + 1; k < channelNumber(); ++k) {
      void *dst = rawChannelData(c - 1);
      void *src = rawChannelData(c);
      memmove(dst, src, byteNumber);
    }
    deprecateDependent(MC_STACK);
  }
}

/*
void ZStack::cleanChannel(int c)
{
  if (m_singleChannelStackVector[c] != NULL) {
    delete m_singleChannelStackVector[c];
    m_singleChannelStackVector[c] = NULL;
  }
}

void ZStack::removeChannel(int c)
{
  cleanChannel(c);
  m_singleChannelStackVector.erase(m_singleChannelStackVector.begin()+c);
}
*/

bool ZStack::load(Stack *stack, bool isOwner)
{
  deprecate(MC_STACK);

  if (C_Stack::kind(stack) == 3) {
    m_stack = Make_Mc_Stack(C_Stack::kind(stack), C_Stack::width(stack), C_Stack::height(stack),
                            C_Stack::depth(stack), 3);
    m_delloc = C_Stack::kill;
    Stack *stack0 = C_Stack::channelExtraction(stack, 0);
    C_Stack::copyChannelValue(m_stack, 0, stack0);
    C_Stack::kill(stack0);
    Stack *stack1 = C_Stack::channelExtraction(stack, 1);
    C_Stack::copyChannelValue(m_stack, 1, stack1);
    C_Stack::kill(stack1);
    Stack *stack2 = C_Stack::channelExtraction(stack, 2);
    C_Stack::copyChannelValue(m_stack, 2, stack2);
    C_Stack::kill(stack2);
    if (isOwner)
      C_Stack::kill(stack);
  } else {
    m_stack = new Mc_Stack;

    C_Stack::view(stack, m_stack);

    if (isOwner) {
      stack->array = NULL;
      Kill_Stack(stack);
      m_delloc = C_Stack::systemKill;
    } else {
      m_delloc = NULL;
    }
  }

  /*
  clean();
  if (stack->kind == COLOR) {
    init(3);
    Stack *stack0 = Stack_Channel_Extraction(stack, 0, NULL);
    m_singleChannelStackVector[0] = new ZSingleChannelStack(stack0, true);
    Stack *stack1 = Stack_Channel_Extraction(stack, 1, NULL);
    m_singleChannelStackVector[1] = new ZSingleChannelStack(stack1, true);
    Stack *stack2 = Stack_Channel_Extraction(stack, 2, NULL);
    m_singleChannelStackVector[2] = new ZSingleChannelStack(stack2, true);
    if (isOwner) {
      Kill_Stack(stack);
    }
  } else {
    init(1);
    m_singleChannelStackVector[0] = new ZSingleChannelStack(stack, isOwner);
  }
  */
  return true;
}

bool ZStack::load(const string &filepath)
{
  deprecate(MC_STACK);

  ZStackFile stackFile;
  stackFile.import(filepath);

  ZStack *res = stackFile.readStack(this);
  if (res)
    res->setSource(filepath);

  return res;

  /*
  if (!filepath.empty()) {
    clean();
    int nchannel = getChannelNumber(filepath.c_str());
    if (nchannel > 0) {
      init(nchannel);
      for (int i=0; i<nchannel; i++) {
        Stack *stack = Read_Sc_Stack(filepath.c_str(), i);
        m_singleChannelStackVector[i] = new ZSingleChannelStack(stack, true);
      }
      setSource(filepath);
    } else { //try other method
      Stack *stack = Read_Stack_U(filepath.c_str());
      if (stack != NULL) {
        load(stack, true);
        setSource(filepath);
      } else {
        return false;
      }
    }
    getLSMInfo(QString::fromStdString(filepath));
    return true;
  }
*/
}

bool ZStack::load(const Stack *ch1, const Stack *ch2, const Stack *ch3)
{
  deprecate(MC_STACK);

  if (ch1 == NULL && ch2 == NULL && ch3 == NULL)
    return false;
  if (!canMerge(ch1, ch2) || !canMerge(ch2, ch3) || !canMerge(ch1, ch3))
    return false;

  if (ch3 != NULL) {
    m_stack = Make_Mc_Stack(C_Stack::kind(ch3), C_Stack::width(ch3), C_Stack::height(ch3),
                            C_Stack::depth(ch3), 3);
    m_delloc = C_Stack::kill;
    C_Stack::copyChannelValue(m_stack, 2, ch3);
    if (ch2 != NULL) {
      C_Stack::copyChannelValue(m_stack, 1, ch2);
    }
    if (ch1 != NULL) {
      C_Stack::copyChannelValue(m_stack, 0, ch1);
    }
  } else if (ch2 != NULL) {
    m_stack = Make_Mc_Stack(C_Stack::kind(ch2), C_Stack::width(ch2), C_Stack::height(ch2),
                            C_Stack::depth(ch2), 2);
    m_delloc = C_Stack::kill;
    C_Stack::copyChannelValue(m_stack, 1, ch2);
    if (ch1 != NULL) {
      C_Stack::copyChannelValue(m_stack, 0, ch1);
    }
  } else {
    m_stack = Make_Mc_Stack(C_Stack::kind(ch1), C_Stack::width(ch1), C_Stack::height(ch1),
                            C_Stack::depth(ch1), 1);
    m_delloc = C_Stack::kill;
    C_Stack::copyChannelValue(m_stack, 0, ch1);
  }
  return true;
}


void ZStack::setSource(const string &filepath, int channel)
{
  m_source.import(filepath);
  m_source.setChannel(channel);
}

void ZStack::setSource(const ZStackFile &file)
{
  m_source = file;
}

void ZStack::setSource(Stack_Document *stackDoc)
{
  m_source.loadStackDocument(stackDoc);
}

void ZStack::setResolution(double x, double y, double z, char unit)
{
  /*
  if (m_source == NULL) {
    m_source = New_Stack_Document();
  }

  m_source->resolution[0] = x;
  m_source->resolution[1] = y;
  m_source->resolution[2] = z;
  m_source->unit = unit;
  */
  m_resolution.setVoxelSize(x, y, z);
  m_resolution.setUnit(unit);
  m_preferredZScale = z / (.5 * (x + y));
}

int ZStack::getChannelNumber(const string &filepath)
{
  int nchannel = 0;
  ZFileType::EFileType type = ZFileType::fileType(filepath);

  if (type == ZFileType::TIFF_FILE ||
      type == ZFileType::LSM_FILE) {
    Tiff_Reader *reader;
    if (type == ZFileType::TIFF_FILE) {
      reader = Open_Tiff_Reader((char*) filepath.c_str(), NULL, 0);
    } else {
      reader = Open_Tiff_Reader((char*) filepath.c_str(), NULL, 1);
    }

    Tiff_Type type = TIFF_BYTE;
    int count = 0;
    Tiff_IFD *ifd = Read_Tiff_IFD(reader);
    uint32_t *val = (uint32_t*)Get_Tiff_Tag(ifd,TIFF_NEW_SUB_FILE_TYPE,&type,&count);
    if (val != NULL) {
      while (*val == 1) {
        Free_Tiff_IFD(ifd);
        Advance_Tiff_Reader(reader);
        if (End_Of_Tiff(reader)) {
          ifd = NULL;
          Free_Tiff_Reader(reader);
          return 0;
        }
        ifd = Read_Tiff_IFD(reader);
        val = (uint32_t*)Get_Tiff_Tag(ifd,TIFF_NEW_SUB_FILE_TYPE,&type,&count);
        if (val == NULL) {
          Free_Tiff_IFD(ifd);
          Free_Tiff_Reader(reader);
          return 0;
        }
      }
    } else {  // try TIFF_IMAGE_WIDTH
      val = (uint32_t*)Get_Tiff_Tag(ifd,TIFF_IMAGE_WIDTH,&type,&count);
      if (val == NULL) {
        Free_Tiff_IFD(ifd);
        Free_Tiff_Reader(reader);
        return 0;
      }
    }

    Tiff_Image *image = Extract_Image_From_IFD(ifd);

    if (image == NULL) {
      return 0;
    }

    nchannel = image->number_channels;
    Kill_Tiff_Image(image);
    Free_Tiff_Reader(reader);
  } else if (type == ZFileType::V3D_RAW_FILE) {
    FILE *fp = Guarded_Fopen(filepath.c_str(), "rb", "Read_Raw_Stack_C");

    char formatkey[] = "raw_image_stack_by_hpeng";
    int lenkey = strlen(formatkey);
    fread(formatkey, 1, lenkey, fp);

    if (strcmp(formatkey, "raw_image_stack_by_hpeng") != 0) {
      fclose(fp);
      return 0;
    }

    char endian;
    fread(&endian, 1, 1, fp);

    uint16_t dataType;
    char sz_buffer[16];
    uint32_t sz[4];

    fread(&dataType, 2, 1, fp);
    fread(sz_buffer, 1, 8, fp);

    int i;
    for (i = 0; i < 4; i++) {
      sz[i] = *((uint16_t*) (sz_buffer + i * 2));
    }

    if ((sz[0] == 0) || (sz[1] == 0) || (sz[2] == 0) || (sz[3] == 0)) {
      fread(sz_buffer + 8, 1, 8, fp);

      for (i = 0; i < 4; i++) {
        sz[i] = *((uint32_t*) (sz_buffer + i * 4));
      }
    }

    nchannel = sz[3];
    fclose(fp);
  } else if (type == ZFileType::PNG_FILE) {
    //No support for multi-channel png yet
    return 1;
  }

  return nchannel;
}

/*
Stack* ZStack::channelData(int c)
{
  if (c < 0 || c >= channel()) {
    return NULL;
  }

  return m_singleChannelStackVector[c]->data();
}
*/

string ZStack::save(const string &filepath) const
{
  if (channelNumber() == 0)
    return string();
  if (channelNumber() == 1) {  //should be fine
    Write_Stack_U(filepath.c_str(), c_stack(), NULL);
    return filepath;
  } else if (kind() == GREY || kind() == GREY16) {
    /*
    Mc_Stack *stack = makeMcStack(c_stack(0), c_stack(1),
                                  c_stack(2));
                                  */
    /*
    m_singleChannelStackVector[0]->data(),
                                  m_singleChannelStackVector[1]->data(),
                                  m_singleChannelStackVector[2]->data());
                                  */
    Write_Mc_Stack(filepath.c_str(), m_stack, NULL);
    return filepath;
  } else {  //save as raw
    string str = filepath;
    if (ZFileType::fileType(filepath) != ZFileType::V3D_RAW_FILE) {
      std::cout << "Unsupported data format for " << str << endl;
      str += ".raw";
      std::cout << str << " saved instead." << endl;
    }

    Write_Mc_Stack(str.c_str(), m_stack, NULL);

    //    FILE *fp = Guarded_Fopen(str.c_str(), "wb", "Write_Raw_Stack");

    //    char formatkey[] = "raw_image_stack_by_hpeng";
    //    int lenkey = strlen(formatkey);
    //    fwrite(formatkey, 1, lenkey, fp);

    //    char endian = 'L';
    //    fwrite(&endian, 1, 1, fp);

    //    uint16_t dataType = kind();
    //    uint32_t sz[4];
    //    sz[0] = width();
    //    sz[1] = height();
    //    sz[2] = depth();
    //    sz[3] = channelNumber();

    //    fwrite(&dataType, 2, 1, fp);
    //    fwrite(sz, 4, 4, fp);

    //    size_t nvoxel = ((size_t) width()) * ((size_t) height()) *
    //        ((size_t) depth());
    //    fclose(fp);
    //    for (int i = 0; i < channelNumber(); ++i) {
    //      fp = Guarded_Fopen(str.c_str(), "ab", "Write_Raw_Stack");
    //      //printf("%zd, %ld\n", nvoxel, ftell(fp));
    //#ifdef _QT_GUI_USED_
    //      qDebug() << nvoxel << ", " << ftell(fp);
    //#endif
    //      size_t num = fwrite(rawChannelData(i), dataType, nvoxel, fp);
    //      //printf("%zd, %ld\n", num, ftell(fp));
    // #ifdef _QT_GUI_USED_
    //      qDebug() << num << ", " << ftell(fp);
    //#endif
    //      fclose(fp);
    //    }
    return str;
  }
}

void* ZStack::projection(ZSingleChannelStack::Proj_Mode mode, ZSingleChannelStack::Stack_Axis axis, int c)
{
  return singleChannelStack(c)->projection(mode, axis);
}

double ZStack::value(int x, int y, int z, int c) const
{
  if (!(IS_IN_CLOSE_RANGE(x, 0, width() - 1) &&
        IS_IN_CLOSE_RANGE(y, 0, height() - 1) &&
        IS_IN_CLOSE_RANGE(c, 0, channelNumber() - 1))) {
    return 0.0;
  }

  if (z < 0) {
    z = maxIntensityDepth(x, y, c);
  } else if (!(IS_IN_CLOSE_RANGE(z, 0, depth() - 1))) {
    return 0.0;
  }

  return singleChannelStack(c)->value(x, y, z);
}

double ZStack::saturatedIntensity() const
{
  if (kind() == 1)
    return 255;
  if (kind() == 2) {
    for (int ch=0; ch<channelNumber(); ++ch) {
      if (const_cast<ZStack*>(this)->max(ch) > 4095)
        return 65535;
      else
        return 4095;
    }
  }
  return 1.0;
}

double ZStack::value(size_t index, int c) const
{
  return singleChannelStack(c)->value(index);
}

void ZStack::setValue(int x, int y, int z, int c, double v)
{
  if (!(IS_IN_CLOSE_RANGE(x, 0, width() - 1) &&
        IS_IN_CLOSE_RANGE(y, 0, height() - 1) &&
        IS_IN_CLOSE_RANGE(z, 0, depth() - 1) &&
        IS_IN_CLOSE_RANGE(c, 0, channelNumber() - 1))) {
    return;
  }

  singleChannelStack(c)->setValue(x, y, z, v);
}

int ZStack::autoThreshold(int ch) const
{
  const Stack *stack = c_stack(ch);

  int thre = 0;
  if (stack->array != NULL) {
    double scale = 1.0*stack->width * stack->height * stack->depth * stack->kind /
        (2.0*1024*1024*1024);
    if (scale >= 1.0) {
      scale = std::ceil(std::sqrt(scale + 0.1));
      stack = C_Stack::resize(stack, stack->width/scale, stack->height/scale, stack->depth);
    }

    int conn = 18;
    Stack *locmax = Stack_Locmax_Region(stack, conn);
    Stack_Label_Objects_Ns(locmax, NULL, 1, 2, 3, conn);
    int nvoxel = Stack_Voxel_Number(locmax);
    int i;

    for (i = 0; i < nvoxel; i++) {
      if (locmax->array[i] < 3) {
        locmax->array[i] = 0;
      } else {
        locmax->array[i] = 1;
      }
    }

    int *hist = Stack_Hist_M(stack, locmax);
    Kill_Stack(locmax);

    int low, high;
    Int_Histogram_Range(hist, &low, &high);

    thre = Int_Histogram_Triangle_Threshold(hist, low, high - 1);

    if (stack != c_stack(ch))
      C_Stack::kill(const_cast<Stack*>(stack));
    free(hist);
  }
  return thre;
}

vector<double> ZStack::color(size_t index) const
{
  vector<double> c(channelNumber());
  for (size_t i = 0; i < c.size(); ++i) {
    c[i] = value(index, i);
  }

  return c;
}

bool ZStack::equalColor(size_t index, const std::vector<double> &co) const
{
  int cn = channelNumber();
  for (int i = 0; i < cn; ++i) {
    if (value(index, i) != co[i]) {
      return false;
    }
  }

  return true;
}

//The object itself must be uint8_t based
bool ZStack::equalColor(size_t index, const std::vector<uint8_t> &co) const
{
  int cn = channelNumber();
  for (int i = 0; i < cn; ++i) {
    if (value8(index, i) != co[i]) {
      return false;
    }
  }

  return true;
}

bool ZStack::equalColor(size_t index, const uint8_t *co, size_t length) const
{
  for (size_t i = 0; i < length; ++i) {
    if (value8(index, i) != co[i]) {
      return false;
    }
  }

  return true;
}

bool ZStack::equalColor(size_t index, size_t channelOffset, const uint8_t *co, size_t length) const
{
  const uint8_t *array = array8() + index;
  for (size_t i = 0; i < length; ++i) {
    if (*array != co[i]) {
      return false;
    }
    array += channelOffset;
  }

  return true;
}

vector<double> ZStack::color(int x, int y, int z) const
{
  vector<double> c(channelNumber());
  for (size_t i = 0; i < c.size(); ++i) {
    c[i] = value(x, y, z, i);
  }

  return c;
}

void ZStack::setValue(size_t index, int c, double value)
{
  singleChannelStack(c)->setValue(index, value);
}

int ZStack::maxIntensityDepth(int x, int y, int c) const
{
  return singleChannelStack(c)->maxIntensityDepth(x, y);
}

bool ZStack::isThresholdable()
{
  if (!isVirtual()) {
    if (channelNumber() == 1) {
      return true;
    }
  }

  return false;
}

bool ZStack::isTracable()
{
  return isThresholdable();
}

bool ZStack::isSwc()
{
  if (isVirtual()) {
    return ZFileType::fileType(m_source.firstUrl()) == ZFileType::SWC_FILE;
    /*
    if (m_source != NULL) {
      if (m_source->type == STACK_DOC_SWC_FILE) {
        return true;
      }
    }
    */
  }

  return false;
}

void ZStack::bcAdjustHint(double *scale, double *offset, int c)
{
  singleChannelStack(c)->bcAdjustHint(scale, offset);
  /*
  if (isDeprecated(STACK_STAT)) {
    m_stat = new ZStack_Stat();
    m_stat->update(c_stack(c));
  }

  *scale = m_stat->m_greyScale;
  *offset = m_stat->m_greyOffset;
  **/
}

bool ZStack::isBinary()
{
  if (isVirtual() || channelNumber() > 1) {
    return false;
  }
  return singleChannelStack(0)->isBinary();
}

bool ZStack::updateFromSource()
{
  /*
  if (m_source != NULL ) {   // use load to support 2-channel 16bit image       todo: add file bundle support as in Import_Stack_Document
    //    Stack *stack = Import_Stack_Document(m_source);
    //    if (stack != NULL) {
    //      clean();
    //      load(stack, true);
    //      return true;
    //    }
    return load(sourcePath());
  }

  return false;
  */

  if (m_source.readStack(this) == NULL) {
    return false;
  }



  return true;
}

bool ZStack::hasSameValue(size_t index1, size_t index2, size_t channelOffset)
{
  return C_Stack::hasSameValue(m_stack, index1, index2, channelOffset);
}

bool ZStack::hasSameValue(size_t index1, size_t index2)
{
  return C_Stack::hasSameValue(m_stack, index1, index2);
}

double ZStack::min()
{
  double minValue = min(0);

  for (int c = 1; c < channelNumber(); ++c) {
    double value = min(c);
    if (minValue > value) {
      minValue = value;
    }
  }

  return minValue;
}

double ZStack::min(int c) const
{
  return singleChannelStack(c)->min();
}

double ZStack::max()
{
  double maxValue = max(0);

  for (int c = 1; c < channelNumber(); ++c) {
    double value = max(c);
    if (maxValue < value) {
      maxValue = value;
    }
  }

  return maxValue;
}

double ZStack::max(int c) const
{
  return singleChannelStack(c)->max();
}

bool ZStack::binarize(int threshold)
{
  bool isChanged = false;

  if (!isVirtual() && isThresholdable()) {
    isChanged = singleChannelStack(0)->binarize(threshold);
    if (kind() != GREY) {
      Translate_Stack(singleChannelStack(0)->data(), GREY, 1);
      data()->kind = GREY;
    }
    if (isChanged) {
      deprecateDependent(MC_STACK);
    }
  }

  return isChanged;
}

bool ZStack::bwsolid()
{
  bool isChanged = false;
  if (isBinary()) {
    isChanged = singleChannelStack(0)->bwsolid();
    if (isChanged) {
      deprecateDependent(MC_STACK);
    }
  }

  return isChanged;
}

bool ZStack::enhanceLine()
{
  bool isChanged = false;

  if (!isVirtual() && channelNumber() == 1) {
    isChanged = singleChannelStack(0)->enhanceLine();
    if (isChanged) {
      deprecateDependent(MC_STACK);
    }
  }

  return isChanged;
}

void ZStack::extractChannel(int c)
{
  if (!isVirtual()) {
    memmove(array8(c), array8(0), getByteNumber(SINGLE_CHANNEL));
    C_Stack::setChannelNumber(m_stack, 1);
  }
}

Stack *ZStack::copyChannel(int c)
{
  Stack *out = NULL;
  if (!isVirtual() && c < channelNumber()) {
    out = Copy_Stack(c_stack(c));
  }
  return out;
}

const char* ZStack::sourcePath() const
{
  return m_source.firstUrl().c_str();
}

bool ZStack::isVirtual() const
{
  return m_stack->array == NULL;
}

void *ZStack::getDataPointer(int c, int slice) const
{
  const uint8_t *array = array8(c);
  array += getByteNumber(SINGLE_PLANE) * slice;

  return (void*) array;
}

bool ZStack::watershed(int c)
{
  if (!isVirtual() && c < channelNumber()) {
    return singleChannelStack(c)->watershed();
  }
  return false;
}

ZStack* ZStack::createSubstack(const std::vector<std::vector<double> > &selected)
{
  ZStack *substack =
      new ZStack(kind(), width(), height(), depth(), channelNumber());

  size_t volume = this->getVoxelNumber();

  for (size_t voxelIndex = 0; voxelIndex != volume; ++voxelIndex) {
    bool isSelected =  false;
    for (size_t selectIndex = 0; selectIndex < selected.size();
         ++selectIndex) {
      isSelected = equalColor(voxelIndex, selected[selectIndex]);
      if (isSelected) {
        break;
      }
    }

    for (int c = 0; c < channelNumber(); c++) {
      if (isSelected) {
        substack->setValue(voxelIndex, c, value(voxelIndex, c));
      } else {
        substack->setValue(voxelIndex, c, 0);
      }
    }
  }

  return substack;
}

ZStack* ZStack::clone() const
{
  ZStack *stack = new ZStack(
        kind(), width(), height(), depth(), channelNumber());
  memcpy(stack->rawChannelData(), rawChannelData(), getByteNumber());

  stack->m_resolution = m_resolution;
  stack->m_preferredZScale = m_preferredZScale;
  stack->m_source = m_source;

  return stack;
}

double ZStack::averageIntensity(ZStack *mask)
{
  size_t volume = getVoxelNumber();
  double v = 0.0;
  int count = 0;
  for (size_t i = 0; i < volume; ++i) {
    if (mask->value8(i) > 0) {
      v += value(i);
      ++count;
    }
  }

  v /= count;

  return v;
}

void ZStack::copyValue(const void *buffer, size_t length, int ch)
{
  memcpy(rawChannelData(ch), buffer, length);
  deprecateDependent(MC_STACK);
}

void ZStack::copyValue(const void *buffer, size_t length, void *loc)
{
  memcpy(loc, buffer, length);
  deprecateDependent(MC_STACK);
}

#ifdef _NEUTUBE_
void ZStack::setChannelColor(int ch, double r, double g, double b)
{
  m_channelColors[ch]->set(glm::vec3(r, g, b));
}

bool ZStack::getLSMInfo(const QString &filepath)
{
  if (!filepath.endsWith(".lsm", Qt::CaseInsensitive))
    return false;

  FILE *fp = fopen(filepath.toLocal8Bit().data(), "rb");

  uint16_t endian;
  fread(&endian, 2, 1, fp);
  if (endian != 0x4949) {
    fclose(fp);
    return false;
  }

  uint16_t magic;
  fread(&magic, 2, 1, fp);
  if (magic != 42) {
    fclose(fp);
    return false;
  }

  uint32_t ifd_offset;
  fread(&ifd_offset, 4, 1, fp);

  fseek(fp, ifd_offset, SEEK_SET);

  uint16_t nifd;
  fread(&nifd, 2, 1, fp);

  uint16_t ifd_label;
  fread(&ifd_label, 2, 1, fp);

  uint16_t i;
  for (i = 1; i < nifd; i++) {
    if (ifd_label == TIF_CZ_LSMINFO) {
      break;
    }
    fseek(fp, 10, SEEK_CUR);
    fread(&ifd_label, 2, 1, fp);
  }
  if (ifd_label != TIF_CZ_LSMINFO) {
    fclose(fp);
    return false;
  }

  uint16_t ifd_type;
  fread(&ifd_type, 2, 1, fp);

  uint32_t ifd_length;
  fread(&ifd_length, 4, 1, fp);

  fread(&ifd_offset, 4, 1, fp);

  fseek(fp, ifd_offset, SEEK_SET);
  fread(&m_lsmInfo, sizeof(Cz_Lsminfo), 1, fp);

  //m_channelColors.clear();
  initChannelColors();
  m_lsmChannelNames.clear();
  m_lsmTimeStamps.clear();
  m_lsmChannelDataTypes.clear();

  if (m_lsmInfo.u32OffsetChannelColors != 0) {
    fseek(fp, m_lsmInfo.u32OffsetChannelColors, SEEK_SET);
    fread(&m_lsmChannelInfo, sizeof(Lsm_Channel_Colors), 1, fp);

    char *chStruct = new char[m_lsmChannelInfo.s32BlockSize];
    fseek(fp, m_lsmInfo.u32OffsetChannelColors, SEEK_SET);
    fread(chStruct, m_lsmChannelInfo.s32BlockSize, 1, fp);
    std::vector<glm::col4> cls(m_lsmChannelInfo.s32NumberColors);
    memcpy(&(cls[0]), chStruct+m_lsmChannelInfo.s32ColorsOffset, sizeof(uint32_t)*cls.size());

    size_t offset = m_lsmChannelInfo.s32NamesOffset;
    int nameIdx = 0;
    while (nameIdx < m_lsmChannelInfo.s32NumberNames) {
      offset += 4;  // skip uint32_t name length
      QString str(chStruct+offset);
      m_lsmChannelNames.push_back(str);
      ++nameIdx;
      offset += str.size() + 1;
    }

    for (int ch=0; ch<m_lsmChannelInfo.s32NumberColors; ++ch) {
      QString chName;
      if (m_lsmChannelNames.size() > (size_t)ch)
        chName = m_lsmChannelNames[ch];
      if (!chName.isEmpty())
        m_channelColors[ch]->setName(chName);
      m_channelColors[ch]->set(glm::vec3(cls[ch])/255.f);
    }

    delete[] chStruct;
  }

  if (m_lsmInfo.u32OffsetTimeStamps != 0) {
    fseek(fp, m_lsmInfo.u32OffsetTimeStamps, SEEK_SET);
    fread(&m_lsmTimeStampInfo, sizeof(Lsm_Time_Stamp_Info), 1, fp);
    double *stamps = new double[m_lsmTimeStampInfo.s32NumberTimeStamps];
    fread(stamps, sizeof(double), m_lsmTimeStampInfo.s32NumberTimeStamps, fp);
    for (int i=0; i<m_lsmTimeStampInfo.s32NumberTimeStamps; ++i)
      m_lsmTimeStamps.push_back(stamps[i]);
    delete[] stamps;
  }

  if (m_lsmInfo.u32OffsetChannelDataTypes != 0) {
    fseek(fp, m_lsmInfo.u32OffsetChannelDataTypes, SEEK_SET);
    uint32_t *dataTypes = new uint32_t[m_lsmInfo.s32DimensionChannels];
    fread(dataTypes, sizeof(uint32_t), m_lsmInfo.s32DimensionChannels, fp);
    for (int i=0; i<m_lsmInfo.s32DimensionChannels; ++i)
      m_lsmChannelDataTypes.push_back(dataTypes[i]);
    delete[] dataTypes;
  }

  fclose(fp);

  // fill zresolution
  setResolution(m_lsmInfo.f64VoxelSizeX * 1e6,
                m_lsmInfo.f64VoxelSizeY * 1e6,
                m_lsmInfo.f64VoxelSizeZ * 1e6,
                'u');

  m_isLSMFile = true;
  return true;
}

void ZStack::logLSMInfo()
{
  if (!m_isLSMFile) {
    LINFO() << sourcePath() << "is not a valid LSM file.";
    return;
  }

  LINFO() << "Start LSM Info for" << sourcePath();
  LINFO() << "MagicNumber:" << hex << m_lsmInfo.u32MagicNumber;
  LINFO() << "DimensionX:" << m_lsmInfo.s32DimensionX;
  LINFO() << "DimensionY:" << m_lsmInfo.s32DimensionY;
  LINFO() << "DimensionZ:" << m_lsmInfo.s32DimensionZ;
  LINFO() << "DimensionChannels:" << m_lsmInfo.s32DimensionChannels;
  LINFO() << "DimensionTime:" << m_lsmInfo.s32DimensionTime;
  switch (m_lsmInfo.s32DataType) {
  case 1: LINFO() << "DataType:" << "8-bit unsigned integer"; break;
  case 2: LINFO() << "DataType:" << "12-bit unsigned integer"; break;
  case 5: LINFO() << "DataType:" << "32-bit float(for \"Time Series Mean-of-ROIs\")"; break;
  //case 0: LINFO() << "DataType:" << "different data types for different channels, see 32OffsetChannelDataTypes"; break;
  }
  for (size_t i=0; i<m_lsmChannelDataTypes.size(); ++i) {
    switch (m_lsmChannelDataTypes[i]) {
    case 1: LINFO() << "Channel" << i+1 << "DataType:" << "8-bit unsigned integer"; break;
    case 2: LINFO() << "Channel" << i+1 << "DataType:" << "12-bit unsigned integer"; break;
    case 5: LINFO() << "Channel" << i+1 << "DataType:" << "32-bit float(for \"Time Series Mean-of-ROIs\")"; break;
    }
  }
  LINFO() << "ThumbnailX:" << m_lsmInfo.s32ThumbnailX;
  LINFO() << "ThumbnailY:" << m_lsmInfo.s32ThumbnailY;
  LINFO() << "VoxelSizeX in meter:" << m_lsmInfo.f64VoxelSizeX;
  LINFO() << "VoxelSizeY in meter:" << m_lsmInfo.f64VoxelSizeY;
  LINFO() << "VoxelSizeZ in meter:" << m_lsmInfo.f64VoxelSizeZ;
  switch (m_lsmInfo.u16ScanType) {
  case 0: LINFO() << "ScanType:" << "normal x-y-z-scan"; break;
  case 1: LINFO() << "ScanType:" << "z-Scan (x-z-plane)"; break;
  case 2: LINFO() << "ScanType:" << "line scan"; break;
  case 3: LINFO() << "ScanType:" << "time series x-y"; break;
  case 4: LINFO() << "ScanType:" << "time series x-z (release 2.0 or later)"; break;
  case 5: LINFO() << "ScanType:" << "time series \"Mean of ROIs\" (release 2.0 or later)"; break;
  case 6: LINFO() << "ScanType:" << "time series x-y-z (release 2.3 or later)"; break;
  case 7: LINFO() << "ScanType:" << "spline scan (release 2.5 or later)"; break;
  case 8: LINFO() << "ScanType:" << "spline plane x-z (release 2.5 or later)"; break;
  case 9: LINFO() << "ScanType:" << "time series spline plane x-z (release 2.5 or later)"; break;
  case 10: LINFO() << "ScanType:" << "point mode (release 3.0 or later)"; break;
  }
  switch (m_lsmInfo.u16SpectralScan) {
  case 0: LINFO() << "SpectralScan:" << "no spectral scan"; break;
  case 1: LINFO() << "SpectralScan:" << "image has been acquired in spectral scan mode with a META detector (release 3.0 or later)"; break;
  }
  switch (m_lsmInfo.u32DataType) {
  case 0: LINFO() << "DataType:" << "Original scan data"; break;
  case 1: LINFO() << "DataType:" << "Calculated data"; break;
  case 2: LINFO() << "DataType:" << "Animation"; break;
  }
  if (m_lsmInfo.f64TimeInterval != 0) {
    LINFO() << "TimeInterval in s:" << m_lsmInfo.f64TimeInterval;
  }
  for (size_t i=0; i<m_lsmTimeStamps.size(); ++i) {
    LINFO() << "TimeStamp" << i+1 << "in s:" << m_lsmTimeStamps[i];
  }
  LINFO() << "DisplayAspectX:" << m_lsmInfo.f64DisplayAspectX;
  LINFO() << "DisplayAspectY:" << m_lsmInfo.f64DisplayAspectY;
  LINFO() << "DisplayAspectZ:" << m_lsmInfo.f64DisplayAspectZ;
  LINFO() << "DisplayAspectTime:" << m_lsmInfo.f64DisplayAspectTime;
  LINFO() << "ObjectiveSphereCorrection:" << m_lsmInfo.f64objectiveSphereCorrection;
  for (size_t i=0; i<m_channelColors.size(); ++i) {
    LINFO() << "Channel" << i+1 << "Name:" << m_lsmChannelNames[i] << "Color(RGB):" << m_channelColors[i]->get();
  }
  LINFO() << "End LSM Info for" << sourcePath();
}
#endif
