/**@file tz_zobject.h
 * @author Ting Zhao
 * @date 16-Nov-2009
 */

#ifndef _TZ_ZOBJECT_H_
#define _TZ_ZOBJECT_H_

#include "tz_cdefs.h"

__BEGIN_DECLS

/* Object interface declarations and definitions
 *
 * New_ZObject $
 * Default_ZObject
 * Clean_ZOject
 * Delete_ZObject $
 * Kill_ZObject $
 * Print_ZObject
 *
 * $: these routines are defined in DEFINE_ZOBJECT_INTERFACE. All other routines
 *    require user definition.
 */

#define NEW_ZOBJECT(ZObject) TZ_CONCATU(New, ZObject)
#define DEFAULT_ZOBJECT(ZObject) TZ_CONCATU(Default, ZObject)
#define CLEAN_ZOBJECT(ZObject) TZ_CONCATU(Clean, ZObject)
#define DELETE_ZOBJECT(ZObject) TZ_CONCATU(Delete, ZObject)
#define KILL_ZOBJECT(ZObject) TZ_CONCATU(Kill, ZObject)
#define PRINT_ZOBJECT(ZObject) TZ_CONCATU(Print, ZObject)

#define DECLARE_ZOBJECT_INTERFACE(ZObject)				\
  ZObject* NEW_ZOBJECT(ZObject) ();					\
  void DEFAULT_ZOBJECT(ZObject)(ZObject *obj); \
  void CLEAN_ZOBJECT(ZObject)(ZObject *obj); \
  void DELETE_ZOBJECT(ZObject)(ZObject *obj); \
  void KILL_ZOBJECT(ZObject)(ZObject *obj);\
  void PRINT_ZOBJECT(ZObject) (ZObject *obj);

#define DEFINE_NEW_ZOBJECT(ZObject)					\
  ZObject* NEW_ZOBJECT(ZObject)() {					\
    ZObject *obj = (ZObject*) Guarded_Malloc(sizeof(ZObject), (char*)__func__);\
    DEFAULT_ZOBJECT(ZObject)(obj);					\
    return obj;								\
  }

#define DEFINE_DELETE_ZOBJECT(ZObject)		\
  void DELETE_ZOBJECT(ZObject)(ZObject *obj) {	\
    if (obj != NULL) {				\
      free(obj);				\
    }						\
  }

#define DEFINE_KILL_ZOBJECT(ZObject)		\
  void KILL_ZOBJECT(ZObject)(ZObject *obj) {	\
    if (obj != NULL) {	\
      CLEAN_ZOBJECT(ZObject)(obj);		\
      DELETE_ZOBJECT(ZObject)(obj);		\
    }						\
  }

#define DEFINE_ZOBJECT_INTERFACE(ZObject)		\
  DEFINE_NEW_ZOBJECT(ZObject)				\
  DEFINE_DELETE_ZOBJECT(ZObject)			\
  DEFINE_KILL_ZOBJECT(ZObject)


__END_DECLS

#endif
