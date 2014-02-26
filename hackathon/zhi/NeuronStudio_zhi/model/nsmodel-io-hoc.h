#ifndef __NS_MODEL_MODEL_IO_HOC_H__
#define __NS_MODEL_MODEL_IO_HOC_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nsascii.h>
#include <std/nsprint.h>
#include <std/nsfile.h>
#include <std/nslog.h>
#include <math/nsvector.h>
#include <image/nsvoxel-info.h>
#include <model/nsmodel.h>
#include <model/nsmodel-spines.h>

NS_DECLS_BEGIN

/* Possible 'flags' values. */
#define NS_HOC_MULTIPLE_FILES      0x0001
#define NS_HOC_ACCESS_SOMAS        0x0002
#define NS_HOC_FILE_NUM_ONE_BASED  0x0004
#define NS_HOC_FILE_NUM_ZERO_PAD   0x0008
/* #define NS_HOC_DONT_BREAK_LOOPS    0x0010 */


/* NOTE: Model vertex colors are altered in the function! */
NS_IMPEXP NsError ns_model_write_hoc
	(
	NsModel            *model,
	const NsVoxelInfo  *voxel_info,
	const nschar       *file_name,
	nsulong             flags,
	const nschar       *soma_ident,
	const nschar       *dend_ident,
	const nschar       *invalid_sections_ident,
	const nschar       *unknown_sections_ident,
	const nschar       *soma_sections_ident,
	const nschar       *basal_dendrite_sections_ident,
	const nschar       *apical_dendrite_sections_ident,
	const nschar       *axon_sections_ident
	/* const nschar* ( *section_ident )( nsint ) */
	);


/* NOTE: Reading .hoc files is too complex since the
	file format is really a script and that would involve
	creating a compiler of a sort. */

NS_DECLS_END

#endif/* __NS_MODEL_MODEL_IO_HOC_H__ */
