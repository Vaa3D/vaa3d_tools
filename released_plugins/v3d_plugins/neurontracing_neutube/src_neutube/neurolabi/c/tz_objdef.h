/**@file tz_objdef.h
 * @brief macros for object manangement
 * @author Ting Zhao
 * @date 05-Nov-2007
 */

#include <stdlib.h>
#include <string.h>
#include "tz_cdefs.h"

/*
 * The user must define <data>_asize(), Reset_<type>() and Make_<type> by 
 * his own.
 */

/* function and variable names */
#define _OBJECT(type) TZ_CONCAT(_, type)
#define FREE_OBJECT_LIST(type) TZ_CONCATU3(Free, type, List)
#define OBJECT_OFFSET(type) TZ_CONCATU(type, Offset)
#define OBJECT_INUSE(type) TZ_CONCATU(type, Inuse)
#define OBJECT_ASIZE(data, asize) TZ_CONCATU(data, asize) (data)
#define SET_OBJECT_ASIZE(type) TZ_CONCATU3(Set, type, Asize)
#define NEW_OBJECT(type) TZ_CONCATU(New, type)
#define RESET_OBJECT_FIELD(type) TZ_CONCATU(Default, type)
#define NEW_OBJECT_STATIC(data) TZ_CONCATU(new, data)
#define NEW_OBJECT_EMPTY_STATIC(data) TZ_CONCATU3(new, data, empty)
#define COPY_OBJECT_STATIC(data) TZ_CONCATU(copy, data)
#define COPY_OBJECT(type) TZ_CONCATU(Copy, type)
#define PACK_OBJECT_STATIC(data) TZ_CONCATU(pack, data)
#define PACK_OBJECT(type) TZ_CONCATU(Pack, type)
#define FREE_OBJECT(type) TZ_CONCATU(Free, type)
#define KILL_OBJECT(type) TZ_CONCATU(Kill, type)
#define RESET_OBJECT_STATIC(data) TZ_CONCATU(reset, data)
#define OBJECT_USAGE(type) TZ_CONCATU(type, Usage)
#define SAVE_OBJECT(type) TZ_CONCATU(Save, type) 
#define LOAD_OBJECT(type) TZ_CONCATU(Load, type)

/* routine string */
#define ROUTINE(fun) #fun

/* Management object structure definition */
#ifndef _MSC_VER
#define DECLARE_OBJECT(type, asize, data)	\
  typedef struct TZ_CONCAT(__,  type) {		\
    struct TZ_CONCAT(__, type) *next;		\
    size_t asize;					\
    type data;					\
  } _OBJECT(type);
#else
#define DECLARE_OBJECT(type, asize, data)	\
  struct _OBJECT(type) {		\
    TZ_CONCAT(_, type) *next;		\
    size_t asize;					\
    type data;					\
  };
#endif

/* Declare static variables */
#define DECLARE_STATIC(type)						\
  static TZ_CONCAT(_, type) * FREE_OBJECT_LIST(type) = NULL;		\
  static size_t OBJECT_OFFSET(type), OBJECT_INUSE(type);

#define OBJECT_HANDLE(type, object)		\
  ((_OBJECT(type) *) (((char*) object) - OBJECT_OFFSET(type)))

/* New_<type>() */
#define DEFINE_NEW_OBJECT(type, array, asize, data)			\
  type * NEW_OBJECT(type) ()						\
  {									\
    TZ_CONCAT(_, type) *object = (_OBJECT(type) *)			\
      Guarded_Malloc(sizeof(_OBJECT(type)), ROUTINE(NEW_OBJECT(type))); \
    OBJECT_OFFSET(type) = ((char *) &(object->data)) - ((char *) object); \
    object->asize = 0;							\
    object->data.array = NULL;						\
    RESET_OBJECT_FIELD(type)(&(object->data));				\
    OBJECT_INUSE(type) += 1;						\
    return &(object->data);						\
  }

/* Set_<type>_Asize() */
#ifdef _MSC_VER
#define DEFINE_SET_OBJECT_ASIZE(type, array, asize, data)		\
  void SET_OBJECT_ASIZE(type) (type *data, size_t new_asize)		\
  {									\
    if (data->array != NULL) {						\
      return;								\
    }									\
    _OBJECT(type) *object = (_OBJECT(type) *) (((char *) data) -        \
					       OBJECT_OFFSET(type));	\
    object->asize = new_asize;						\
    data->array = (decltype(data->array))Guarded_Malloc(object->asize,				\
				 ROUTINE(SET_OBJECT_ASIZE(type)));	\
  }
#else
#define DEFINE_SET_OBJECT_ASIZE(type, array, asize, data)		\
  void SET_OBJECT_ASIZE(type) (type *data, size_t new_asize)		\
  {									\
    if (data->array != NULL) {						\
      return;								\
    }									\
    _OBJECT(type) *object = (_OBJECT(type) *) (((char *) data) -        \
					       OBJECT_OFFSET(type));	\
    object->asize = new_asize;						\
    data->array = Guarded_Malloc(object->asize,				\
				 ROUTINE(SET_OBJECT_ASIZE(type)));	\
  }
