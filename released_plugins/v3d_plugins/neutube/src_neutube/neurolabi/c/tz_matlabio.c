#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifndef _MSC_VER
#include <dirent.h>
#else
#include "tz_dirent.h"
#endif
#include "tz_matlabio.h"
#ifdef HAVE_LIBZ
#  include <zlib.h>
#endif
#include "tz_error.h"
#include "tz_matlabdefs.h"

#define MI_ENDIAN 0x4D49
#define IM_ENDIAN 0x494D
#define COMPLEX_FLAG 0x00200000
#define GLOBAL_FLAG 0x00100000
#define LOGICAL_FLAG 0x00020000

#define  FLAG_MASK ((tz_uint32)65280)
#define CLASS_MASK ((tz_uint32)255)

#define DATA_TYPE_ERROR 1 
#define DATA_SIZE_ERROR 2
#define ENDIAN_ERROR 3
#define ENDIAN_MISMATCH 4
#define DATA_FLAG_ERROR 5
#define DATA_CLASS_ERROR 6
#define OTHER_ERROR 100

#define TAG_OFFSET 8
#define DESCRP_LENGTH 116

void Clean_Matlab_Array(Matlab_Array *ma)
{
  free(ma->array);
}

void Default_Matlab_Array(Matlab_Array *ma)
{
  ma->type = miDOUBLE;
  ma->flag = 0;
  ma->ndim = 0;
  ma->array = NULL;
}

DEFINE_ZOBJECT_INTERFACE(Matlab_Array)

/*normalize an integer to be a multiple of 8*/
inline tz_uint32 norm_size(tz_uint32 size)
{
  if( (size&0x00000007) == 0) {
    return size;
  }

  return ((size>>3)+1)<<3;  
}

int parse_header(void* stream,char* description,tz_uint64* subsys_offset,tz_uint16* version,tz_uint16* endian) 
{
  memcpy((void*)description,stream,DESCRP_LENGTH);
  *subsys_offset = *((tz_uint64*)((byte_t*)stream+DESCRP_LENGTH));
  *version = *((tz_uint16*)((byte_t*)stream+124));
  *endian = *((tz_uint16*)((byte_t*)stream+126));

  if(*endian==IM_ENDIAN) //uncompatible endian
    return ENDIAN_MISMATCH;
  
  if(*endian!=MI_ENDIAN) //unknown endian
    return ENDIAN_ERROR;

  return 0;
}

int parse_tag(void* stream,tz_uint32* data_type,tz_uint32* data_size)
{
  *data_type = *( ((tz_uint32*)stream) );
  if(*data_type<1 || *data_type>18) //invalid type
    return DATA_TYPE_ERROR;
  *data_size = *( ((tz_uint32*)stream)+1 );

  return 0;
}

int parse_element(void* stream,tz_uint32* data_type,tz_uint32* data_size,tz_uint32* data_start)
{
  tz_uint32 small_data = *((tz_uint32*)stream) & ((tz_uint32)4294901760u);

  if(small_data) {
    *data_type = *( ((tz_uint16*)stream) );
    *data_size = *( ((tz_uint16*)stream)+1 );
    *data_start = 4;
  } else {
    *data_type = *( ((tz_uint32*)stream) );
    *data_size = *( ((tz_uint32*)stream)+1 );
    *data_start = 8;    
  }

  return 0;
}

int parse_arrayflags(void* stream,tz_uint8* flags,tz_uint8* data_class,tz_uint32* offset)
{
  int succ;
  tz_uint32 data_type,data_size,data_start,data,mask_data;
  int i;

  succ = parse_element(stream,&data_type,&data_size,&data_start);

  if(data_type!=miUINT32) //data type error
    return DATA_TYPE_ERROR;
  if(data_size!=8) //data size error
    return DATA_SIZE_ERROR;

  data = *( (tz_uint32*)((byte_t*)stream+data_start) );
  mask_data = data&FLAG_MASK;

  if(!mask_data)
    *flags = 0;
  else{
    for(i=1;i<4;i++) {
      if(mask_data&0x000000FF)
	break;
      mask_data >>= 8;
    }
  }

  *flags = *( (tz_uint8*)(&mask_data) );

  mask_data = data&CLASS_MASK;

  if(!mask_data)
    return DATA_CLASS_ERROR;

  for(i=1;i<4;i++) {
    if(mask_data&0x000000FF)
      break;
    mask_data >>= 8;
  }
  *data_class = *( (tz_uint8*)(&mask_data) );

  *offset = norm_size(data_start+data_size);

  return succ;
}

int parse_dim(void* stream,tz_uint32* ndim,tz_uint32* dim,tz_uint32* offset)
{
  int succ;
  tz_uint32 data_type,data_size,data_start;

  succ = parse_element(stream,&data_type,&data_size,&data_start);

  if(data_type!=miINT32) //data_type error
    return DATA_TYPE_ERROR;

  *ndim = data_size/type_size(data_type);
  tz_uint32 i;
  for(i=0;i<*ndim;i++)
    dim[i] = *( (tz_uint32*)((byte_t*)stream+data_start)+i );

  *offset = norm_size(data_start+data_size);

  return succ;
}

int parse_varname(void* stream,char* name,tz_uint32* offset)
{
  int succ;
  tz_uint32 data_type,data_size,data_start;

  succ = parse_element(stream,&data_type,&data_size,&data_start);

  if(data_type!=miINT8)
    return DATA_TYPE_ERROR;

  memcpy(name,(byte_t*)stream+data_start,data_size);
  name[data_size] = '\0';

  *offset = norm_size(data_start+data_size);

  return succ;  
}

