/**@file tz_pixel_array.h
 * @brief pixel array
 * @author Ting Zhao
 * @date 30-Nov-2007
 */

#ifndef _TZ_PIXEL_ARRAY_H_
#define _TZ_PIXEL_ARRAY_H_

#include "tz_cdefs.h"

__BEGIN_DECLS

typedef struct tagPixel_Array {
  int kind; /* pixel kind, which is a kind of an image or stack */
  int size; /* number of pixels */
  void *array; /* value of the pixels */
} Pixel_Array;

Pixel_Array *New_Pixel_Array();
void Default_Pixel_Array(Pixel_Array *pa);

Pixel_Array *Make_Pixel_Array(int kind, int size);
Pixel_Array *Copy_Pixel_Array(const Pixel_Array *pa);
void   Pack_Pixel_Array(Pixel_Array *pa);
void   Free_Pixel_Array(Pixel_Array *pa);
void   Kill_Pixel_Array(Pixel_Array *pa);
void   Reset_Pixel_Array(Pixel_Array *pa);
int    Pixel_Array_Usage();

size_t Pixel_Array_Fread(Pixel_Array *pa, FILE *stream);
size_t Pixel_Array_Fwrite(const Pixel_Array *pa, FILE *stream);

Pixel_Array* Pixel_Array_Read(const char *file_path);
void Pixel_Array_Write(const char *file_path, const Pixel_Array *pa);

void Print_Pixel_Array(const Pixel_Array *pa);

void Pixel_Array_Export(const char *file_path, const Pixel_Array *pa);

double Pixel_Array_Value(const Pixel_Array *pa, int index);

__END_DECLS

#endif
