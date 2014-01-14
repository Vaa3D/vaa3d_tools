#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include "tz_error.h"

#define Error_Code errno 

const static char *Error_Msg[] = {
  "file open error",
  "write file error",
  "read file error",
  "file format error",
  "data type error",
  "incompatible data",
  "memory allocation error",
  "null pointer error",
  "data value error",
  "possible bug in the code",
  "unavaible function",
  "exception thrown",
  "unknown error",
  "assertion failed",
  "obsolete function",
  "function under development",
  "untested function",
  "tracing message",  
  "unidentified error"
};

/* tz_errmsg(): Generate error message.
 *
 * Args: error_type - type of the error
 *
 * Return: the error message. 
 */
const char* tz_errmsg(int error_type)
{
  if (error_type < USER_ERROR_START) {
    return strerror(error_type);
  }

  int index = sizeof(Error_Msg) / sizeof(char*);

  switch(error_type) {
  case ERROR_FREE:
    //do nothing
    return NULL;
  case ERROR_IO_OPEN:
    index--;
  case ERROR_IO_WRITE:
    index--;
  case ERROR_IO_READ:
    index--;
  case ERROR_IO_FORMAT:
    index--;
  case ERROR_DATA_TYPE:
    index--;
  case ERROR_DATA_COMPTB:
    index--;
  case ERROR_MALLOC:
    index--;
  case ERROR_POINTER_NULL:
    index--;
  case ERROR_DATA_VALUE:
    index--;
  case ERROR_CODE_BUG:
    index--;
  case ERROR_NA_FUNC:
    index--;
  case EXCEPTION_THROWN:
    index--;
  case ERROR_UNKNOWN:
    index--;
  case ERROR_ASSERT_FAILED:
    index--;
  case ERROR_OBSOLETE_FUNC:
    index--;
  case ERROR_PART_FUNC:
    index--;
  case ERROR_UNTESTED_FUNC:
    index--;
  case TRACE_FLAG:
    index--;
  default:
    index--;
  }

  return Error_Msg[index];
}

int Get_Error_Code()
{
  return Error_Code;
}

#ifdef _DEBUG_

#define Print_Exception(type, msg)				\
  {								\
    fprintf(stderr, "%s in function: %s\n", type, __func__);	\
    fprintf(stderr, "  %s:%d: %s\n", __FILE__, __LINE__, msg);  \
  }								\

void tz_error(int error_type, const char* file, int line, const char* func)
{
  fprintf(stderr, "%s in function: %s\n", "Error", func);
  fprintf(stderr, "  %s:%d: %s\n", file, line, tz_errmsg(error_type)); 
  abort();
}

void tz_warn(int error_type, const char* file, int line, const char* func)
{
  fprintf(stderr, "%s in function: %s\n", "Warning", func);
  fprintf(stderr, "  %s:%d: %s\n", file, line, tz_errmsg(error_type)); 
  Error_Code = error_type;
}

#else
    
void Print_Exception(const char *type, const char *msg)
{
  fprintf(stderr, "%s occured: %s\n", type, msg);  
}

void tz_error(int error_type)
{
  fprintf(stderr, "Program aborted because of an error: %s\n", 
	  tz_errmsg(error_type));
  abort();
}

void tz_warn(int error_type)
{
  fprintf(stderr, "Warning: %s\n", tz_errmsg(error_type));
  Error_Code = error_type;
}

#endif
