/**@file tz_error.h
 * @brief error handling
 * @author Ting Zhao
 * @date 10-Oct-2007
 */

#ifndef _TZ_ERROR_H_
#define _TZ_ERROR_H_

#include <stdio.h>

#include "tz_cdefs.h"
#ifdef ENABLE_C_EXCEPTION
#include "tz_exception.h"
#endif

__BEGIN_DECLS

/**@addtogroup error_ Error handling (tz_error.h)
 * @{
 */

#define ERROR_FREE    0               /* no error                  */

static const int SUCCESS = ERROR_FREE;

#define USER_ERROR_START 1024

/* To add and error, one needs to add a string in Error_Msg and a case in
 * tz_errmsg(), both at the same relative position. Then increase 
 * ERROR_MSG_NUMBER by 1. */
enum {
  ERROR_IO_OPEN = USER_ERROR_START,   /**< file open error           */
  ERROR_IO_WRITE,                     /**< file write error          */
  ERROR_IO_READ,                      /**< file read error           */
  ERROR_IO_FORMAT,                    /**< file format error         */
  ERROR_DATA_TYPE,                    /**< data type error           */
  ERROR_DATA_COMPTB,                  /**< data compatiblitity error */
  ERROR_MALLOC,                       /**< memory allocation error   */
  ERROR_POINTER_NULL,                 /**< null pointer              */
  ERROR_DATA_VALUE,                   /**< data value error          */
  ERROR_CODE_BUG,                     /**< coding bug                */
  ERROR_NA_FUNC,                      /**< function not available    */
  EXCEPTION_THROWN,                   /**< an exception is thrown    */
  ERROR_UNKNOWN,                      /**< unknown error             */
  ERROR_ASSERT_FAILED,                /**< assert failed             */ 
  ERROR_OBSOLETE_FUNC,                /**< obsolete function         */
  ERROR_PART_FUNC,                    /**< function under developing */
  ERROR_UNTESTED_FUNC,                /**< untested function */
  TRACE_FLAG,                         /**< tracing message           */
  ERROR_OTHER                         /**< other errors              */
};

/*
 * tz_errmsg() returns a string of the error messages of the error code 
 * <error_type>.
 */
const char* tz_errmsg(int error_type);

/*
 * Get_Error_Code() returns the current error code.
 */
int Get_Error_Code();

#ifdef _DEBUG_ /* debug version */

void tz_error(int error_type, const char* file, int line, const char* func);
void tz_warn(int error_type, const char* file, int line, const char* func);

# define TRACE(msg)				\
  TZ_WARN(TRACE_FLAG);				\
  printf("%s\n",msg)

# define TZ_ERROR(error_type)				\
  tz_error(error_type, __FILE__, __LINE__, __func__)

# define TZ_WARN(error_type)				\
  tz_warn(error_type, __FILE__, __LINE__, __func__)

/*
 * PRINT_EXCEPTION() prints an exception. <type> is the name of the type of 
 * the exception and <msg> is the detailed message. 
 */
#define PRINT_EXCEPTION(type, msg)				\
  {								\
    if (type != NULL) {               				\
    fprintf(stderr, "%s in function: %s\n", type, __func__);	\
    } else {							\
      fprintf(stderr, "Error in function: %s\n", __func__);	\
    }								\
    fprintf(stderr, "  %s:%d: %s\n", __FILE__, __LINE__, msg);  \
  }

#define ASSERT TZ_ASSERT /* obsolete interface */
#define TZ_ASSERT(condition, msg)					\
  {									\
    if (!(condition)) {							\
      fprintf(stderr,"Assertion failed in the function %s.\n",		\
	      __func__);						\
      fprintf(stderr, "  %s:%d: %s\n", __FILE__, __LINE__, msg);	\
      TZ_ERROR(ERROR_ASSERT_FAILED);					\
    }									\
  }

#define TZ_PRIOR_ASSERT(prior, condition, msg)				\
  {									\
    if (prior) {							\
      if (!(condition)) {						\
	fprintf(stderr,"Assertion failed in the function %s.\n",	\
		__func__);						\
	fprintf(stderr, "  %s:%d: %s\n", __FILE__, __LINE__, msg);	\
	TZ_ERROR(ERROR_ASSERT_FAILED);					\
      }									\
    }									\
  }

#else /* release version */

void tz_error(int error_type);
void tz_warn(int error_type);

# define TRACE(msg) /* empty */

# define TZ_ERROR(error_type) tz_error(error_type)

# define TZ_WARN(error_type) tz_warn(error_type)

    
#define PRINT_EXCEPTION(type, msg)				\
  {								\
    if (type == NULL) {                                         \
      fprintf(stderr, "Exception: %s\n", msg);                  \
    } else {                                                    \
      fprintf(stderr, "%s: %s\n", type, msg);			\
    }                                                           \
  }

#undef assert
#define assert(e) ((void)0)

#define ASSERT TZ_ASSERT /* obsolete interface */
#define TZ_ASSERT(condition, msg) ((void)0)
#define TZ_PRIOR_ASSERT(prior, condition, msg) ((void)0)

#endif /* _DEBUG_ */

#ifdef ENABLE_C_EXCEPTION
/* exception handling */

#define THROW(e)				\
  TZ_WARN(EXCEPTION_THROWN);			\
  Throw e

#  define INIT_EXCEPTION						\
  define_exception_type(int);						\
  extern struct exception_context the_exception_context[1];

#  define INIT_EXCEPTION_MAIN(e)			\
  define_exception_type(int);				\
  struct exception_context the_exception_context[1];	\
  int e;
#else
#  define THROW(e) TZ_WARN(EXCEPTION_THROWN)
#  define INIT_EXCEPTION
#  define INIT_EXCEPTION_MAIN(e)
#endif
/**********************/

#define ASSERT_NOT_NULL(p) ASSERT((p) != NULL, "Null pointer")

/**@}*/

__END_DECLS

#endif