void* uncompress_data(void* stream,tz_uint32 length,tz_uint32 *length2)
{
#ifdef HAVE_LIBZ
  Bytef* destData;
  int status;
  uLong destLen = (uLong)length*2+12;
  destData = (Bytef*) malloc(destLen);
  status = uncompress(destData,&destLen,(Bytef*)stream,length);
  while(status == Z_BUF_ERROR) {
    destLen +=length;
    destData = (Bytef*) realloc(destData,destLen);
    status = uncompress(destData,&destLen,(Bytef*)stream,length);
  }
  *length2 = (tz_uint32)destLen;

  return (void*)destData;
#else
  TZ_ERROR(ERROR_NA_FUNC);
  return NULL;
#endif
}

int parse_data(void* stream,Matlab_Array* mr,int has_tag)
{
  int succ;
  tz_uint32 dataType,dataSize,dataOffset,totalOffset;

  if(has_tag) {
    parse_tag(stream,&dataType,&dataSize);
    
    if(dataType!=miMATRIX)
      return DATA_TYPE_ERROR;
    
    if(dataSize%8)
      return DATA_SIZE_ERROR;

    totalOffset = 8;
  } else
    totalOffset = 0;

  succ = parse_arrayflags((byte_t*)stream+totalOffset,&(mr->flag),&(mr->type),&dataOffset);
  if(succ)
    return succ;

  totalOffset += dataOffset;

  succ = parse_dim((byte_t*)stream+totalOffset,&(mr->ndim),mr->dim,&dataOffset);
  if(succ)
    return succ;
 
  totalOffset += dataOffset;

  succ = parse_varname((byte_t*)stream+totalOffset,mr->name,&dataOffset);
  if(succ)
    return succ;

  totalOffset += dataOffset;

  succ = parse_element((byte_t*)stream+totalOffset,&dataType,&dataSize,&dataOffset);
  if(succ)
    return succ;

  mr->array = malloc(dataSize);
  memcpy(mr->array,(byte_t*)stream+totalOffset+dataOffset,dataSize);
  
  return succ;
}

tz_uint32 type_size(tz_uint32 type)
{
  switch(type) {
  case miINT8:
  case miUINT8:
  case miUTF8:
    return 1;
  case miINT16:
  case miUINT16:
  case miUTF16:
    return 2;
  case miINT32:
  case miUINT32:
  case miSINGLE:
  case miUTF32:
    return 4;
  case miDOUBLE:
  case miINT64:
  case miUINT64:
    return 8;
  default:
    return 0;
  }
}

void print_mrinfo(Matlab_Array* mr)
{
  printf("Data type: %u; Flag: %u\n",mr->type,mr->flag);
  printf("Data size: ");
  tz_uint32 i;
  for(i=0;i<mr->ndim;i++)
    printf("%u ",mr->dim[i]);
  printf("\n");
  printf("Variable name: %s\n",mr->name);
  printf("Data address: %p\n",mr->array);
}

int mr_read(const char *filename, Matlab_Array* mr)
{
  FILE *fp  = fopen(filename,"r");

  if (fp == NULL)
    TZ_WARN(ERROR_IO_OPEN);

  char fileHeader[128];
  fread(fileHeader,128,sizeof(char),fp);
  char tag[8];
  tz_uint32 dataType,dataSize;
  void* data;

  fread(tag,8,1,fp);
  int status = parse_tag(tag,&dataType,&dataSize);
  if(status)
    return status;

  data = malloc(dataSize);
  fread(data,dataSize,1,fp);

  fclose(fp);

  void* uncomData = data;
  tz_uint32 uncomDatasize = dataSize;
  int has_tag = 0;

  if(dataType==miCOMPRESSED) {
    has_tag = 1;

    uncomData = uncompress_data(data,dataSize,&uncomDatasize);
#ifdef _DEBUG_2
    FILE* fp2 = fopen("../data/mattest_uncom.mat","w+");
    fwrite(fileHeader,1,128,fp2);
    fwrite(uncomData,1,uncomDatasize,fp2);
    fclose(fp2);
#endif
    free(data);
  }

  status = parse_data(uncomData,mr,has_tag);

  free(uncomData);

  if (status)
    TZ_WARN(ERROR_UNKNOWN);

  return status;
}

#define FIND_MATLAB_BINPATH_FAILED(path)	\
  path[0] = '\0';				\
  if (dir != NULL) closedir(dir);		\
  return;

void Find_Matlab_Binpath(char *path)
{
  const char *app_root = "/Applications";
  DIR *dir = opendir(app_root);
  if (dir == NULL) {
    FIND_MATLAB_BINPATH_FAILED(path);
  } else {
    struct dirent *ent = readdir(dir);
    while (ent != NULL) {
      if (strlen(ent->d_name) >= 6) { /* MATLAB */
	if (strncmp(ent->d_name, "MATLAB", 6) == 0) {
	  strcpy(path, app_root);
	  strcat(path, "/");
	  strcat(path, ent->d_name);
	  strcat(path, "/bin/matlab");
	  break;
	}
      }
      ent = readdir(dir);
    }

    if (ent == NULL) {
      FIND_MATLAB_BINPATH_FAILED(path);
    }

    closedir(dir);
  }
}

Matlab_Array* Read_Matlab_Array(const char *filepath)
{
  Matlab_Array *ma = New_Matlab_Array();
  
  mr_read(filepath, ma);

  return ma;
}
