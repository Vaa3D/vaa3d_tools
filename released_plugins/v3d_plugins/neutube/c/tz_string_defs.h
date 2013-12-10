/**@file tz_string_defs.h
 * @brief definitions for string
 * @author Ting Zhao
 * @date 13-Dec-2008
 */

#ifndef _TZ_STRING_DEFS_H_
#define _TZ_STRING_DEFS_H_

#include "tz_cdefs.h"

__BEGIN_DECLS

#define MAX_LINE_DIM 1000 /* maximum length of a line can be */
#define MAX_WORD_LENGTH 100 /* maximum length of a word can be. '\0' is 
			     * counted as a character in the word. */
#define MAX_STRING_LENGTH 10000

/**@brief Workspace for line processing.
 *
 */
#ifdef _MSC_VER
typedef struct __String_Workspace {
#else
typedef struct _String_Workspace {
#endif
  char *array;
  char *iterator;
  int size;
} String_Workspace;

void Default_String_Workspace(String_Workspace *sw);

void Construct_String_Workspace(String_Workspace * sw, int size);
void Clean_String_Workspace(String_Workspace * sw);

String_Workspace* Make_String_Workspace();

/* Functions defined in the macros */
String_Workspace* New_String_Workspace();
String_Workspace *Copy_String_Workspace(const String_Workspace *sw);
void   Pack_String_Workspace(String_Workspace *sw);
void   Free_String_Workspace(String_Workspace *sw);
void   Kill_String_Workspace(String_Workspace *sw);
void   Reset_String_Workspace();
int    String_Workspace_Usage();

int Save_String_Workspace(String_Workspace *sw, char *file_path);
String_Workspace* Load_String_Workspace(char *file_path);
/***********************************/

void String_Workspace_Realloc(String_Workspace *sw, int size);
void String_Workspace_Grow(String_Workspace *sw);



__END_DECLS

#endif
