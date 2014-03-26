#include "zstackdocument.h"
#include "zstack.hxx"
#include "tz_image_io.h"
#include "c_stack.h"

ZStackDocument::ZStackDocument()
{
  m_doc = NULL;
}

ZStackDocument::~ZStackDocument()
{
  if (m_doc != NULL) {
    Kill_Stack_Document(m_doc);
  }
}

File_Bundle_S ZStackDocument::toFileBundleS()
{
  File_Bundle_S fs;
  fs.prefix = const_cast<char*>(m_prefix.c_str());
  fs.suffix = const_cast<char*>(m_suffix.c_str());
  fs.first_num = m_firstNum;
  fs.last_num = m_lastNum;
  fs.num_width = m_numWidth;
}

ZStack* ZStackDocument::readData()
{
  ZStack *data = NULL;

  if (!m_urlList.empty()) {
    switch (m_type) {
    case SINGLE_FILE:
      data->load(m_urlList[0]);
      break;
    case FILE_BUNDLE:
    {
      File_Bundle_S fs = toFileBundleS();
      Stack *stack = Read_Stack_Planes_S(&fs);
      data->load(stack, true, NULL, false, NULL, false);
    }
      break;
    case FILE_LIST:
    {
      Stack *stack = Make_Stack(kind(), width(), height(), depth());
      for (size_t i = 0; i < m_urlList.size(); i++) {
        Stack *slice = Read_Stack(m_urlList[i]);
        C_Stack::copyPlaneValue(stack, slice->array, i);
        Free_Stack(slice);
      }
      Reset_Stack();
      data->load(stack, true, NULL, false, NULL, false);
    }
      break;
    case IMAGE_SERIES:
    {
      data->loadImageSequence(m_urlList[0].c_str());
    }
      break;
    }
  }

  return data;
}
