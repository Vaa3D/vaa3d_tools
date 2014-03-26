/**@file tz_locne_chain.h
 * @brief additional routines of Locne_Chain
 * @author Ting Zhao
 * @date 06-May-2008
 */

#ifndef _TZ_LOCNE_CHAIN_H_
#define _TZ_LOCNE_CHAIN_H_

#include <image_lib.h>
#include "tz_cdefs.h"
#include "tz_locne_chain_com.h"

__BEGIN_DECLS

void Locne_Chain_Subchain(Locne_Chain *chain, int start, int end);

double* Locne_Chain_Area_Array(Locne_Chain *chain, double *a, int *length);

/* Hit test for a locne chain. The chain must have a constant orientaion,
 * i.e. all ellipses are parallel. */
BOOL Locne_Chain_Hittest(Locne_Chain *chain, const coordinate_3d_t point);

double Locne_Chain_Span(Locne_Chain *chain);

int Locne_Chain_Swc_Fprint(FILE *fp, Locne_Chain *chain, int type,
			   int start_id, int parent_id, 
			   Dlist_Direction_e direction);

Trace_Workspace *Make_Locne_Trace_Workspace(Dlist_Direction_e trace_direction,
					    int max_length, double min_score,
					    double max_r, double max_ecc);

__END_DECLS

#endif 
