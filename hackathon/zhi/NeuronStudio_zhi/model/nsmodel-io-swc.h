#ifndef __NS_MODEL_MODEL_IO_SWC_H__
#define __NS_MODEL_MODEL_IO_SWC_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nsascii.h>
#include <std/nsprint.h>
#include <std/nshashtable.h>
#include <std/nsfile.h>
#include <std/nslog.h>
#include <math/nsvector.h>
#include <model/nsmodel.h>
#include <model/nsmodel-function.h>

NS_DECLS_BEGIN

#define NS_SWC_HEADER_FIELD_MAX_CHARS  255

typedef struct _NsSwcHeader
	{
	nschar  source			 [ NS_SWC_HEADER_FIELD_MAX_CHARS + 1 ];
	nschar  creature      [ NS_SWC_HEADER_FIELD_MAX_CHARS + 1 ];
	nschar  region        [ NS_SWC_HEADER_FIELD_MAX_CHARS + 1 ];
	nschar  field         [ NS_SWC_HEADER_FIELD_MAX_CHARS + 1 ];
	nschar  type          [ NS_SWC_HEADER_FIELD_MAX_CHARS + 1 ];
	nschar  contributor   [ NS_SWC_HEADER_FIELD_MAX_CHARS + 1 ];
	nschar  reference     [ NS_SWC_HEADER_FIELD_MAX_CHARS + 1 ];
	nschar  raw           [ NS_SWC_HEADER_FIELD_MAX_CHARS + 1 ];
	nschar  extras        [ NS_SWC_HEADER_FIELD_MAX_CHARS + 1 ];
	nschar  soma_area     [ NS_SWC_HEADER_FIELD_MAX_CHARS + 1 ];
	nschar  shrinkage     [ NS_SWC_HEADER_FIELD_MAX_CHARS + 1 ];
	nschar  version_number[ NS_SWC_HEADER_FIELD_MAX_CHARS + 1 ];
	nschar  version_date  [ NS_SWC_HEADER_FIELD_MAX_CHARS + 1 ];
	nschar  scale         [ NS_SWC_HEADER_FIELD_MAX_CHARS + 1 ];
	}
	NsSwcHeader;

#define NS_SWC_HEADER_NUM_FIELDS\
	( sizeof( NsSwcHeader ) / ( sizeof( nschar ) * ( NS_SWC_HEADER_FIELD_MAX_CHARS + 1 ) ) )


/* Possible 'flags' values. */
#define NS_SWC_MULTIPLE_FILES      0x0001
#define NS_SWC_FILE_NUM_ONE_BASED  0x0002
#define NS_SWC_FILE_NUM_ZERO_PAD   0x0004
/* #define NS_SWC_DONT_BREAK_LOOPS    0x0008 */
#define NS_SWC_OUTPUT_EDGES        0x0010


/* NOTE: Model vertex colors are altered in the function! */
NS_IMPEXP NsError ns_model_write_swc
	(
	NsModel            *model,
	const NsSwcHeader  *swc_header,
	const nschar       *file_name,
	nsulong             flags
	);

/* 'line_num' can be NULL. If not, it is passed the line
	number where an error occurs, or 0 if no error. */
NS_IMPEXP NsError ns_model_read_swc
	(
	NsModel       *model,
	NsSwcHeader   *swc_header,
	const nschar  *file_name,
	nsulong        flags,
	nslong        *line_num
	);


/* Set and store the ID label for the vertices in the model.
	The value can be retrived for a particular vertex by
	calling ns_model_vertex_get_data( vertex, NS_MODEL_VERTEX_DATA_SLOT0 ).

	'multiple_files' applies the labeling as if the NS_SWC_MULTIPLE_FILES
	flag was specified during a write operation.
*/
NS_IMPEXP void ns_model_set_swc_labels( NsModel *model, nsboolean multiple_files );

NS_DECLS_END

#endif/* __NS_MODEL_MODEL_IO_SWC_H__ */
