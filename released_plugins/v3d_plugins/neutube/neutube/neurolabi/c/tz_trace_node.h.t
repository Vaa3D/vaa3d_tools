/**@file tz_<1t>_node.h
 * @brief node for <2T> chain
 * @author Ting Zhao
 * @date 23-Oct-2008
 */

#ifndef _TZ_<1T>_NODE_H_
#define _TZ_<1T>_NODE_H_

#include "tz_cdefs.h"
#include "tz_<2t>.h"
#include "tz_trace_defs.h"

/* 1T Locseg 2T Local_Neuroseg */

__BEGIN_DECLS

/**@struct _<1T>_Node tz_<1t>_node.h
 *
 * @{
 */
typedef struct _<1T>_Node {
  <2T> *locseg; /**< local neuron segment */
  Trace_Record *tr;       /**< trace record */
} <1T>_Node;

/**@}*/

typedef <1T>_Node* <1T>_Node_P;

<1T>_Node* New_<1T>_Node();
void Delete_<1T>_Node(<1T>_Node *p);

<1T>_Node* Make_<1T>_Node(const <2T> *locseg,
			      const Trace_Record *tr);
void Kill_<1T>_Node(<1T>_Node *p);

void Clean_<1T>_Node(<1T>_Node *p);

<1T>_Node* Copy_<1T>_Node(const <1T>_Node *p);

BOOL <1T>_Node_Is_Identical(const <1T>_Node *p1, const <1T>_Node *p2);

void Print_<1T>_Node(const <1T>_Node *p);
void Print_<1T>_Node_Info(const <1T>_Node *p);

void <1T>_Node_Fwrite(const <1T>_Node *p, FILE *stream);
<1T>_Node* <1T>_Node_Fread(<1T>_Node *p, FILE *stream);

void <1T>_Node_Fwrite_V(const <1T>_Node *p, FILE *stream, double version);
<1T>_Node* <1T>_Node_Fread_V(<1T>_Node *p, FILE *stream, double version);

void <1T>_Node_Copy_Trace_Record(<1T>_Node *p, const Trace_Record *tr);

void <1T>_Node_Set_Score(<1T>_Node *p, const Stack_Fit_Score *fs);
void <1T>_Node_Set_Hit_Region(<1T>_Node *p, int hit_region);
void <1T>_Node_Set_Index(<1T>_Node *p, int index);
void <1T>_Node_Set_Refit(<1T>_Node *p, BOOL is_refit);
void <1T>_Node_Set_Direction(<1T>_Node *p, Dlist_Direction_e direction);
void <1T>_Node_Set_Fit_Height(<1T>_Node *p, int index, int value);
void <1T>_Node_Set_Fix_Point(<1T>_Node *p, double value);

BOOL <1T>_Node_Fix_Pos(<1T>_Node *p, double *pos);

__END_DECLS

#endif
