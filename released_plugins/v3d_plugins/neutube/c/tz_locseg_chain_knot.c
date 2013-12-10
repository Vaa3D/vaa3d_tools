#include <stdio.h>
#include <utilities.h>
#include "tz_locseg_chain_knot.h"

Locseg_Chain_Knot* New_Locseg_Chain_Knot()
{
  Locseg_Chain_Knot *knot = (Locseg_Chain_Knot*)
    Guarded_Malloc(sizeof(Locseg_Chain_Knot), "New_Locseg_Chain_Knot");
  
  knot->id = -1;
  knot->offset = 0.0;

  return knot;
}

Locseg_Chain_Knot* Make_Locseg_Chain_Knot(int id, double offset)
{
  Locseg_Chain_Knot *knot = New_Locseg_Chain_Knot();
  knot->id = id;
  knot->offset = offset;

  return knot;
}

void Print_Locseg_Chain_Knot(const Locseg_Chain_Knot *knot)
{
  printf("%d, %g\n", knot->id, knot->offset);
}

#define LOCSEG_CHAIN_IS_EQUAL_EPS 1e-5

BOOL Locseg_Chain_Knot_Is_Equal(const Locseg_Chain_Knot *knot1, 
				const Locseg_Chain_Knot *knot2) 
{
  return ((knot1->id == knot2->id) && 
	  (fabs(knot1->offset - knot2->offset) < LOCSEG_CHAIN_IS_EQUAL_EPS));
}

Locseg_Chain_Knot_Array* New_Locseg_Chain_Knot_Array()
{
  Locseg_Chain_Knot_Array *ka = (Locseg_Chain_Knot_Array*)
    Guarded_Malloc(sizeof(Locseg_Chain_Knot_Array),
		   "New_Locseg_Chain_Knot_Array");
  ka->chain = NULL;
  ka->knot = NULL;

  return ka;
}

void Delete_Locseg_Chain_Knot_Array(Locseg_Chain_Knot_Array *ka)
{
  free(ka);
}

void Clean_Locseg_Chain_Knot_Array(Locseg_Chain_Knot_Array *ka)
{
  Kill_Unipointer_Arraylist(ka->knot);
  ka->chain = NULL;
  ka->knot = NULL;
}

void Kill_Locseg_Chain_Knot_Array(Locseg_Chain_Knot_Array *ka)
{
  Clean_Locseg_Chain_Knot_Array(ka);
  Delete_Locseg_Chain_Knot_Array(ka);
}

int Locseg_Chain_Knot_Array_Length(const Locseg_Chain_Knot_Array *ka)
{
  if (ka == NULL) {
    return 0;
  }

  return ka->knot->length;
}

void Locseg_Chain_Knot_Array_Append(Locseg_Chain_Knot_Array *ka, 
				    Locseg_Chain_Knot *knot)
{
  if (ka == NULL) {
    return;
  }

  if (ka->knot == NULL) {
    ka->knot = Make_Unipointer_Arraylist(0, 1);
  }

  Unipointer_Arraylist_Add(ka->knot, knot);
}

void Locseg_Chain_Knot_Array_Append_U(Locseg_Chain_Knot_Array *ka, 
				      Locseg_Chain_Knot *knot)
{
  if (Locseg_Chain_Knot_Is_Equal(Locseg_Chain_Knot_Array_Last(ka), knot) 
      == FALSE) {
    Unipointer_Arraylist_Add(ka->knot, knot);
  }
}

Locseg_Chain_Knot* 
Locseg_Chain_Knot_Array_At(const Locseg_Chain_Knot_Array *ka, int index)
{
  if (index < 0 || index >= ka->knot->length) {
    return NULL;
  }

  return (Locseg_Chain_Knot *) ka->knot->array[index];
}

Locseg_Chain_Knot* 
Locseg_Chain_Knot_Array_Last(const Locseg_Chain_Knot_Array *ka)
{
  return Locseg_Chain_Knot_Array_At(ka, ka->knot->length - 1);
}

void Print_Locseg_Chain_Knot_Array(const Locseg_Chain_Knot_Array *ka)
{
  int i;
  for (i = 0; i < ka->knot->length; i++) {
    Print_Locseg_Chain_Knot(Locseg_Chain_Knot_Array_At(ka, i));
  }
}

