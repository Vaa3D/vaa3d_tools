/**@file tz_locnp_chain.h
 * @brief additional routines for chain of locnp
 * @author Ting Zhao
 * @date 06-May-2008
 */

#ifndef _TZ_LOCNP_CHAIN_H_
#define _TZ_LOCNP_CHAIN_H_

#include <image_lib.h>
#include "tz_cdefs.h"
#include "tz_locnp_chain_com.h"


__BEGIN_DECLS

/*
 * Test if tracing should stop.
 * Return code: See tz_trace_defs.h for more details.
 */
int Locnp_Chain_Trace_Test2(void *argv[]);

__END_DECLS

#endif 
