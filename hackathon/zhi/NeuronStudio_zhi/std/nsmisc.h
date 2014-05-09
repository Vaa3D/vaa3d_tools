#ifndef __NS_STD_MISC_H__
#define __NS_STD_MISC_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nsascii.h>
#include <std/nsutil.h>

NS_DECLS_BEGIN

NS_IMPEXP void ns_num_bytes_to_string
	(
	nssize   num_bytes,
	nschar  *string,
	nssize   max_chars
	);


#define NS_NUM_BYTES_TO_STRING_LONG_FORM  0x01

NS_IMPEXP void ns_num_bytes_to_string_ex
	(
	nssize   num_bytes,
	nschar  *string,
	nssize   max_chars,
	nsulong  flags
	);

NS_DECLS_END

#endif/* __NS_STD_MISC_H__ */
