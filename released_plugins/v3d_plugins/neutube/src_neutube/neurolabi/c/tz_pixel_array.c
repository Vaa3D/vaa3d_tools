/* tz_pixel_array.c
 *
 * 30-Nov-2007 Initial write: Ting Zhao
 */

#include <image_lib.h>
#include <utilities.h>
#include "tz_error.h"
#include "tz_objdef.h"
#include "tz_image_lib.h"
#include "tz_pixel_array.h"
#include "tz_u8array.h"
#include "tz_u16array.h"
#include "tz_farray.h"
#include "tz_darray.h"

INIT_EXCEPTION

static inline int pixel_array_asize(const Pixel_Array *pa)
{ return pa->size * pa->kind; }

void Default_Pixel_Array(Pixel_Array *pa)
{
  if (pa->array == NULL) {
    return;
  }

  pa->size = 0;
  pa->kind = 1;
}

DEFINE_OBJECT_MANAGEMENT(Pixel_Array, array, asize, pixel_array)

Pixel_Array *Make_Pixel_Array(int kind, int size)
{
  Pixel_Array *pa = NULL;
  pa = new_pixel_array(size * kind, "Make_Pixel_Array");
  pa->kind = kind;
  pa->size = size;

  return pa;
}			

static size_t pixel_array_fread_attribute(Pixel_Array *pa, FILE *stream)
{
  size_t read_size = 0;

  read_size += fread(&(pa->kind), sizeof(int), 1, stream);
  read_size += fread(&(pa->size), sizeof(int), 1, stream);

  return read_size;
}

size_t Pixel_Array_Fread(Pixel_Array *pa, FILE *stream)
{
  size_t read_size = 0;

  read_size += pixel_array_fread_attribute(pa, stream);
  read_size += fread(pa->array, pixel_array_asize(pa), 1, stream);

  return read_size;
}

size_t Pixel_Array_Fwrite(const Pixel_Array *pa, FILE *stream)
{
  size_t written_size = 0;

  written_size += fwrite(&(pa->kind), sizeof(int), 1, stream);
  written_size += fwrite(&(pa->size), sizeof(int), 1, stream);
  written_size += fwrite(pa->array, pixel_array_asize(pa), 1, stream);
  
  return written_size;
}

Pixel_Array* Pixel_Array_Read(const char *file_path)
{
  Pixel_Array tmp_array;
  
  FILE *fp = Guarded_Fopen((char *) file_path, "rb", "Pixel_Array_Read");

  pixel_array_fread_attribute(&tmp_array, fp);

  Pixel_Array *pa = Make_Pixel_Array(tmp_array.kind, tmp_array.size);
  
  fread(pa->array, pixel_array_asize(pa), 1, fp);
  
  fclose (fp);

  return pa;
}

void Pixel_Array_Write(const char *file_path, const Pixel_Array *pa)
{
  FILE *fp = Guarded_Fopen((char *) file_path, "wb", "Pixel_Array_Write");
  Pixel_Array_Fwrite(pa, fp);
  fclose(fp);
}

/* Print_Pixel_Array(): Print pixel array.
 */
void Print_Pixel_Array(const Pixel_Array *pa)
{
  int i;

  DEFINE_ARRAY_ALL(array, pa);

  printf("Pixel array: kind - %d, size - %d\n", pa->kind, pa->size);
  switch (pa->kind) {
  case GREY:
    for (i = 0; i < pa->size; i++) {
      printf("%u\n", array_grey[i]);
    }
    break;
  case GREY16:
    for (i = 0; i < pa->size; i++) {
      printf("%u\n", array_grey16[i]);
    }
    break;
  case COLOR:
    for (i = 0; i < pa->size; i++) {
      printf("%u, %u, %u\n", array_color[i][0], array_color[i][1], 
	     array_color[i][2]);
    }
    break;
  case FLOAT32:
    for (i = 0; i < pa->size; i++) {
      printf("%g\n", array_float32[i]);
    }
    break;
  case FLOAT64:
    for (i = 0; i < pa->size; i++) {
      printf("%g\n", array_float64[i]);
    }
    break;
  default:
    THROW(ERROR_DATA_TYPE); 
  }
}

void Pixel_Array_Export(const char *file_path, const Pixel_Array *pa)
{
  switch (pa->kind) {
  case GREY:
    u8array_write(file_path, (const tz_uint8*)pa->array, pa->size);
    break;
  case GREY16:
    u16array_write(file_path, (const tz_uint16*)pa->array, pa->size);
    break;
  case FLOAT32:
    farray_write(file_path, (const float*)pa->array, pa->size);
    break;
  case FLOAT64:
    darray_write(file_path, (const double*)pa->array, pa->size);
    break;  
  default:
    TZ_ERROR(ERROR_DATA_TYPE);
  }
}

double Pixel_Array_Value(const Pixel_Array *pa, int index)
{
  switch (pa->kind) {
  case GREY:
    return ((uint8 *) pa->array)[index];
  case GREY16:
    return ((uint16 *) pa->array)[index];
  case FLOAT32:
    return ((float *) pa->array)[index];
  case FLOAT64:
    return ((double *) pa->array)[index];
  default:
    TZ_ERROR(ERROR_DATA_TYPE);
  }

  return 0.0;
}