Geo3d_Circle* 
Locseg_Chain_Knot_Array_To_Circle_Z(Locseg_Chain_Knot_Array *ka, double z_scale,
				    Geo3d_Circle *circle)
{
  int n = Locseg_Chain_Knot_Array_Length(ka);

  if (circle == NULL) {
    circle = Make_Geo3d_Circle_Array(n);
  }

  Geo3d_Circle *circle_head = circle;
  Local_Neuroseg *locseg = NULL;
  
  Locseg_Chain_Iterator_Start(ka->chain, DL_HEAD);
  int index = 0;
  int knot_index = 0;

  while ((locseg = Locseg_Chain_Next_Seg(ka->chain)) != NULL) {
    Local_Neuroseg locseg2;
    locseg2 = *locseg;

    if (z_scale != 1.0) {
      Local_Neuroseg_Scale_Z(&locseg2, z_scale);
    }

    Locseg_Chain_Knot *knot = Locseg_Chain_Knot_Array_At(ka, knot_index);
    while (knot != NULL) {
      if (knot->id == index) {
	Local_Neuroseg_To_Circle_T(&locseg2, knot->offset, NEUROSEG_CIRCLE_RX, 
				   circle);
	circle++;
	knot_index++;
	knot = Locseg_Chain_Knot_Array_At(ka, knot_index);
      } else {
	break;
      }
    }
    
    index++;
  }

  return circle_head;
}

Geo3d_Circle* 
Locseg_Chain_Knot_Array_To_Circle_S(Locseg_Chain_Knot_Array *ka, double z_scale,
				    double xy_scale, Geo3d_Circle *circle)
{
  int n = Locseg_Chain_Knot_Array_Length(ka);

  if (circle == NULL) {
    circle = Make_Geo3d_Circle_Array(n);
  }

  Geo3d_Circle *circle_head = circle;
  Local_Neuroseg *locseg = NULL;
  
  Locseg_Chain_Iterator_Start(ka->chain, DL_HEAD);
  int index = 0;
  int knot_index = 0;

  while ((locseg = Locseg_Chain_Next_Seg(ka->chain)) != NULL) {
    Local_Neuroseg locseg2;
    locseg2 = *locseg;

    /*
    if (z_scale != 1.0) {
      Local_Neuroseg_Scale_Z(&locseg2, z_scale);
    }
    */
    Local_Neuroseg_Scale(&locseg2, xy_scale, z_scale);

    Locseg_Chain_Knot *knot = Locseg_Chain_Knot_Array_At(ka, knot_index);
    while (knot != NULL) {
      if (knot->id == index) {
	Local_Neuroseg_To_Circle_T(&locseg2, knot->offset, NEUROSEG_CIRCLE_RX,
				   circle);
	circle++;
	knot_index++;
	knot = Locseg_Chain_Knot_Array_At(ka, knot_index);
      } else {
	break;
      }
    }
    
    index++;
  }

  return circle_head;
}

Geo3d_Ellipse* 
Locseg_Chain_Knot_Array_To_Ellipse_Z(Locseg_Chain_Knot_Array *ka, 
				     double z_scale,
				     Geo3d_Ellipse *ellipse)
{
  int n = Locseg_Chain_Knot_Array_Length(ka);

  if (ellipse == NULL) {
    GUARDED_MALLOC_ARRAY(ellipse, n, Geo3d_Ellipse);
  }

  Geo3d_Ellipse *ellipse_head = ellipse;
  Local_Neuroseg *locseg = NULL;
  
  Locseg_Chain_Iterator_Start(ka->chain, DL_HEAD);
  int index = 0;
  int knot_index = 0;

  while ((locseg = Locseg_Chain_Next_Seg(ka->chain)) != NULL) {
    Local_Neuroseg locseg2;
    locseg2 = *locseg;

    if (z_scale != 1.0) {
      Local_Neuroseg_Scale_Z(&locseg2, z_scale);
    }

    Locseg_Chain_Knot *knot = Locseg_Chain_Knot_Array_At(ka, knot_index);
    while (knot != NULL) {
      if (knot->id == index) {
	Local_Neuroseg_To_Geo3d_Ellipse_Z(&locseg2, ellipse,
					 knot->offset * (locseg2.seg.h - 1.0));
	ellipse++;
	knot_index++;
	knot = Locseg_Chain_Knot_Array_At(ka, knot_index);
      } else {
	break;
      }
    }
    
    index++;
  }

  return ellipse_head;
}

void Locseg_Chain_Knot_Pos(Locseg_Chain_Knot_Array *ka, int index, double *pos)
{
  Locseg_Chain_Knot *knot = Locseg_Chain_Knot_Array_At(ka, index);
  Local_Neuroseg *locseg = Locseg_Chain_Peek_Seg_At(ka->chain, knot->id);
  
  Local_Neuroseg_Axis_Coord_N(locseg, knot->offset, pos);
}
