/**@file tz_object_3d_linked_list_ext.h
 * @brief additional routines for 3d object list
 * @author Ting Zhao
 * @date 09-Dec-2007
 */

#ifndef _TZ_OBJECT_3D_LINKED_LIST_EXT_H_
#define _TZ_OBJECT_3D_LINKED_LIST_EXT_H_

#include "tz_cdefs.h"
#include "tz_object_3d_linked_list.h"

__BEGIN_DECLS

int Object_3d_List_Size(const Object_3d_List *objs);

void Object_3d_List_Translate(Object_3d_List *objs, int x, int y, int z);
void Object_3d_List_Centroid(const Object_3d_List *objs, double *pos);
void Object_3d_List_Centroid_Max(const Object_3d_List *objs, double *pos);

Object_3d* Object_3d_List_To_Object(const Object_3d_List *objs, Object_3d *obj);

typedef struct
{ int      numneu;    //   Number of neurons reported
  mylib_Region **neuron;    //   Neuron i in [0,numneu) is neuron[i]
  int     *color;     //   Colors of neurons
  int     *cluster;
  int     *status;
} Neurons;

Neurons* Make_Neurons(int numneu, int numchan);
void Kill_Neurons(Neurons *neu);
int Infer_Neuron_File_Channel_Number(const char *file_path);
Neurons *read_neurons(FILE *input, int numchans);
Neurons *Read_Neurons(const char *file_path, int numchans);
void Write_Neurons(const char *file_path, const Neurons *neu, 
    const mylib_Prolog *prolog, int numchan);
void Print_Neurons(const Neurons *neuron);

/**@brief Convert neurons to a label field.
 * 
 * <out> must be GREY16 type if it is not NULL.
 */
Stack* Neuron_To_Stack(Neurons *neu, Stack *out);

size_t Neuron_Size(const Neurons *neu);

Object_3d_List* Neuron_To_Object_3d(const Neurons *neu);
__END_DECLS

#endif
