/* tz_<1t>_node.c
 *
 * 11-Aug-2008 Initial write: Ting Zhao
 */

#include <stdlib.h>
#include <utilities.h>
#include "tz_error.h"
#include "tz_trace_utils.h"
#include "tz_<1t>_node.h"
#include "string.h"

<1T>_Node* New_<1T>_Node()
{
  <1T>_Node *p = (<1T>_Node*) 
    Guarded_Malloc(sizeof(<1T>_Node), "New_<1T>_Node");
  
  p->locseg = NULL;
  p->tr = NULL;

  return p;
}

void Delete_<1T>_Node(<1T>_Node *p)
{
  if (p != NULL) {
    free(p);
  }
}

void Clean_<1T>_Node(<1T>_Node *p)
{
  if (p != NULL) {
    Delete_<2T>(p->locseg);
    p->locseg = NULL;
    free(p->tr);
    p->tr = NULL;
  }
}

<1T>_Node* Make_<1T>_Node(const <2T> *locseg,
			      const Trace_Record *tr)
{
  <1T>_Node *p = New_<1T>_Node();
  p->locseg = (<2T> *) locseg;
  p->tr = (Trace_Record *) tr;

  return p;
}

void Kill_<1T>_Node(<1T>_Node *p)
{
  Clean_<1T>_Node(p);
  Delete_<1T>_Node(p);
}

<1T>_Node* Copy_<1T>_Node(const <1T>_Node *p)
{
  return Make_<1T>_Node(Copy_<2T>(p->locseg),
			Copy_Trace_Record(p->tr));
}

BOOL <1T>_Node_Is_Identical(const <1T>_Node *p1, const <1T>_Node *p2)
{
  if (p1 == p2) {
    return TRUE;
  }

  if ((p1 == NULL) || (p2 == NULL)) {
    return FALSE;
  }

  if (p1->tr != p2->tr) {
    if ((p1->tr == NULL) || (p2->tr == NULL)) {
      return FALSE;
    }
    
    if (memcmp(p1->tr, p2->tr, sizeof(Trace_Record)) != 0) {
      return FALSE;
    }
  }

  if (p1->locseg != p2->locseg) {
    if ((p1->locseg == NULL) || (p2->locseg == NULL)) {
      return FALSE;
    }

    if (memcmp(p1->locseg, p2->locseg, sizeof(<2T>)) != 0) {
      return FALSE;
    }
  }

  return TRUE;
}

void Print_<1T>_Node(const <1T>_Node *p)
{
  Print_<2T>(p->locseg);
  Print_Trace_Record(p->tr);
}

void Print_<1T>_Node_Info(const <1T>_Node *p)
{
  Print_<2T>(p->locseg);
}

void <1T>_Node_Fwrite(const <1T>_Node *p, FILE *stream)
{
  ASSERT(p != NULL, "Null <1T>_Node");
  ASSERT(stream != NULL, "Invalid file pointer");
  
  <2T>_Fwrite(p->locseg, stream);
  Trace_Record_Fwrite(p->tr, stream);
}

void <1T>_Node_Fwrite_V(const <1T>_Node *p, FILE *stream, double version)
{
  ASSERT(p != NULL, "Null <1T>_Node");
  ASSERT(stream != NULL, "Invalid file pointer");
  
  <2T>_Fwrite(p->locseg, stream);
  Trace_Record_Fwrite_V(p->tr, stream, version);
}

<1T>_Node* <1T>_Node_Fread(<1T>_Node *p, FILE *stream)
{
  ASSERT(stream != NULL, "Invalid file pointer");
  
  if (feof(stream)) {
    return NULL;
  }
  
  BOOL is_local_alloc = FALSE;

  if (p == NULL) {
    p = New_<1T>_Node();
    is_local_alloc = TRUE;
  }
  
  if ((p->locseg = <2T>_Fread(p->locseg, stream)) == NULL) {
    if (is_local_alloc == TRUE) {
      Delete_<1T>_Node(p);
    }
    p = NULL;
  } else {
    if ((p->tr = Trace_Record_Fread(p->tr, stream)) == NULL) {
      if (feof(stream)) {
	if (is_local_alloc == TRUE) {
	  Delete_<1T>_Node(p);
	}
	p = NULL;
      }
    }
  }

  return p;
}

