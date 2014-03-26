#include <utilities.h>
#include "tz_objdef.h"
#include "tz_string_defs.h"

static inline int string_workspace_asize(const String_Workspace *sw)
{ return (sw->size + 1) * sizeof(int); }

DEFINE_OBJECT_MANAGEMENT(String_Workspace, array, asize, string_workspace)

void Default_String_Workspace(String_Workspace *sw)
{
  sw->iterator = NULL;
  sw->size = 0;
}

void Construct_String_Workspace(String_Workspace * sw, int size)
{
  sw->size = size;
  sw->array = (char*) Guarded_Malloc(string_workspace_asize(sw), 
				     "Construct_String_Workspace");
  sw->iterator = sw->array;
}

void Clean_String_Workspace(String_Workspace * sw)
{
  free(sw->array);
  sw->array = NULL;
  sw->iterator = NULL;
  sw->size = 0;
}

String_Workspace *Make_String_Workspace(int size)
{ 
  String_Workspace *sw= New_String_Workspace();
  
  Construct_String_Workspace(sw, size);

  return sw;
}

void String_Workspace_Realloc(String_Workspace *sw, int size)
{
  sw->size = size;
  sw->array = (char*)Guarded_Realloc(sw->array, string_workspace_asize(sw),
			      "String_Workspace_Realloc");
}

void String_Workspace_Grow(String_Workspace *sw)
{
  if (sw->size > 0) {
    String_Workspace_Realloc(sw, sw->size * 2);
    sw->size *= 2;
  } else {
    Construct_String_Workspace(sw, 2);
  }
}
