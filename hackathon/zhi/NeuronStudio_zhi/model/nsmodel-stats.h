#ifndef __NS_MODEL_MODEL_STATS_H__
#define __NS_MODEL_MODEL_STATS_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nslist.h>
#include <std/nsbytearray.h>
#include <std/nsprint.h>
#include <std/nsmath.h>
#include <std/nsfile.h>
#include <std/nsprogress.h>
#include <math/nsvector.h>
#include <model/nsmodel.h>

NS_DECLS_BEGIN

typedef struct _NsModelStats
	{
	NsByteArray   buffer;
	nssize        num_conn_comp;
	}
	NsModelStats;


NS_IMPEXP NsError ns_model_stats_construct
	(
	NsModelStats  *stats,
	NsModel       *model,
	NsProgress    *progress
	);

NS_IMPEXP void ns_model_stats_destruct( NsModelStats *stats );

NS_IMPEXP nssize ns_model_stats_num_conn_comp( const NsModelStats *stats );

NS_IMPEXP nslong ns_model_stats_min_order
	(
	const NsModelStats  *stats,
	nssize               conn_comp
	);

NS_IMPEXP nslong ns_model_stats_max_order
	(
	const NsModelStats  *stats,
	nssize               conn_comp
	);

NS_IMPEXP nssize ns_model_stats_num_edges
	(
	const NsModelStats  *stats,
	nssize               conn_comp,
	nslong               order
	);

NS_IMPEXP nsfloat ns_model_stats_avg_length
	(
	const NsModelStats  *stats,
	nssize               conn_comp,
	nslong               order
	);

NS_IMPEXP nsfloat ns_model_stats_avg_diameter
	(
	const NsModelStats  *stats,
	nssize               conn_comp,
	nslong               order
	);

NS_IMPEXP nsfloat ns_model_stats_std_dev_length
	(
	const NsModelStats  *stats,
	nssize               conn_comp,
	nslong               order
	);

NS_IMPEXP nsfloat ns_model_stats_std_dev_diameter
	(
	const NsModelStats  *stats,
	nssize               conn_comp,
	nslong               order
	);

NS_IMPEXP NsError ns_model_stats_write( const NsModelStats *stats, const nschar *name );

NS_DECLS_END

#endif/* __NS_MODEL_MODEL_STATS_H__ */
