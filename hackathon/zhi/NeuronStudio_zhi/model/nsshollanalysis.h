#ifndef __NS_MODEL_SHOLL_ANALYSIS_H__
#define __NS_MODEL_SHOLL_ANALYSIS_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nsprogress.h>
#include <std/nsmemory.h>
#include <std/nsfile.h>
#include <std/nslist.h>
#include <std/nsutil.h>
#include <std/nsmath.h>
#include <model/nsmodel.h>
#include <model/nsmodel-spines.h>
#include <model/nsspines-classifier.h>

NS_DECLS_BEGIN

typedef struct _NsShollAnalysis
	{
	NsList               annuli;
	NsModel             *model;
	NsVector3f           origin;
	NsSpinesClassifier  *classifier;
	nsfloat              min_radius;
	nsfloat              max_radius;
	nsfloat              radius_delta;
	nssize               num_annuli;
	NsProgress          *progress;
	}
	NsShollAnalysis;


NS_IMPEXP void ns_sholl_analysis_construct( NsShollAnalysis *sa );
NS_IMPEXP void ns_sholl_analysis_destruct( NsShollAnalysis *sa );

/* Returns the number of annuli. */
NS_IMPEXP nssize ns_sholl_analysis_size( const NsShollAnalysis *sa );

NS_IMPEXP void ns_sholl_analysis_clear( NsShollAnalysis *sa );

NS_IMPEXP const NsVector3f* ns_sholl_analysis_origin( const NsShollAnalysis *sa );

NS_IMPEXP NsError ns_sholl_analysis_run
	(
	NsShollAnalysis  *sa,
	NsModel          *model,
	nsfloat           min_radius,
	nsfloat           max_radius,
	nsfloat           radius_delta,
	NsProgress       *progress
	);

NS_IMPEXP NsError ns_sholl_analysis_write( const NsShollAnalysis *sa, const nschar *file_name );


typedef nslistiter nsshollannulus;

NS_IMPEXP nsshollannulus ns_sholl_analysis_begin( const NsShollAnalysis *sa );
NS_IMPEXP nsshollannulus ns_sholl_analysis_end( const NsShollAnalysis *sa );

#define ns_sholl_annulus_not_equal( annulus1, annulus2 )\
   ns_list_iter_not_equal( (annulus1), (annulus2) )

#define ns_sholl_annulus_next( annulus )\
   ns_list_iter_next( ( nslistiter )(annulus) )

NS_IMPEXP nssize ns_sholl_annulus_index( const nsshollannulus A );
NS_IMPEXP nsfloat ns_sholl_annulus_start_radius( const nsshollannulus A );
NS_IMPEXP nsfloat ns_sholl_annulus_end_radius( const nsshollannulus A );

NS_DECLS_END

#endif/* __NS_MODEL_SHOLL_ANALYSIS_H__ */
