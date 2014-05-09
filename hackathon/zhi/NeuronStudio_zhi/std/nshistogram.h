#ifndef __NS_STD_HISTOGRAM_H__
#define __NS_STD_HISTOGRAM_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nsmemory.h>
#include <std/nsfile.h>

NS_DECLS_BEGIN

/* Simple histogram class. Statistical methods not provided. */
typedef struct _NsHistogram
	{
	nslong  **bands;
	nssize   *band_sizes;
	nssize    num_bands;
	}
	NsHistogram;


NS_IMPEXP NsError ns_histogram_construct
	(
	NsHistogram   *histogram,
	nssize         num_bands,
	const nssize  *band_sizes
	);

NS_IMPEXP void ns_histogram_destruct( NsHistogram *histogram );

NS_IMPEXP void ns_histogram_clear( NsHistogram *histogram );

/* NOTE: No rollback on failure! */
NS_IMPEXP NsError ns_histogram_resize
	(
	NsHistogram   *histogram,
	nssize         num_bands,
	const nssize  *band_sizes
	);

/* Returns the total size of all the bands. */
NS_IMPEXP nssize ns_histogram_size( const NsHistogram *histogram );
NS_IMPEXP nssize ns_histogram_band_size( const NsHistogram *histogram, nssize band );

NS_IMPEXP nssize ns_histogram_num_bands( const NsHistogram *histogram );

NS_IMPEXP void ns_histogram_up( NsHistogram *histogram, nssize band, nssize at );

/* Do we really need a down function? */
NS_IMPEXP void ns_histogram_down( NsHistogram *histogram, nssize band, nssize at );

NS_IMPEXP nslong ns_histogram_at( const NsHistogram *histogram, nssize band, nssize at );

NS_IMPEXP NsError ns_histogram_write( const NsHistogram *histogram, const nschar *file_name );

NS_DECLS_END

#endif/* __NS_STD_HISTOGRAM_H__ */
