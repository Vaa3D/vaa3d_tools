#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "tz_error.h"
#include "tz_array.h"

#define MIN_DOUBLE 1e-10


void array_error(const char *msg,const char *arg)
{
  fprintf(stderr,"\nError in tz_array.c:\n");
  fprintf(stderr,msg,arg);
  fprintf(stderr,"\n");
  exit(1);
}

void array_warning(const char *msg,const char *arg)
{
  fprintf(stderr,"\nWarning in tz_array.c:\n");
  fprintf(stderr,msg,arg);
  fprintf(stderr,"\n");
}

/*
 * Copy a segment of an array to another array at certain position.
 * a2 --> a1+offset if offset > 0
 * a2-offset --> a1 if offset < 0
 * overlap indicates if the two arrays overlap (1 for overalp, 0 for not)
 */
void arraycpy(void *a1,void *a2,int offset,int length,int overlap)
{
  if(!overlap) {
    if(offset<0)
      memcpy(a1,(byte_t*)a2-offset,length);
    else
      memcpy((byte_t*)a1+offset,a2,length);
  } else {
    if(offset<0)
      memmove(a1,(byte_t*)a2-offset,length);
    else
      memmove((byte_t*)a1+offset,a2,length);
  }
}

/* array_print(): Print an array of a certain type.
 *
 * Args: a1 - the array to print;
 *       length - the number of the elements of the given type;
 *       type - type of the array.
 * 
 * Return: void.
 */
void array_print(void* a1,int length,Data_Type_t type)
{
  tz_float64* array_f64 = (tz_float64*)a1;
  tz_float32* array_f32 = (tz_float32*)a1;
  tz_int64* array_i64 = (tz_int64*)a1;
  tz_uint64* array_u64 = (tz_uint64*)a1;
  tz_int32* array_i32 = (tz_int32*)a1;
  tz_uint32* array_u32 = (tz_uint32*)a1;
  int* array_i = (int*)a1;
  unsigned int* array_u = (unsigned int*)a1;
  tz_int16* array_i16 = (tz_int16*)a1;
  tz_uint16* array_u16 = (tz_uint16*)a1;
  tz_int8* array_i8 = (tz_int8*)a1;
  tz_uint8* array_u8 = (tz_uint8*)a1;

  int i;
  switch(type) {
  case FLOAT64_TD:
    for(i=0;i<length;i++)
      printf("%g ",array_f64[i]);
    break;
  case FLOAT32_TD:
    for(i=0;i<length;i++)
      printf("%g ",array_f32[i]);
    break;
  case INT64_TD:
    for(i=0;i<length;i++)
      printf("%lld ",array_i64[i]);
    break;
  case UINT64_TD:
    for(i=0;i<length;i++)
      printf("%llu ",array_u64[i]);
    break;
  case INT32_TD:
    for(i=0;i<length;i++)
      printf("%d ",array_i32[i]);
    break;
  case UINT32_TD:
    for(i=0;i<length;i++)
      printf("%u ",array_u32[i]);
    break;
  case INT_TD:
    for(i=0;i<length;i++)
      printf("%d ",array_i[i]);
    break;
  case UINT_TD:
    for(i=0;i<length;i++)
      printf("%u ",array_u[i]);
    break;
  case INT16_TD:
    for(i=0;i<length;i++)
      printf("%d ",array_i16[i]);
    break;
  case UINT16_TD:
    for(i=0;i<length;i++)
      printf("%u ",array_u16[i]);
    break;
  case INT8_TD:
    for(i=0;i<length;i++)
      printf("%d ",array_i8[i]);
    break;
  case UINT8_TD:
    for(i=0;i<length;i++)
      printf("%u ",array_u8[i]);
    break;
  default:
    array_error("Invalid type in %s","array_print");
  }
  printf("\n");
}

