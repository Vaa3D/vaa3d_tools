/**@file tz_stack_document.h
 * @author Ting Zhao
 * @date 10-May-2009
 */

#ifndef _TZ_STACK_DOCUMENT_H_
#define _TZ_STACK_DOCUMENT_H_

#include "tz_cdefs.h"
#include "tz_image_lib_defs.h"

__BEGIN_DECLS

enum {
  STACK_DOC_TIF_FILE, STACK_DOC_LSM_FILE, STACK_DOC_FILE_BUNDLE, 
  STACK_DOC_RAW_FILE, STACK_DOC_SWC_FILE, STACK_DOC_FILE_LIST,
  STACK_DOC_UNDEFINED
};

typedef struct _Stack_Document {
  int type;
  double resolution[3];
  char unit; /* 'p' - pixel; 'u' - um */
  double offset[3];
  int channel;
  void *ci;
} Stack_Document;

Stack_Document *New_Stack_Document();
Stack_Document *Copy_Stack_Document(Stack_Document* src);
void Set_Stack_Document(Stack_Document *doc, int type, void const *ci);

void Clean_Stack_Document(Stack_Document *doc);
void Delete_Stack_Document(Stack_Document *doc);
void Kill_Stack_Document(Stack_Document *doc);

Stack* Import_Stack_Document(const Stack_Document *doc);

char* Stack_Document_File_Path(const Stack_Document *doc);

__END_DECLS

#endif
