/* tz_object_3d_linked_list_ext.c
 *
 * 09-Dec-2007 Initial write: Ting Zhao
 */

#include <stdlib.h>
#include <string.h>
#include "tz_error.h"
#include "tz_object_3d_linked_list_ext.h"
#include "tz_stack_lib.h"
#include "tz_image_io.h"

INIT_EXCEPTION

/* Object_3d_List_Size(): Total size of the objects in a list.
 *
 * Args: objs - object list.
 *
 * Return: total size. It returns 0 if <objs> is NULL.
 */
int Object_3d_List_Size(const Object_3d_List *objs)
{
  int size = 0;
  while (objs != NULL) {
    size += objs->data->size;
    objs = objs->next;
  }

  return size;
}

/* This file provides more functions for using the object 3d list. It is
 * not a template generated file.*/

/* Object_3d_List_Translate(): Translate all objects in a list.
 *
 * Args: objs - object list, whose objects will be translated;
 *       x - x offset;
 *       y - y offset;
 *       z - z offset.
 *
 * Return: void.
 */
void Object_3d_List_Translate(Object_3d_List *objs, int x, int y, int z)
{
  while (objs != NULL) {
    Object_3d_Translate(objs->data, x, y, z);
    objs = objs->next;
  }
}

/* Object_3d_List_Centroid(): Calculate the centroid of a list of objects.
 *
 * Args: objs - a list of objects;
 *       pos - array to store the results.
 *
 * Return: void.
 */
void Object_3d_List_Centroid(const Object_3d_List *objs, double *pos)
{
  int totalSize = 0;
  double obj_centroid[3];
  int i;
  pos[0] = pos[1] = pos[2] = 0.0;
  while (objs != NULL) {
    Object_3d_Centroid(objs->data, obj_centroid);
    for (i = 0; i < 3; i++) {
      pos[i] += obj_centroid[i] * (double) objs->data->size;
    }
    
    totalSize += objs->data->size;
    objs = objs->next;
  }

  for (i = 0; i < 3; i++) {
    pos[i] /= (double) totalSize;
  }
}

/* Object_3d_List_Centroid_Max(): Centroid of the largest object.
 */
void Object_3d_List_Centroid_Max(const Object_3d_List *objs, double *pos)
{
  int max_size = 0;

  pos[0] = pos[1] = pos[2] = 0.0;
  while (objs != NULL) {
    if (objs->data->size > max_size) {
      Object_3d_Centroid(objs->data, pos);
      max_size = objs->data->size;
    }
    objs = objs->next;
  }
}

Object_3d* Object_3d_List_To_Object(const Object_3d_List *objs, Object_3d *obj)
{
  if (objs == NULL) {
    return NULL;
  }

  int total_size = Object_3d_List_Size(objs);

  if (obj == NULL) {
    obj = Make_Object_3d(total_size, 0);
  } else {
    if (total_size != obj->size) {
      THROW(ERROR_DATA_COMPTB);
    }
  }

  Voxel_t *dst_array = obj->voxels;
  while (objs != NULL) {
    memcpy(dst_array, objs->data->voxels, sizeof(Voxel_t) * objs->data->size);
    dst_array += objs->data->size;
    objs = objs->next;
  }

  return obj;
}

Neurons *read_neurons(FILE *input, int numchans)
{ 
  Neurons *neu = (Neurons *) Guarded_Malloc(sizeof(Neurons),Program_Name());
  int      i;

  fread(neu,sizeof(Neurons),1,input);

  neu->neuron  = (mylib_Region **) Guarded_Malloc(sizeof(mylib_Region *)*neu->numneu,Program_Name());
  neu->color   = (int *) Guarded_Malloc(sizeof(int)*neu->numneu*numchans,Program_Name());
  neu->cluster = (int *) Guarded_Malloc(sizeof(int)*neu->numneu,Program_Name());
  neu->status  = (int *) Guarded_Malloc(sizeof(int)*neu->numneu,Program_Name());

  for (i = 0; i < neu->numneu; i++)
    neu->neuron[i] = Read_Region(input);

  fread(neu->color,sizeof(int),neu->numneu*numchans,input);
  fread(neu->cluster,sizeof(int),neu->numneu,input);
  fread(neu->status,sizeof(int),neu->numneu,input);

  return (neu);
}

Neurons* Make_Neurons(int numneu, int numchan)
{
  Neurons *neu = (Neurons *) Guarded_Malloc(sizeof(Neurons),Program_Name());
  neu->numneu = numneu;
  neu->neuron  = (mylib_Region **) Guarded_Calloc(neu->numneu,
      sizeof(mylib_Region *),Program_Name());
  neu->color   = (int *) Guarded_Calloc(neu->numneu * numchan, 
      sizeof(int),Program_Name());
  neu->cluster = (int *) Guarded_Calloc(neu->numneu,sizeof(int),Program_Name());
  neu->status  = (int *) Guarded_Calloc(neu->numneu,sizeof(int),Program_Name());

  return neu;
}

void Kill_Neurons(Neurons *neu)
{ int i;

  for (i = 0; i < neu->numneu; i++)
    Kill_Region(neu->neuron[i]);
  free(neu->color);
  free(neu->cluster);
  free(neu->neuron);
  free(neu->status);
}