/* array_write(): Write an array to a file.
 *
 * Args: a1 - the array to write;
 *       length - the number of bytes to write;
 *       filename - path of the file. If the file does not exist, it will be
 *                  created. Otherwise it will be
 *
 * Return: 0 if the writing is successful, otherwite it returns an error code.
 */
int array_write(const void *a1,int length,const char *filename)
{
  FILE* fp;
  if( (fp=fopen(filename,"w+"))==NULL )
    return ERROR_IO_OPEN;

  if(fwrite(&length,sizeof(int),1,fp)!=1) {
    fclose(fp);
    return ERROR_IO_WRITE;
  }

  if(fwrite(a1,1,length,fp)!=length) {
    fclose(fp);
    return ERROR_IO_WRITE;
  }
  
  fclose(fp);

  return 0;
}

/* array_read(): read an array from a file;
 *
 * Args: a1 - array to read;
 *       length - length of the array. No need to initialize the value, which
 *                will be read from the file. The unit is byte.
 *       filename - path of the file, whose content begins with the length of
 *                  the array.
 *
 * Return: 0 if it is successful, otherwise it returns an error code.
 */
int array_read(void *a1,int *length,const char* filename)
{
  FILE *fp;
  if( (fp=fopen(filename,"r"))==NULL )
      return ERROR_IO_OPEN;
  
  if(fread(length,sizeof(int),1,fp)!=1) {
    fclose(fp);
    return ERROR_IO_READ;
  }
  
  if(fread(a1,1,*length,fp)!=*length) {
    fclose(fp);
    return ERROR_IO_READ;
  }

  fclose(fp);
  
  return 0;
}

/* array_writet(): Write an array with a certain data type to a file.
 *
 * Args: a1 - the array to write;
 *       type - type of the array;
 *       length - the number of elements of the type;
 *       filename - path of the witten file. It will write the type first, 
 *                  length second and then the array. The file will be 
 *                  overwritten if it exists, or the file will be created.
 *
 * Return: 0 if it is successful, otherwise it returns an error code.
 */
int array_writet(const void *a1,Data_Type_t type,int length,
		 const char *filename)
{
  FILE *fp;

  if(!valid_type(type))
    return ERROR_DATA_TYPE;

  if( (fp=fopen(filename,"w+"))==NULL )
      return ERROR_IO_OPEN;

  if(fwrite(&type,sizeof(Data_Type_t),1,fp)!=1) {
    fclose(fp);
    return ERROR_IO_WRITE;
  }
  
  if(fwrite(&length,sizeof(int),1,fp)!=1) {
    fclose(fp);
    return ERROR_IO_WRITE;
  }

  if(fwrite(a1,gDataSize[type],length,fp)!=length) {
    fclose(fp);
    return ERROR_IO_WRITE;
  }

  fclose(fp);

  return 0;
}

/* array_readt(): Read an array from a file.
 *     
 * Args: a1 - the pointer to store the read array;
 *       type - type of the array;
 *       length - number of the elements of the array with the type;
 *       filename - path of the file.
 * 
 * Return: 0 if it is successful, otherwise it returns FALSE.
 *
 * Note: <a1>, <type>, <length> are where the results are stored.
 */
int array_readt(void *a1,Data_Type_t *type,int *length,
		 const char *filename)
{
  FILE* fp;
  
  if( (fp=fopen(filename,"r"))==NULL )
      return ERROR_IO_OPEN;

  if(fread(type,sizeof(Data_Type_t),1,fp)!=1) {
    fclose(fp);
    return ERROR_IO_READ;
  }

  if(!valid_type(*type)) {
    fclose(fp);
    return ERROR_IO_FORMAT;
  }

  if(fread(length,sizeof(int),1,fp)!=1) {
    fclose(fp);
    return ERROR_IO_READ;
  }

  if(fread(a1,gDataSize[*type],*length,fp)!=*length) {
    fclose(fp);
    return ERROR_IO_READ;
  }

  fclose(fp);

  return 0;
}
