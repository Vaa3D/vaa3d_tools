/**@file tz_file_list.h
 * @author Ting Zhao
 * @date 16-Jul-2012
 */

#ifndef _TZ_FILE_LIST_H_
#define _TZ_FILE_LIST_H_

#include "tz_cdefs.h"
#include "tz_zobject.h"

__BEGIN_DECLS

#define FILE_LIST_MAX_PATH_LENGTH 500

typedef char File_Path_String_t[FILE_LIST_MAX_PATH_LENGTH];

typedef struct _File_List {
  int file_number;
  File_Path_String_t *file_path;
} File_List;

DECLARE_ZOBJECT_INTERFACE(File_List)

File_List* Make_File_List(int n);
File_List* Copy_File_List(File_List *list);

File_List* File_List_Load_Dir(const char *dir_path, const char *ext,
    File_List *list);
void File_List_Sort_By_Number(File_List *list);

__END_DECLS

#endif