void Print_Neurons(const Neurons *neuron)
{
  printf("%d neurons:\n", neuron->numneu);

  int i;
  for (i = 0; i < neuron->numneu; ++i) {
    printf("  neuron %d: %lu pixels\n", i + 1, Region_Size(neuron->neuron[i]));
  }
}

int Infer_Neuron_File_Channel_Number(const char *file_path)
{
  FILE *input = fopen(file_path, "rb");

  if (input == NULL) {
    printf("Cannot open %s\n", file_path);
  }

  fseek(input, sizeof(double), SEEK_CUR);
  fseek(input, sizeof(double), SEEK_CUR);
  fseek(input, sizeof(int), SEEK_CUR);
  int NStacks;
  fread(&NStacks,sizeof(int),1,input);
  int m;
  int i;
  for (i = 0; i < NStacks; i++) {
    fread(&m,sizeof(int),1,input);
    fseek(input, sizeof(char) * m, SEEK_CUR);
    fseek(input, sizeof(int), SEEK_CUR);
  }
  fread(&m,sizeof(int),1,input);

  char signal_file[MAX_PATH_LENGTH];
  fread(signal_file, sizeof(char), m, input);
  signal_file[m] = '\0';
  char *dir = dname(file_path, NULL);

  char *signal_path = fullpath(dir, signal_file, NULL);

  free(dir);

  int channel_number = 0;
  if (fexist(signal_path) && fhasext(signal_path, "pbd")) {
    channel_number = V3dpbd_Channel_Number(signal_path);
  } else {
 //fseek(input, sizeof(char) * m, SEEK_CUR);

    size_t offset = 0;
    char buffer[7];
    buffer[6] = '\0';

    /* Search for the first occurence of "Region" */
    while (!feof(input)) {
      fread(buffer, sizeof(char), 1, input);
      ++offset;
      if (buffer[0] == 'R') {
        fread(buffer + 1, sizeof(char), 5, input);
        offset += 5;
        if (strcmp(buffer, "Region") == 0) {
          break;
        } else {
          fseek(input, -5, SEEK_SET);
          offset -= 5;
        }
      }
    }

    fclose(input);

    channel_number = (offset - 6 - sizeof(Neurons)) / sizeof(double) / 2;
  }

  return channel_number;
}

Neurons *Read_Neurons(const char *file_path, int numchans)
{
  FILE *fp = Guarded_Fopen(file_path, "rb", "Read_Neurons");

  mylib_Prolog *prolog = Read_Prolog(fp, numchans, 1, NULL);
#ifdef _DEBUG_
  Print_Prolog(prolog);
#endif

  Kill_Prolog(prolog);

  Neurons *neu = read_neurons(fp, numchans);

  fclose(fp);

  return neu;
}

Object_3d_List* Neuron_To_Object_3d(const Neurons *neu)
{
  if (neu == NULL) {
    return NULL;
  }

  if (neu->numneu == 0) {
    return NULL;
  }

  int i;
  /* alloc <obj_list> */
  Object_3d_List *obj_list = Object_3d_List_New();

  for (i = 0; i < neu->numneu; ++i) {
    Object_3d *obj = Region_To_Object_3d(neu->neuron[i]);
    Object_3d_List_Add_Last(obj_list, obj);
  }

  /* return <obj_list> */
  return obj_list;
}

Stack* Neuron_To_Stack(Neurons *neu, Stack *out)
{
  if (neu == NULL) {
    return NULL;
  }

  if (neu->numneu == 0) {
    return NULL;
  }

  TZ_ASSERT(neu->numneu <= 65535, "Too many neurons.");

  int width, height, depth;
  Region_Stack_Size(neu->neuron[0], &width, &height, &depth);
  if (out == NULL) {
    out = Make_Stack(GREY16, width, height, depth);
  }

  TZ_ASSERT(out->kind == GREY16, "Unsupported stack kind.");
  
  Zero_Stack(out);

  int i;
  for (i = 0; i < neu->numneu; ++i) {
    Region_Label_Stack(neu->neuron[i], i + 1, out);
  }

  return out;
}

void Write_Neurons(const char *file_path, const Neurons *neu, 
    const mylib_Prolog *prolog, int numchan)
{
  FILE *fp = fopen(file_path, "w");
  if (fp == NULL) {
    fprintf(stderr, "Cannont open %s to write", file_path);
  }

  Write_Prolog(prolog, fp);
  int i;
  fwrite(neu, sizeof(Neurons), 1, fp);
  for (i = 0; i < neu->numneu; ++i) {
    Write_Region(neu->neuron[i], fp);
  }
  fwrite(neu->color, sizeof(int), neu->numneu * numchan, fp);
  fwrite(neu->cluster, sizeof(int), neu->numneu, fp);
  fwrite(neu->status, sizeof(int), neu->numneu, fp);

  fclose(fp);
}

size_t Neuron_Size(const Neurons *neu)
{
  int i;
  size_t s = 0;
  for (i = 0; i < neu->numneu; ++i) {
    s += Region_Size(neu->neuron[i]);
  }

  return s;
}
