/* tz_<1t>_chain.c
 *
 * 06-May-2008 Initial write: Ting Zhao
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#if defined(_WIN32) || defined(_WIN64)
  #define PCRE_STATIC
  #include <pcreposix.h>
#else
  #include <regex.h>
#endif
#include <string.h>
#ifndef _MSC_VER
#include <dirent.h>
#else
#include "tz_dirent.h"
#endif
#include <utilities.h>
#include "tz_constant.h"
#include "tz_error.h"
#include "tz_interface.h"
#include "tz_stack_sampling.h"
#include "tz_trace_utils.h"
#include "tz_darray.h"
#include "tz_cont_fun.h"
#include "tz_<1t>_chain_com.h"
#include "tz_vrml_material.h"
#include "tz_vrml_io.h"
#include "tz_geo3d_utils.h"
#include "tz_string.h"
#include "tz_geo3d_point_array.h"
#include "tz_stack_graph.h"
#include "tz_image_array.h"
#include "tz_swc_cell.h"
#include "tz_u8array.h"


<1T>_Chain* New_<1T>_Chain()
{
  <1T>_Chain *chain = 
    (<1T>_Chain *) Guarded_Malloc(sizeof(<1T>_Chain), "New_<1T>_Chain");

  chain->list = NULL;
  chain->iterator = NULL;

  return chain;
}

void Delete_<1T>_Chain(<1T>_Chain *chain)
{
  if (chain != NULL) {
    free(chain);
  }
}

void Construct_<1T>_Chain(<1T>_Chain *chain, <1T>_Node *p)
{
  chain->list = <1T>_Node_Dlist_New();
  <1T>_Node_Dlist_Set_Data(chain->list, p);
}

void Clean_<1T>_Chain(<1T>_Chain *chain)
{
  if (chain != NULL) {
    <1T>_Node_Dlist_Remove_All(chain->list);
    chain->list = NULL;
    chain->iterator = NULL;
  }
}

<1T>_Chain* Make_<1T>_Chain(<1T>_Node *p)
{
  <1T>_Chain *chain = New_<1T>_Chain();
  Construct_<1T>_Chain(chain, p);

  return chain;
}

void Kill_<1T>_Chain(<1T>_Chain *chain)
{
  Clean_<1T>_Chain(chain);
  Delete_<1T>_Chain(chain);
}

int <1T>_Chain_Length(<1T>_Chain *chain)
{
  if (chain == NULL) {
    return 0;
  }

  return <1T>_Node_Dlist_Length(chain->list, DL_BOTHDIR);
}

BOOL <1T>_Chain_Is_Empty(<1T>_Chain *chain)
{
  if (chain == NULL) {
    return TRUE;
  }

  if (chain->list == NULL) {
    return TRUE;
  }

  return FALSE;
}

<1T>_Node_Dlist* <1T>_Chain_Iterator_Start(<1T>_Chain *chain, 
					       Dlist_End_e d)
{
  <1T>_Node_Dlist *old_iter = chain->iterator;

  switch (d) {
  case DL_HEAD:
    chain->iterator = <1T>_Node_Dlist_Head(chain->list);
    break;
  case DL_TAIL:
    chain->iterator = <1T>_Node_Dlist_Tail(chain->list);
    break;
  default:
    TZ_ERROR(ERROR_DATA_VALUE);
    break;
  }

  return old_iter;
}

<1T>_Node_Dlist* <1T>_Chain_Iterator_Locate(<1T>_Chain *chain, int index)
{
  <1T>_Node_Dlist *old_iter = NULL;
  
  if (index >= 0) {
    old_iter = <1T>_Chain_Iterator_Start(chain, DL_HEAD);
  } else {
    old_iter = <1T>_Chain_Iterator_Start(chain, DL_TAIL);
  }

  chain->iterator = Locate_<1T>_Node_Dlist(chain->iterator, index);
  
  return old_iter;
}

<1T>_Node* <1T>_Chain_Next(<1T>_Chain *chain)
{
  if (chain == NULL) {
    return NULL;
  }

  if ((chain->list == NULL) || (chain->iterator == NULL)) {
    return NULL;
  }

  <1T>_Node *p = chain->iterator->data;

  chain->iterator = chain->iterator->next;

  return p;
}

<1T>_Node* <1T>_Chain_Prev(<1T>_Chain *chain)
{
  if (chain == NULL) {
    return NULL;
  }

  if ((chain->list == NULL) || (chain->iterator == NULL)) {
    return NULL;
  }

  <1T>_Node *p = chain->iterator->data;

  chain->iterator = chain->iterator->prev;

  return p;
}

<1T>_Node* <1T>_Chain_Next_D(<1T>_Chain *chain, Dlist_End_e d)
{
  if (d == DL_HEAD) {
    return <1T>_Chain_Next(chain);
  }

  if (d == DL_TAIL) {
    return <1T>_Chain_Prev(chain);
  }

  return NULL;
}

<1T>_Node* <1T>_Chain_Peek(const <1T>_Chain *chain)
{
  if (chain == NULL) {
    return NULL;
  }

  if ((chain->list == NULL) || (chain->iterator == NULL)) {
    return NULL;
  }

  return chain->iterator->data;
}

<2T>* <1T>_Chain_Peek_Seg(const <1T>_Chain *chain)
{
  <1T>_Node *p = <1T>_Chain_Peek(chain);
  if (p == NULL) {
    return NULL;
  }

  return p->locseg;
}

<1T>_Node* <1T>_Chain_Peek_Next(const <1T>_Chain *chain)
{
  if (chain == NULL) {
    return NULL;
  }

  if (chain->iterator == NULL) {
    return NULL;
  }

  if (chain->iterator->next == NULL) {
    return NULL;
  }

  return chain->iterator->next->data;
}

<1T>_Node* <1T>_Chain_Peek_Prev(const <1T>_Chain *chain)
{
  if (chain == NULL) {
    return NULL;
  }

  if (chain->iterator == NULL) {
    return NULL;
  }

  if (chain->iterator->prev == NULL) {
    return NULL;
  }

  return chain->iterator->prev->data;  
}

<2T>* <1T>_Chain_Next_Seg(<1T>_Chain *chain)
{
  <1T>_Node *p = <1T>_Chain_Next(chain);
  if (p == NULL) {
    return NULL;
  } else {
    return p->locseg;
  }
}

<2T>* <1T>_Chain_Prev_Seg(<1T>_Chain *chain)
{
  <1T>_Node *p = <1T>_Chain_Prev(chain);
  if (p == NULL) {
    return NULL;
  } else {
    return p->locseg;
  }
}

<2T>* <1T>_Chain_Next_Seg_D(<1T>_Chain *chain, Dlist_End_e d)
{
  if (d == DL_HEAD) {
    return <1T>_Chain_Next_Seg(chain);
  }

  if (d == DL_TAIL) {
    return <1T>_Chain_Prev_Seg(chain);
  }

  return NULL;
}

void Print_<1T>_Chain(<1T>_Chain *chain)
{
  <1T>_Node_Dlist* old_iter = <1T>_Chain_Iterator_Start(chain, DL_HEAD);
  <1T>_Node *p = NULL;
  int n = 0;
  while ((p = <1T>_Chain_Next(chain)) != NULL) {
    Print_<1T>_Node(p);
    n++;
  }

  printf("Total length: %d\n", n);

  chain->iterator = old_iter;
}

void Write_<1T>_Chain(const char *file_path, <1T>_Chain *chain)
{
  FILE *fp = Guarded_Fopen((char*) file_path, "wb", "Write_<1T>_Chain");
  
  <1T>_Node_Dlist *old_iter = <1T>_Chain_Iterator_Start(chain, DL_HEAD);
  <1T>_Node *p = NULL;
  
  double version = CHAIN_VERSION;
  if (version > 0.0) {
    double start = -version;
    fwrite(&start, sizeof(double), 1, fp);
  }

  while ((p = <1T>_Chain_Next(chain)) != NULL) {
    <1T>_Node_Fwrite_V(p, fp, version);
  }
  
  fclose(fp);

  chain->iterator = old_iter;
}

<1T>_Chain* Read_<1T>_Chain(const char *file_path)
{
  //FILE *fp = Guarded_Fopen((char*) file_path, "r", "Read_<1T>_Chain");
  FILE *fp = NULL;
  if ((fp = fopen(file_path, "rb")) == NULL) {
    return NULL;
  }
  
  <1T>_Chain *chain = New_<1T>_Chain();
  <1T>_Node *node = NULL;

  double start;
  double version = 0;
  fread(&start, sizeof(double), 1, fp);
  if (start >= 0) {
    fseek(fp, 0, SEEK_SET);
  } else {
    version = -start;
  }

  while ((node = <1T>_Node_Fread_V(NULL, fp, version)) != NULL) {
    <1T>_Chain_Add_Node(chain, node, DL_TAIL);
  }

  fclose(fp);

  return chain;
}

<1T>_Node* <1T>_Chain_Head(<1T>_Chain *chain)
{
  if (chain == NULL) {
    return NULL;
  }

  if (chain->list == NULL) {
    return NULL;
  }

  <1T>_Node_Dlist *tail = <1T>_Node_Dlist_Head(chain->list);

  return tail->data;
}

<1T>_Node* <1T>_Chain_Tail(<1T>_Chain *chain)
{
  if (chain == NULL) {
    return NULL;
  }

  if (chain->list == NULL) {
    return NULL;
  }

  <1T>_Node_Dlist *tail = <1T>_Node_Dlist_Tail(chain->list);

  return tail->data;
}

<2T>* <1T>_Chain_Head_Seg(<1T>_Chain *chain)
{
  <1T>_Node *p = <1T>_Chain_Head(chain);
  if (p == NULL) {
    return NULL;
  } else {
    return p->locseg;
  }
}

<2T>* <1T>_Chain_Tail_Seg(<1T>_Chain *chain)
{
  <1T>_Node *p = <1T>_Chain_Tail(chain);
  if (p == NULL) {
    return NULL;
  } else {
    return p->locseg;
  }
}

<1T>_Node* <1T>_Chain_Peek_At(<1T>_Chain *chain, int index)
{
  <1T>_Node_Dlist *old_iter = <1T>_Chain_Iterator_Locate(chain, index);
  <1T>_Node *node = <1T>_Chain_Peek(chain);

  chain->iterator = old_iter;
  
  return node;
}

<2T>* <1T>_Chain_Peek_Seg_At(<1T>_Chain *chain, int index)
{
  <1T>_Node_Dlist *old_iter = <1T>_Chain_Iterator_Locate(chain, index);
  <2T> *locseg = <1T>_Chain_Peek_Seg(chain);
  chain->iterator = old_iter;
  
  return locseg;
}

void <1T>_Chain_Add(<1T>_Chain *chain, <2T> *locseg, 
		      Trace_Record *tr, Dlist_End_e end)
{
  TZ_ASSERT(chain != NULL, "Cannot add node to NULL chain.");

  <1T>_Node *p = Make_<1T>_Node(locseg, tr);

  if (chain->list == NULL) {
    chain->list = <1T>_Node_Dlist_New();
    <1T>_Node_Dlist_Set_Data(chain->list, p);
  } else {
    chain->list = <1T>_Node_Dlist_Add(chain->list, p, end);
  }
}

void <1T>_Chain_Add_Node(<1T>_Chain *chain, <1T>_Node *p, Dlist_End_e end)
{
  TZ_ASSERT(chain != NULL, "Cannot add node to NULL chain.");
  if (chain->list == NULL) {
    chain->list = <1T>_Node_Dlist_New();
    <1T>_Node_Dlist_Set_Data(chain->list, p);
  } else {
    if (end == DL_HEAD) {
      chain->list = <1T>_Node_Dlist_Add(chain->list, p, end);
    } else {
      <1T>_Node_Dlist_Add(chain->list, p, end);
    }
  }
}

void <1T>_Chain_Cat(<1T>_Chain *chain1, const <1T>_Chain *chain2)
{
  if (chain2 == NULL) {
    return;
  }

  <1T>_Node_Dlist *head = <1T>_Node_Dlist_Head(chain2->list);
  if (head == NULL) {
    return;
  }

  if (chain1->list == NULL) {
    chain1->list = head;
  } else {
    <1T>_Node_Dlist *tail = <1T>_Node_Dlist_Tail(chain1->list);
    tail->next = head;
    head->prev = tail;
  }
}

void <1T>_Chain_Insert(<1T>_Chain *chain, <2T> *<1t>, 
		       Trace_Record *tr, int index)
{
  ASSERT(chain != NULL, "Cannot add node to NULL chain.");

  if (index < 0) {
    <1T>_Chain_Add(chain, <1t>, tr, DL_HEAD);
  } else {
    <1T>_Chain_Iterator_Locate(chain, index);
  
    if (chain->iterator == NULL) {
      <1T>_Chain_Add(chain, <1t>, tr, DL_TAIL);
    } else {
      <1T>_Node *p = Make_<1T>_Node(<1t>, tr);
      <1T>_Node_Dlist_Insert(chain->iterator, p);
    }
  }
}

void <1T>_Chain_Remove_End(<1T>_Chain *chain, Dlist_End_e end)
{
  if (<1T>_Chain_Is_Empty(chain) == FALSE) {
    chain->list = <1T>_Node_Dlist_Remove_End(chain->list, end);
  }
}

void <1T>_Chain_Remove_Current(<1T>_Chain *chain)
{
  if (<1T>_Chain_Is_Empty(chain) == FALSE) {
    if (chain->iterator != NULL) {
      <1T>_Node_Dlist *next = chain->iterator->next;
      chain->list = <1T>_Node_Dlist_Remove_Node(chain->iterator);
      chain->iterator = next;
    }
  }
}

<1T>_Node* <1T>_Chain_Take_Current(<1T>_Chain *chain)
{
  if (<1T>_Chain_Is_Empty(chain)) {
    return NULL;
  }
    
  if (chain->iterator == NULL) {
    return NULL;
  }  

  <1T>_Node *node = chain->iterator->data;
  chain->iterator->data = NULL;
  <1T>_Chain_Remove_Current(chain);

  return node;
}

<1T>_Chain* Dir_<1T>_Chain_N(const char *dir_name, const char *pattern,
				 int *n, int *file_num)
{
  *n = dir_fnum_p(dir_name, pattern);
  if (*n == 0) {
    return NULL;
  }

  <1T>_Chain *chain_array =
    (<1T>_Chain*) Guarded_Malloc(sizeof(<1T>_Chain) * (*n), 
				   "Dir_Neurochain");
  
  DIR *dir = opendir(dir_name);
  struct dirent *ent = readdir(dir);
  int index = 0;

  /* allocate a string for storing full path. 100 is just an initial guess */
  char *full_path = (char *) Guarded_Malloc(strlen(dir_name) + 100, 
					    "Dir_Neurochain");
  strcpy(full_path, dir_name);  
  strcat(full_path, "/");
	
  regex_t preg;
  regcomp(&preg, pattern, REG_BASIC);

  while (ent != NULL) {
    if (regexec(&preg, ent->d_name, 0, NULL, REG_BASIC) ==0) {
      int path_length = strlen(dir_name) + strlen(ent->d_name) + 1;
      if (path_length > strlen(full_path)) {
	full_path = (char *) Guarded_Realloc(full_path, path_length + 1, 
					     "Dir_Neurochain");
      }
      strcpy(full_path + strlen(dir_name) + 1, ent->d_name);
#ifdef _DEBUG_2
      printf("%d: %s\n", index, full_path);
#endif
      if (file_num != NULL) {
	file_num[index] = String_Last_Integer(ent->d_name);
      }

      <1T>_Chain *chain = Read_<1T>_Chain(full_path);
      chain_array[index++].list = chain->list;
      Delete_<1T>_Chain(chain);
    }
    ent = readdir(dir);
  }
  free(full_path);
  closedir(dir);

  regfree(&preg);

  return chain_array;
}

