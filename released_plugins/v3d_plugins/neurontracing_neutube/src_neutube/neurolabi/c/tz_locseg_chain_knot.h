/**@file tz_locseg_chain_knot.h
 * @brief Locseg chain knot and its array
 * @author Ting Zhao
 * @date 14-Aug-2009
 */

#ifndef _TZ_LOCSEG_CHAIN_KNOT_H_
#define _TZ_LOCSEG_CHAIN_KNOT_H_

#include "tz_cdefs.h"
#include "tz_locseg_chain_com.h"
#include "tz_unipointer_arraylist.h"
#include "tz_geo3d_ellipse.h"

__BEGIN_DECLS


typedef struct _Locseg_Chain_Knot {
  int id; /* locseg index */
  double offset; /* normalized offset from bottom */
} Locseg_Chain_Knot;

typedef struct _Locseg_Chain_Knot_Array {
  Locseg_Chain *chain;
  Unipointer_Arraylist *knot;
} Locseg_Chain_Knot_Array;

Locseg_Chain_Knot* New_Locseg_Chain_Knot();
Locseg_Chain_Knot* Make_Locseg_Chain_Knot(int id, double offset);
void Print_Locseg_Chain_Knot(const Locseg_Chain_Knot *knot);
BOOL Locseg_Chain_Knot_Is_Equal(const Locseg_Chain_Knot *knot1, 
				const Locseg_Chain_Knot *knot2);

Locseg_Chain_Knot_Array* New_Locseg_Chain_Knot_Array();
void Delete_Locseg_Chain_Knot_Array(Locseg_Chain_Knot_Array *ka);
void Clean_Locseg_Chain_Knot_Array(Locseg_Chain_Knot_Array *ka);
void Kill_Locseg_Chain_Knot_Array(Locseg_Chain_Knot_Array *ka);

int Locseg_Chain_Knot_Array_Length(const Locseg_Chain_Knot_Array *ka);

Locseg_Chain_Knot* 
Locseg_Chain_Knot_Array_At(const Locseg_Chain_Knot_Array *ka, int index);
Locseg_Chain_Knot* 
Locseg_Chain_Knot_Array_Last(const Locseg_Chain_Knot_Array *ka);

void Locseg_Chain_Knot_Array_Append(Locseg_Chain_Knot_Array *ka, 
				    Locseg_Chain_Knot *knot);
void Locseg_Chain_Knot_Array_Append_U(Locseg_Chain_Knot_Array *ka, 
				      Locseg_Chain_Knot *knot);

void Print_Locseg_Chain_Knot_Array(const Locseg_Chain_Knot_Array *ka);

Geo3d_Circle* 
Locseg_Chain_Knot_Array_To_Circle_Z(Locseg_Chain_Knot_Array *ka, double z_scale,
				    Geo3d_Circle *circle);
Geo3d_Ellipse* 
Locseg_Chain_Knot_Array_To_Ellipse_Z(Locseg_Chain_Knot_Array *ka, 
				     double z_scale,
				     Geo3d_Ellipse *ellipse);

/*@brief Turn a knot array into a local ellipse array.
 *
 * Locseg_Chain_Knot_Array_To_Locne_Z() returns the local ellipse array from 
 * <ka>.
 */
Local_Neuroseg_Ellipse*
Locseg_Chain_Knot_Array_To_Locne_Z(Locseg_Chain_Knot_Array *ka, 
				   double z_scale,
				   Local_Neuroseg_Ellipse *ellipse);

void Locseg_Chain_Knot_Pos(Locseg_Chain_Knot_Array *ka, int index, double *pos);

__END_DECLS

#endif