<1T>_Node* <1T>_Node_Fread_V(<1T>_Node *p, FILE *stream, double version)
{
  ASSERT(stream != NULL, "Invalid file pointer");
  
  if (feof(stream)) {
    return NULL;
  }
  
  BOOL is_local_alloc = FALSE;

  if (p == NULL) {
    p = New_<1T>_Node();
    is_local_alloc = TRUE;
  }
  
  if ((p->locseg = <2T>_Fread(p->locseg, stream)) == NULL) {
    if (is_local_alloc == TRUE) {
      Delete_<1T>_Node(p);
    }
    p = NULL;
  } else {
    if ((p->tr = Trace_Record_Fread_V(p->tr, stream, version)) == NULL) {
      if (feof(stream)) {
	if (is_local_alloc == TRUE) {
	  Delete_<1T>_Node(p);
	}
	p = NULL;
      }
    }
  }

  return p;
}

void <1T>_Node_Copy_Trace_Record(<1T>_Node *p, const Trace_Record *tr)
{
  if (p->tr == NULL) {
    p->tr = New_Trace_Record();
  }

  Trace_Record_Copy(p->tr, tr);
}

void <1T>_Node_Set_Score(<1T>_Node *p, const Stack_Fit_Score *fs)
{
  if (p->tr == NULL) {
    p->tr = New_Trace_Record();
  }

  Trace_Record_Set_Score(p->tr, fs);
}

void <1T>_Node_Set_Hit_Region(<1T>_Node *p, int hit_region)
{
  if (p->tr == NULL) {
    p->tr = New_Trace_Record();
  }

  Trace_Record_Set_Hit_Region(p->tr, hit_region);
}

void <1T>_Node_Set_Index(<1T>_Node *p, int index)
{
  if (p->tr == NULL) {
    p->tr = New_Trace_Record();
  }

  Trace_Record_Set_Index(p->tr, index);
}

void <1T>_Node_Set_Refit(<1T>_Node *p, BOOL is_refit)
{
  if (p->tr == NULL) {
    p->tr = New_Trace_Record();
  }

  Trace_Record_Set_Refit(p->tr, is_refit);
}

void <1T>_Node_Set_Direction(<1T>_Node *p, Dlist_Direction_e direction)
{
  if (p->tr == NULL) {
    p->tr = New_Trace_Record();
  }

  Trace_Record_Set_Direction(p->tr, direction);
}

void <1T>_Node_Set_Fit_Height(<1T>_Node *p, int index, int value)
{
  if (p->tr == NULL) {
    p->tr = New_Trace_Record();
  }

  Trace_Record_Set_Fit_Height(p->tr, index, value);
}

void <1T>_Node_Set_Fix_Point(<1T>_Node *p, double value)
{
  if (p->tr == NULL) {
    p->tr = New_Trace_Record();
  }

  Trace_Record_Set_Fix_Point(p->tr, value);
}

double <1T>_Node_Fix_Point(<1T>_Node *p)
{
  double t = Trace_Record_Fix_Point(p->tr);
  if ((t < 0.0) || (t > 1.0)){
    switch (Trace_Record_Direction(p->tr)) {
    case DL_FORWARD:
      t = 0.0;
      break;
    case DL_BACKWARD:
      t = 1.0;
      break;
    case DL_BOTHDIR:
      t = -1.0;
      break;
    default:
      t = 0.0;
    }
  }

  return t;
}

BOOL <1T>_Node_Fix_Pos(<1T>_Node *p, double *pos)
{
<2T=Local_Neuroseg>
  double t = <1T>_Node_Fix_Point(p);
  if (t >= 0.0) {
    Local_Neuroseg_Axis_Coord_N(p->locseg, t, pos);
    return TRUE;
  }
</T>

   return FALSE;
}
