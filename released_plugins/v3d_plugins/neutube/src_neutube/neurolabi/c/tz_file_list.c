#include <stdlib.h>
#ifndef _MSC_VER
#include <dirent.h>
#else
#include "tz_dirent.h"
#endif
#include <string.h>
#include <errno.h>
#include "tz_file_list.h"
#include "tz_utilities.h"
#include "tz_iarray.h"
#include "tz_string.h"


DEFINE_ZOBJECT_INTERFACE(File_List)

void Default_File_List(File_List *list)
{
  list->file_number = 0;
  list->file_path = NULL;
}

void Clean_File_List(File_List *list)
{
  free(list->file_path);
}

void Print_File_List(File_List *list)
{
  printf("%d files\n", list->file_number);

  int i;
  for (i = 0; i < list->file_number; i++) {
    printf("%s\n", list->file_path[i]);
  }
}

File_List* Make_File_List(int n)
{
  File_List *list = New_File_List();

  list->file_number = n;
  if (n > 0) {
    list->file_path = (File_Path_String_t*)Guarded_Malloc(sizeof(File_Path_String_t) * n,
        "Make_File_List");
  }

  return list;
}

File_List* Copy_File_List(File_List *list)
{
  File_List *newlist = NULL;
  
  if (list != NULL) {
    newlist = Make_File_List(list->file_number);
    int i;
    for (i = 0; i < list->file_number; i++) {
      strcpy(newlist->file_path[i], list->file_path[i]);
    }
  }

  return newlist;
}

File_List* File_List_Load_Dir(const char *dir_path, const char *ext,
    File_List *list)
{
  int nfile = dir_fnum(dir_path, ext);

  if (list == NULL) {
    list = Make_File_List(nfile);
  } else {
    Clean_File_List(list);
    list->file_number = nfile;
    if (nfile > 0) {
      list->file_path = (File_Path_String_t*)Guarded_Malloc(sizeof(File_Path_String_t) * nfile,
          "File_List_Load_Dir");
    }
  }

  if (nfile > 0) {
    DIR *dir = opendir(dir_path);
    if (dir == NULL) {
      perror(strerror(errno));
      return list;
    }

    struct dirent *ent = readdir(dir);
    int index = 0;
    while (ent != NULL) {
      const char *file_ext = fextn(ent->d_name);
      if (file_ext != NULL) {
        if (strcmp(file_ext, ext) ==0) {
          sprintf(list->file_path[index], "%s/%s", dir_path, ent->d_name);
          index++;
        }
      }
      ent = readdir(dir);
    }
    closedir(dir);
  }

  return list;
}

void File_List_Sort_By_Number(File_List *list)
{
  if (list != NULL) {
    if (list->file_number > 1) { 
      int *file_number = iarray_calloc(list->file_number);
      int *index = iarray_calloc(list->file_number);
      int i;
      for (i = 0; i < list->file_number; i++) {
        file_number[i] = String_Last_Integer(list->file_path[i]);
      }
      iarray_qsort(file_number, index, list->file_number);
      File_List *tmplist = Copy_File_List(list);
      
      for (i = 0; i < list->file_number; i++) {
        strcpy(list->file_path[i], tmplist->file_path[index[i]]);
      }
      Kill_File_List(tmplist);
    }
  }
}
