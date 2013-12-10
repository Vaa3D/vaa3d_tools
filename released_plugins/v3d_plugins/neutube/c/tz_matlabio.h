/**@file tz_matlabio.h
 * @brief I/O for Matlab
 * @author Ting Zhao
 */

#ifndef _MATLABIO_H_
#define _MATLABIO_H_

#include "tz_cdefs.h"
#include "tz_utilities.h"
#include "tz_zobject.h"

__BEGIN_DECLS

/**@brief A structure for Matlab array.
  */
typedef struct _Matlab_Array {
  tz_uint8 type; /* type of the array (Class) */
  tz_uint8 flag; /* set it to 0 for local non-logical real data */
  tz_uint32 ndim;
  tz_uint32 dim[5];
  char name[128]; /* name of the array */
  void* array;
} Matlab_Array;

DECLARE_ZOBJECT_INTERFACE(Matlab_Array)

int parse_header(void* stream,char* description,tz_uint64* subsys_offset,tz_uint16* version,tz_uint16* endian);
int parse_tag(void* stream,tz_uint32* data_type,tz_uint32* data_size);
int parse_element(void* stream,tz_uint32* data_type,tz_uint32* data_size,tz_uint32* data_start);
int parse_arrayflags(void* stream,tz_uint8* flags,tz_uint8* data_class,tz_uint32* offset);
int parse_dim(void* stream,tz_uint32* ndim,tz_uint32* dim,tz_uint32* offset);
int parse_varname(void* stream,char* name,tz_uint32* offset);
void* uncompress_data(void* stream,tz_uint32 length,tz_uint32 *length2);
int parse_data(void* stream,Matlab_Array* mr,int has_tag);
tz_uint32 type_size(tz_uint32 type);
tz_uint32 norm_size(tz_uint32 size);
void print_mrinfo(Matlab_Array* mr);

/* 
 * mr_read() reads a variable in the matlab file <filename> into <mr>. It only
 * reads the first variable and can not deal with the situation when a double
 * variable is saved in another type.
 */
int mr_read(const char *filename, Matlab_Array* mr);
//int mr_write(const char *filename, Matlab_Array* mr);

void Find_Matlab_Binpath(char *path);

Matlab_Array* Read_Matlab_Array(const char *filepath);

__END_DECLS

#endif