<1T>_Chain** Dir_<1T>_Chain_Nd(const char *dir_name, const char *pattern,
			       int *n, int *file_num)
{
  *n = dir_fnum_p(dir_name, pattern);
  if (*n == 0) {
    return NULL;
  }

  <1T>_Chain **chain_array =
    (<1T>_Chain**) Guarded_Malloc(sizeof(<1T>_Chain*) * (*n), 
				   "Dir_Neurochain");
  
  DIR *dir = opendir(dir_name);
  struct dirent *ent = readdir(dir);
  int index = 0;

  /* allocate a string for storing full path. 100 is just an initial guess */
  char *full_path = (char *) Guarded_Malloc(strlen(dir_name) + 100, 
					    "Dir_Neurochain");
  strcpy(full_path, dir_name);  
  strcat(full_path, "/");
	
  regex_t preg;
  regcomp(&preg, pattern, REG_BASIC);

  while (ent != NULL) {
    if (regexec(&preg, ent->d_name, 0, NULL, REG_BASIC) ==0) {
      int path_length = strlen(dir_name) + strlen(ent->d_name) + 1;
      if (path_length > strlen(full_path)) {
	full_path = (char *) Guarded_Realloc(full_path, path_length + 1, 
					     "Dir_Neurochain");
      }
      strcpy(full_path + strlen(dir_name) + 1, ent->d_name);
#ifdef _DEBUG_2
      printf("%d: %s\n", index, full_path);
#endif
      if (file_num != NULL) {
	file_num[index] = String_Last_Integer(ent->d_name);
      }

      chain_array[index++] = Read_<1T>_Chain(full_path);
    }
    ent = readdir(dir);
  }
  free(full_path);
  closedir(dir);

  regfree(&preg);

  return chain_array;
}

/*
void Neuron_Structure_Set_<1T>_Chain_Array(Neuron_Structure *ns,
					   <1T>_Chain *chain_array)
{
  int ncomp = Neuron_Structure_Component_Number(ns);
  int i;
  int type = <1T>_Chain_Component_Type();
  for (i = 0; i < ncomp; i++) {
    Neuron_Component *nc = Neuron_Structure_Get_Component(ns, i);
    Set_Neuron_Component(nc, type, chain_array + i);
  }
}
*/
