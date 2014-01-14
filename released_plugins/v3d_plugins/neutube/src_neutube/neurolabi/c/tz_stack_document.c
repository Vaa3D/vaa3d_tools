#include <string.h>
#include <utilities.h>
#include "tz_stack_document.h"
#include "tz_image_io.h"
#include "tz_file_list.h"

Stack_Document* New_Stack_Document()
{
  Stack_Document *doc = (Stack_Document*) 
    Guarded_Malloc(sizeof(Stack_Document), "New_Stack_Document");

  doc->type = STACK_DOC_UNDEFINED;
  doc->resolution[0] = 1.0;
  doc->resolution[1] = 1.0;
  doc->resolution[2] = 1.0;
  doc->unit = 'p';
  doc->offset[0] = 0.0;
  doc->offset[1] = 0.0;
  doc->offset[2] = 0.0;
  doc->channel = -1;
  doc->ci = NULL;

  return doc;
}

Stack_Document *Copy_Stack_Document(Stack_Document* src)
{
  if (src == NULL) {
    return NULL;
  }

  Stack_Document *dst = New_Stack_Document();
  *dst = *src;
  switch (dst->type) {
  case STACK_DOC_TIF_FILE:
  case STACK_DOC_LSM_FILE:
  case STACK_DOC_RAW_FILE:
  case STACK_DOC_SWC_FILE:
    {
      int n = strlen((char*) src->ci);
      dst->ci = malloc(sizeof(char) * (n + 1));
      memcpy(dst->ci, src->ci, sizeof(char) * (n + 1));
    }
    break;
  case STACK_DOC_FILE_BUNDLE:
    {
      dst->ci = Copy_File_Bundle_S((File_Bundle_S*)src->ci);
    }
    break;
  }
  return dst;
}

void Set_Stack_Document(Stack_Document *doc, int type, void const *ci)
{
  Clean_Stack_Document(doc);

  doc->type = type;

  switch (doc->type) {
  case STACK_DOC_TIF_FILE:
  case STACK_DOC_LSM_FILE:
  case STACK_DOC_RAW_FILE:
  case STACK_DOC_SWC_FILE:
    {
      int n = strlen((char*) ci);
      doc->ci = malloc(sizeof(char) * (n + 1));
      memcpy(doc->ci, ci, sizeof(char) * (n + 1));
    }
    break;
  case STACK_DOC_FILE_BUNDLE:
    {
      doc->ci = malloc(sizeof(File_Bundle_S));
      memcpy(doc->ci, ci, sizeof(File_Bundle_S));
    }
    break;
  case STACK_DOC_FILE_LIST:
    {
      doc->ci = (void*) ci;
    }
  }
}

void Clean_Stack_Document(Stack_Document *doc)
{
  if (doc->ci != NULL) {
    switch (doc->type) {
    case STACK_DOC_TIF_FILE:
    case STACK_DOC_LSM_FILE:
    case STACK_DOC_RAW_FILE:
      free(doc->ci);
      break;
    case STACK_DOC_FILE_BUNDLE:
      Clean_File_Bundle_S((File_Bundle_S*) doc->ci);
      free(doc->ci);
      break;
    default:
      break;
    }

    doc->ci = NULL;
  }

  doc->type = STACK_DOC_UNDEFINED;
}

void Delete_Stack_Document(Stack_Document *doc)
{
  free(doc);
}

void Kill_Stack_Document(Stack_Document *doc)
{
  Clean_Stack_Document(doc);
  Delete_Stack_Document(doc);
}

Stack* Import_Stack_Document(const Stack_Document *doc)
{
  Stack *stack = NULL;
  switch (doc->type) {
  case STACK_DOC_TIF_FILE:
    if (doc->channel == -1) {
      stack = Read_Stack((char*) doc->ci);
    } else {
      stack = Read_Sc_Stack((char*) doc->ci, doc->channel);
    }
    break;
  case STACK_DOC_LSM_FILE:
    stack = Read_Lsm_Stack((char*) doc->ci, doc->channel);
    break;
  case STACK_DOC_RAW_FILE:
    if (doc->channel == -1) {
      stack = Read_Raw_Stack((char*) doc->ci);
    } else {
      stack = Read_Raw_Stack_C((char*) doc->ci, doc->channel);
    }
    break;
  case STACK_DOC_FILE_BUNDLE:
    if (doc->channel == -1) {
      stack = Read_Stack_Planes_S((File_Bundle_S*) doc->ci);
    } else {
      stack = Read_Stack_Planes_Sc((File_Bundle_S*) doc->ci, doc->channel);
    }
    break;
  case STACK_DOC_FILE_LIST:
    {
      stack = Read_Image_List((File_List*) doc->ci);
    }
    break;
  default:
    break;
  }

  return stack;
}

char* Stack_Document_File_Path(const Stack_Document *doc)
{
  switch (doc->type) {
  case STACK_DOC_TIF_FILE:
  case STACK_DOC_RAW_FILE:
  case STACK_DOC_LSM_FILE:
    return (char*) doc->ci;
    break;
  case STACK_DOC_FILE_BUNDLE:
    return ((File_Bundle_S*) doc->ci)->prefix;
    break;
  case STACK_DOC_FILE_LIST:
    {
      File_List *list = (File_List*) doc->ci; 
      if (list->file_number == 0) {
        return NULL;
      }
      return list->file_path[0];
    }
    break;
  default:
    break;
  }  

  return NULL;
}