#endif

/* Define static function of newing an object : new_<data>() */
#ifdef _MSC_VER
#define DECLARE_NEW_OBJECT_STATIC(type, array, asize, data)		\
  static inline type * NEW_OBJECT_STATIC(data) (size_t asize, char *routine) \
  {									\
    TZ_CONCAT(_, type) *object;						\
    if (FREE_OBJECT_LIST(type) == NULL) {				\
      object = (_OBJECT(type) *)					\
	Guarded_Malloc(sizeof(_OBJECT(type)), routine);			\
      OBJECT_OFFSET(type) = ((char *) &(object->data)) - ((char *) object); \
      object->asize = asize;						\
      object->data.array = (decltype(object->data.array))Guarded_Malloc(asize, routine);		\
      OBJECT_INUSE(type) += 1;						\
    } else {								\
      object = FREE_OBJECT_LIST(type);					\
      FREE_OBJECT_LIST(type) = object->next;				\
      if (object->asize < asize) {					\
	object->asize = asize;						\
	object->data.array = (decltype(object->data.array))Guarded_Realloc(object->data.array,	\
					     asize, routine);		\
      }									\
      OBJECT_INUSE(type) += 1;						\
    }									\
    return (&(object->data));						\
  }
#else
#define DECLARE_NEW_OBJECT_STATIC(type, array, asize, data)		\
  static inline type * NEW_OBJECT_STATIC(data) (size_t asize, char *routine) \
  {									\
    TZ_CONCAT(_, type) *object;						\
    if (FREE_OBJECT_LIST(type) == NULL) {				\
      object = (_OBJECT(type) *)					\
	Guarded_Malloc(sizeof(_OBJECT(type)), routine);			\
      OBJECT_OFFSET(type) = ((char *) &(object->data)) - ((char *) object); \
      object->asize = asize;						\
      object->data.array = Guarded_Malloc(asize, routine);		\
      OBJECT_INUSE(type) += 1;						\
    } else {								\
      object = FREE_OBJECT_LIST(type);					\
      FREE_OBJECT_LIST(type) = object->next;				\
      if (object->asize < asize) {					\
	object->asize = asize;						\
	object->data.array = Guarded_Realloc(object->data.array,	\
					     asize, routine);		\
      }									\
      OBJECT_INUSE(type) += 1;						\
    }									\
    return (&(object->data));						\
  }
#endif

/* Define static function of newing an empty object : new_<data>_empty() */
#define DECLARE_NEW_OBJECT_EMPTY_STATIC(type, array, asize, data)		\
  static inline type * NEW_OBJECT_EMPTY_STATIC(data) (size_t asize, char *routine) \
  {									\
    TZ_CONCAT(_, type) *object;						\
    object = (_OBJECT(type) *)					\
      Guarded_Malloc(sizeof(_OBJECT(type)), routine);			\
    OBJECT_OFFSET(type) = ((char *) &(object->data)) - ((char *) object); \
    object->asize = asize;						\
    OBJECT_INUSE(type) += 1;						\
    return (&(object->data));						\
  }

/* Define static function of copying a function : copy_<data>() */
#define DECLARE_COPY_OBJECT(type, array, asize, data)			\
  static inline type * COPY_OBJECT_STATIC(data) (const type *data)	\
  {									\
    type *copy = NEW_OBJECT_STATIC(data) (OBJECT_ASIZE(data, asize),	\
					  ROUTINE(COPY_OBJECT(type)));	\
    type temp = *copy;							\
    *copy = *data;							\
    copy->array = temp.array;						\
    memcpy(copy->array, data->array, OBJECT_ASIZE(data, asize));	\
    return (copy);							\
  }

/* Define the function of copying an object : Copy_<type>() */
#define DEFINE_COPY_OBJECT(type, data)		\
  type * COPY_OBJECT(type) (const type *data)	\
  { return (COPY_OBJECT_STATIC(data) (data)); }

/* Define the static function of packing an object */
#ifdef _MSC_VER
#define DECLARE_PACK_OBJECT(type, array, asize, data)			\
  static inline void PACK_OBJECT_STATIC(data) (type *data)		\
  {									\
    _OBJECT(type) *object = (_OBJECT(type) *) (((char *) data) -	\
					       OBJECT_OFFSET(type));	\
    if (object->asize != OBJECT_ASIZE(data, asize)) {			\
      object->asize = OBJECT_ASIZE(data, asize);			\
      object->data.array = (decltype(object->data.array))Guarded_Realloc(object->data.array,		\
					   object->asize,		\
					   ROUTINE(PACK_OBJECT(type))); \
    }									\
  }
