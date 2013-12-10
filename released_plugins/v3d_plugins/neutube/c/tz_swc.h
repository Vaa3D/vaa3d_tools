/**@file tz_swc.h
 * @author Ting Zhao
 * @date 10-Jul-2009
 */

#ifndef _TZ_SWC_H_
#define _TZ_SWC_H_

#include "tz_cdefs.h"
#include "tz_swc_arraylist.h"
#include "tz_graph.h"

__BEGIN_DECLS

int* Swc_Arraylist_Queue(const Swc_Arraylist *sc, int *q);
Graph* Swc_Arraylist_Graph(const Swc_Arraylist *sc, int *q);

Swc_Arraylist* Swc_Arraylist_Rebuild(const Swc_Arraylist *sc, 
				     const Graph *graph);

void Swc_Arraylist_To_Swc_File(const Swc_Arraylist *sc, const char *file_path);

__END_DECLS

#endif
