#ifndef __NS_MODEL_MODEL_IO_NZM_H__
#define __NS_MODEL_MODEL_IO_NZM_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nslist.h>
#include <std/nsstack.h>
#include <std/nsascii.h>
#include <std/nsprint.h>
#include <std/nsfile.h>
#include <std/nslog.h>
#include <std/nsmath.h>
#include <math/nsvector.h>
#include <model/nsmodel.h>

NS_DECLS_BEGIN

/* 'line_num' can be NULL. If not, it is passed the line
	number where an error occurs, or 0 if no error. */
NS_IMPEXP NsError ns_model_read_nzm
	(
	NsModel       *model,
	const nschar  *file_name,
	nsulong        flags,
	nslong        *line_num
	);

NS_DECLS_END

#endif/* __NS_MODEL_MODEL_IO_NZM_H__ */