#else
#define DECLARE_PACK_OBJECT(type, array, asize, data)			\
  static inline void PACK_OBJECT_STATIC(data) (type *data)		\
  {									\
    _OBJECT(type) *object = (_OBJECT(type) *) (((char *) data) -	\
					       OBJECT_OFFSET(type));	\
    if (object->asize != OBJECT_ASIZE(data, asize)) {			\
      object->asize = OBJECT_ASIZE(data, asize);			\
      object->data.array = Guarded_Realloc(object->data.array,		\
					   object->asize,		\
					   ROUTINE(PACK_OBJECT(type))); \
    }									\
  }
#endif

/* Define the function of packing an object */
#define DEFINE_PACK_OBJECT(type, array, asize, data) \
  void PACK_OBJECT(type) (type *data)		     \
  { PACK_OBJECT_STATIC(data) (data); }		     

/* Define the function of freeing an object */ 
#define DEFINE_FREE_OBJECT(type, array, data)				\
  void FREE_OBJECT(type) (type *data)					\
  {									\
    _OBJECT(type) *object  = (_OBJECT(type) *)				\
      (((char *) data) - OBJECT_OFFSET(type));				\
    object->next = FREE_OBJECT_LIST(type);				\
    FREE_OBJECT_LIST(type) = object;					\
    OBJECT_INUSE(type) -= 1;						\
  }

/* Define the function of killing an object */ 
#define DEFINE_KILL_OBJECT(type, array, data)				\
  void KILL_OBJECT(type) (type *data)					\
  {									\
    free(data->array);							\
    free(((char *) data) - OBJECT_OFFSET(type));			\
    OBJECT_INUSE(type) -= 1;						\
  }

#define DECLARE_RESET_OBJECT(type, array, data)			\
  static inline void RESET_OBJECT_STATIC(data) (type *data)	\
  {								\
    _OBJECT(type) *object;					\
    while (FREE_OBJECT_LIST(type) != NULL) {			\
      object = FREE_OBJECT_LIST(type);				\
      FREE_OBJECT_LIST(type) = object->next;			\
      KILL_OBJECT(type) (&(object->data));			\
    }								\
  }

#define DEFINE_OBJECT_USAGE(type)		\
  int OBJECT_USAGE(type) ()			\
  { return (OBJECT_INUSE(type)); }

/* Define the function of saving an object */ 
#define DEFINE_SAVE_OBJECT(type, array, asize, data)		\
  int SAVE_OBJECT(type) (type *data, char *file_path)		\
  {								\
    FILE *fp = fopen(file_path, "w+");				\
    if (fp == NULL) {						\
      return 1;							\
    }								\
    size_t array_size = OBJECT_ASIZE(data, asize);		\
    size_t info_size = sizeof(*data) - sizeof(data->array);	\
								\
    fwrite(&array_size, sizeof(size_t), 1, fp);			\
    fwrite(data, info_size, 1, fp);				\
    fwrite(data->array, array_size, 1, fp);			\
								\
    fclose(fp);							\
    return 0;							\
  }							

/* Define the function of loading an object */ 
#define DEFINE_LOAD_OBJECT(type, array, data)				\
  type* LOAD_OBJECT(type) (char *file_path)				\
  {									\
    FILE *fp = fopen(file_path, "r");					\
    if (fp == NULL) {							\
      return NULL;							\
    }									\
									\
    size_t array_size = 0;						\
    fread(&array_size, sizeof(size_t), 1, fp);				\
    type *data = NEW_OBJECT_STATIC(data)				\
      (array_size, ROUTINE(LOAD_OBJECT(type)));				\
    size_t info_size = sizeof(*data) - sizeof(data->array);		\
    fread(data, info_size, 1, fp);					\
    fread(data->array, array_size, 1, fp);				\
									\
    fclose(fp);								\
									\
    return data;							\
  }

/* type - object structure type name
 * array - array name in the object
 * asize - decorate array size function
 * data - object argument name, which is also used to decorate functions
 */
#define DEFINE_OBJECT_MANAGEMENT(type, array, asize, data)	\
  DECLARE_OBJECT(type, asize, data)				\
  DECLARE_STATIC(type)						\
  DECLARE_NEW_OBJECT_STATIC(type, array, asize, data)		\
  DECLARE_NEW_OBJECT_EMPTY_STATIC(type, array, asize, data)		\
  DECLARE_COPY_OBJECT(type, array, asize, data)			\
  DEFINE_COPY_OBJECT(type, data)				\
  DECLARE_PACK_OBJECT(type, array, asize, data)			\
  DEFINE_PACK_OBJECT(type, array, asize, data)			\
  DEFINE_FREE_OBJECT(type, array, data)				\
  DEFINE_KILL_OBJECT(type, array, data)				\
  DECLARE_RESET_OBJECT(type, array, data)			\
  DEFINE_OBJECT_USAGE(type)					\
  DEFINE_SAVE_OBJECT(type, array, asize, data)			\
  DEFINE_LOAD_OBJECT(type, array, data)				\
  DEFINE_NEW_OBJECT(type, array, asize, data)			\
  DEFINE_SET_OBJECT_ASIZE(type, array, asize, data)
