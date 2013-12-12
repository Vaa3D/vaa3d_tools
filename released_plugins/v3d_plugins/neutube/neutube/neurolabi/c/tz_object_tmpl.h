/**@file tz_object_tmpl.h
 * @brief object template
 * @author Ting Zhao
 * @date 05-Nov-2007
 */

typedef struct tagObject_T {
  int size;
  int *array;
} Object_T;

void Default_Object_T(Object_T *object_t);
Object_T *Make_Object_T(int size);

/* Functions defined in the macros */
Object_T *Copy_Object_T(const Object_T *object_t);
void   Pack_Object_T(Object_T *object_t);
void   Free_Object_T(Object_T *object_t);
void   Kill_Object_T(Object_T *object_t);
void   Reset_Object_T();
int    Object_T_Usage();

int Save_Object_T(Object_T *object_t, char *file_path);
Object_T* Load_Object_T(char *file_path);
/***********************************/


